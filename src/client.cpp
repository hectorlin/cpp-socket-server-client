#include "../include/client.hpp"
#include <iostream>
#include <cstring>
#include <algorithm>

SocketClient::SocketClient(const std::string& ip, int port) 
    : clientSocket(-1), serverIp(ip), serverPort(port) {}

SocketClient::~SocketClient() {
    disconnect();
}

bool SocketClient::connect() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Failed to create client socket" << std::endl;
        return false;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return false;
    }
    
    if (::connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return false;
    }
    
    std::cout << "Connected to server " << serverIp << ":" << serverPort << std::endl;
    return true;
}

void SocketClient::disconnect() {
    if (clientSocket >= 0) {
        close(clientSocket);
        clientSocket = -1;
    }
}

std::string SocketClient::sendRequest(const std::string& request) {
    if (clientSocket < 0) {
        return "ERROR: Not connected to server";
    }
    
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
    
    // Send request
    if (send(clientSocket, processedRequest.c_str(), processedRequest.length(), 0) < 0) {
        return "ERROR: Failed to send request";
    }
    
    // Receive response
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0) {
        return "ERROR: Failed to receive response";
    }
    
    std::string response(buffer);
    
    // Execute post-processing interceptors
    for (auto& interceptor : interceptors) {
        interceptor->postProcess(processedRequest, response);
    }
    
    return response;
}

void SocketClient::addInterceptor(std::unique_ptr<IInterceptor> interceptor) {
    interceptors.push_back(std::move(interceptor));
} 