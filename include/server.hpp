#pragma once
#include "interfaces.hpp"
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class SocketServer {
private:
    static SocketServer* instance;
    static std::mutex mutex;
    
    int serverSocket;
    std::atomic<bool> running;
    std::vector<std::thread> workerThreads;
    std::vector<std::unique_ptr<IService>> services;
    std::vector<std::unique_ptr<IInterceptor>> interceptors;
    
    SocketServer();
    ~SocketServer();
    SocketServer(const SocketServer&) = delete;
    SocketServer& operator=(const SocketServer&) = delete;
    
    void acceptConnections();
    void handleClient(int clientSocket);
    std::string processRequest(const std::string& request);

public:
    static SocketServer* getInstance();
    void start(int port);
    void stop();
    void addService(std::unique_ptr<IService> service);
    void addInterceptor(std::unique_ptr<IInterceptor> interceptor);
}; 