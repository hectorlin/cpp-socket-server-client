#include "../include/hft_server.hpp"
#include <iostream>
#include <algorithm>
#include <signal.h>
#include <mutex>

HFTServer* HFTServer::instance = nullptr;
std::mutex HFTServer::mutex;

HFTServer::HFTServer() : serverSocket(-1), epollFd(-1), running(false), 
                         requestQueue(50000), responseBuffers(HFT_THREAD_POOL_SIZE * 2) {
    startTime = std::chrono::high_resolution_clock::now();
}

HFTServer::~HFTServer() {
    stop();
}

HFTServer* HFTServer::getInstance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new HFTServer();
    }
    return instance;
}

void HFTServer::setNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void HFTServer::setupEpoll() {
    epollFd = epoll_create1(0);
    if (epollFd == -1) {
        throw std::runtime_error("Failed to create epoll instance");
    }
    
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        throw std::runtime_error("Failed to add server socket to epoll");
    }
}

void HFTServer::start(int port) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        throw std::runtime_error("Failed to create socket");
    }
    
    // Set socket options for HFT
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    
    // Set TCP_NODELAY for low latency
    setsockopt(serverSocket, IPPROTO_TCP, 1, &opt, sizeof(opt)); // TCP_NODELAY = 1
    
    // Set send/receive buffer sizes
    int bufferSize = 65536;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
    setsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::runtime_error("Failed to bind socket");
    }
    
    if (listen(serverSocket, SOMAXCONN) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }
    
    setNonBlocking(serverSocket);
    setupEpoll();
    
    running = true;
    std::cout << "HFT Server started on port " << port << std::endl;
    
    // Start worker threads
    for (int i = 0; i < HFT_THREAD_POOL_SIZE; ++i) {
        workerThreads.emplace_back(&HFTServer::workerThread, this);
    }
    
    acceptConnections();
}

void HFTServer::stop() {
    running = false;
    
    if (epollFd != -1) {
        close(epollFd);
        epollFd = -1;
    }
    
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
    
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads.clear();
    
    std::cout << "HFT Server stopped" << std::endl;
}

void HFTServer::acceptConnections() {
    struct epoll_event events[HFT_MAX_EVENTS];
    
    while (running) {
        int numEvents = epoll_wait(epollFd, events, HFT_MAX_EVENTS, 1);
        
        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == serverSocket) {
                // Accept new connection
                sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
                
                if (clientSocket != -1) {
                    setNonBlocking(clientSocket);
                    
                    // Set TCP_NODELAY for client socket
                    int opt = 1;
                    setsockopt(clientSocket, IPPROTO_TCP, 1, &opt, sizeof(opt)); // TCP_NODELAY = 1
                    
                    struct epoll_event event;
                    event.events = EPOLLIN | EPOLLET; // Edge triggered
                    event.data.fd = clientSocket;
                    
                    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) != -1) {
                        char clientIP[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
                        std::cout << "New HFT connection from " << clientIP << std::endl;
                    } else {
                        close(clientSocket);
                    }
                }
            } else {
                // Handle client data
                handleClient(events[i].data.fd);
            }
        }
    }
}

void HFTServer::handleClient(int clientSocket) {
    char buffer[HFT_BUFFER_SIZE];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::string request(buffer);
        
        // Add to lock-free queue for processing
        requestQueue.enqueue(std::make_pair(clientSocket, request));
    } else if (bytesRead == 0) {
        // Client disconnected
        epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
        close(clientSocket);
    }
}

void HFTServer::workerThread() {
    std::pair<int, std::string> request;
    
    while (running) {
        if (requestQueue.dequeue(request)) {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            std::string response = processRequest(request.second);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
            
            totalRequests++;
            totalLatency += latency;
            
            // Send response
            send(request.first, response.c_str(), response.length(), MSG_NOSIGNAL);
        } else {
            // Brief sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
}

std::string HFTServer::processRequest(const std::string& request) {
    std::string processedRequest = request;
    
    // Execute pre-processing interceptors (optimized order)
    for (auto& interceptor : interceptors) {
        if (!interceptor->preProcess(processedRequest)) {
            return "ERROR: Request rejected by interceptor";
        }
    }
    
    // Find appropriate service
    std::string response;
    for (auto& service : services) {
        response = service->processRequest(processedRequest);
        if (!response.empty()) {
            break;
        }
    }
    
    if (response.empty()) {
        response = "ERROR: No service available to handle request";
    }
    
    // Execute post-processing interceptors
    for (auto& interceptor : interceptors) {
        interceptor->postProcess(processedRequest, response);
    }
    
    return response;
}

void HFTServer::addService(std::unique_ptr<IService> service) {
    service->initialize();
    services.push_back(std::move(service));
}

void HFTServer::addInterceptor(std::unique_ptr<IInterceptor> interceptor) {
    interceptors.push_back(std::move(interceptor));
    
    // Sort interceptors by priority for optimal processing order
    std::sort(interceptors.begin(), interceptors.end(),
              [](const std::unique_ptr<IInterceptor>& a, const std::unique_ptr<IInterceptor>& b) {
                  return a->getPriority() < b->getPriority();
              });
}

HFTResponseBuffer* HFTServer::getResponseBuffer() {
    size_t index = bufferIndex.fetch_add(1) % responseBuffers.size();
    return &responseBuffers[index];
}

void HFTServer::resetMetrics() {
    totalRequests = 0;
    totalLatency = 0;
    startTime = std::chrono::high_resolution_clock::now();
} 