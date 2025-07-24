#pragma once
#include "interfaces.hpp"
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class SocketClient {
private:
    int clientSocket;
    std::string serverIp;
    int serverPort;
    std::vector<std::unique_ptr<IInterceptor>> interceptors;
    
public:
    SocketClient(const std::string& ip, int port);
    ~SocketClient();
    
    bool connect();
    void disconnect();
    std::string sendRequest(const std::string& request);
    void addInterceptor(std::unique_ptr<IInterceptor> interceptor);
}; 