#include "../include/server.hpp"
#include "../include/services.hpp"
#include "../include/interceptors.hpp"
#include <iostream>
#include <signal.h>

SocketServer* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down server..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    // Parse command line arguments
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        std::cout << "Starting Socket Server with Singleton, Service, and Interceptor Architecture" << std::endl;
        std::cout << "==================================================================" << std::endl;
        
        auto server = SocketServer::getInstance();
        g_server = server;
        
        // Add services
        std::cout << "\n[SETUP] Adding services..." << std::endl;
        server->addService(std::unique_ptr<EchoService>(new EchoService()));
        server->addService(std::unique_ptr<CalculatorService>(new CalculatorService()));
        server->addService(std::unique_ptr<FileService>(new FileService()));
        
        // Add interceptors
        std::cout << "[SETUP] Adding interceptors..." << std::endl;
        server->addInterceptor(std::unique_ptr<LoggingInterceptor>(new LoggingInterceptor()));
        server->addInterceptor(std::unique_ptr<AuthenticationInterceptor>(new AuthenticationInterceptor("secret123")));
        server->addInterceptor(std::unique_ptr<RateLimitingInterceptor>(new RateLimitingInterceptor(10000)));
        server->addInterceptor(std::unique_ptr<ValidationInterceptor>(new ValidationInterceptor()));
        
        std::cout << "\n[INFO] Available commands:" << std::endl;
        std::cout << "  TOKEN:secret123 ECHO <message>     - Echo service" << std::endl;
        std::cout << "  TOKEN:secret123 CAL <expression>   - Calculator service" << std::endl;
        std::cout << "  TOKEN:secret123 READ <filename>    - File read service" << std::endl;
        std::cout << "  TOKEN:secret123 WRITE <filename> <content> - File write service" << std::endl;
        std::cout << "\n[INFO] Server will start on port " << port << std::endl;
        std::cout << "[INFO] Press Ctrl+C to stop the server" << std::endl;
        
        // Start server
        server->start(port);
        
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 