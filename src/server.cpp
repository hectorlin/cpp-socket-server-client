#include "../include/server.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <stdexcept>

SocketServer* SocketServer::instance = nullptr;
std::mutex SocketServer::mutex;

SocketServer::SocketServer() : serverSocket(-1), running(false) {}

SocketServer::~SocketServer() {
    stop();
}

SocketServer* SocketServer::getInstance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        instance = new SocketServer();
    }
    return instance;
}

void SocketServer::start(int port) {
    if (running) return;
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }
    
    if (listen(serverSocket, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }
    
    running = true;
    std::cout << "Server started on port " << port << std::endl;
    
    acceptConnections();
}

void SocketServer::stop() {
    if (!running) return;
    
    running = false;
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }
    
    for (auto& thread : workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workerThreads.clear();
    
    std::cout << "Server stopped" << std::endl;
}

void SocketServer::acceptConnections() {
    while (running) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (running) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }
        
        std::cout << "New connection from " << inet_ntoa(clientAddr.sin_addr) << std::endl;
        workerThreads.emplace_back(&SocketServer::handleClient, this, clientSocket);
    }
}

void SocketServer::handleClient(int clientSocket) {
    char buffer[1024];
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) break;
        
        std::string request(buffer);
        std::string response = processRequest(request);
        
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    
    close(clientSocket);
}

std::string SocketServer::processRequest(const std::string& request) {
    std::string processedRequest = request;
    
    // Execute pre-processing interceptors
    std::sort(interceptors.begin(), interceptors.end(), 
              [](const std::unique_ptr<IInterceptor>& a, const std::unique_ptr<IInterceptor>& b) {
                  return a->getPriority() < b->getPriority();
              });
    
    for (auto& interceptor : interceptors) {
        if (!interceptor->preProcess(processedRequest)) {
            return "ERROR: Request rejected by interceptor";
        }
    }
    
    // Process request
    std::string response;
    for (auto& service : services) {
        response = service->processRequest(processedRequest);
        if (!response.empty()) break;
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

void SocketServer::addService(std::unique_ptr<IService> service) {
    service->initialize();
    services.push_back(std::move(service));
}

void SocketServer::addInterceptor(std::unique_ptr<IInterceptor> interceptor) {
    interceptors.push_back(std::move(interceptor));
} 