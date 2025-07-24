#include "../include/hft_server.hpp"
#include "../include/services.hpp"
#include "../include/interceptors.hpp"
#include <iostream>
#include <signal.h>
#include <chrono>
#include <thread>

HFTServer* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down HFT server..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

void printPerformanceStats(const HFTServer* server) {
    auto requests = server->getTotalRequests();
    auto avgLatency = server->getAverageLatency();
    
    std::cout << "\n=== HFT Performance Statistics ===" << std::endl;
    std::cout << "Total Requests: " << requests << std::endl;
    std::cout << "Average Latency: " << avgLatency << " μs" << std::endl;
    if (requests > 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        std::cout << "Requests/sec: " << (requests / std::max(1ULL, static_cast<unsigned long long>(duration))) << std::endl;
    }
    std::cout << "=================================" << std::endl;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    
    std::cout << "Starting HFT-Optimized Socket Server" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Thread Pool Size: " << HFT_THREAD_POOL_SIZE << std::endl;
    std::cout << "Buffer Size: " << HFT_BUFFER_SIZE << " bytes" << std::endl;
    std::cout << "Max Events: " << HFT_MAX_EVENTS << std::endl;
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        g_server = HFTServer::getInstance();
        
        // Add services
        std::cout << "\n[SETUP] Adding services..." << std::endl;
        g_server->addService(std::unique_ptr<EchoService>(new EchoService()));
        g_server->addService(std::unique_ptr<CalculatorService>(new CalculatorService()));
        g_server->addService(std::unique_ptr<FileService>(new FileService()));
        
        // Add interceptors (minimal for HFT)
        std::cout << "[SETUP] Adding interceptors..." << std::endl;
        // Note: Removed logging and rate limiting for HFT performance
        g_server->addInterceptor(std::unique_ptr<AuthenticationInterceptor>(new AuthenticationInterceptor("secret123")));
        
        std::cout << "\n[INFO] Available commands:" << std::endl;
        std::cout << "  TOKEN:secret123 ECHO <message>     - Echo service" << std::endl;
        std::cout << "  TOKEN:secret123 CAL <expression>   - Calculator service" << std::endl;
        std::cout << "  TOKEN:secret123 READ <filename>    - File read service" << std::endl;
        std::cout << "  TOKEN:secret123 WRITE <filename> <content> - File write service" << std::endl;
        
        std::cout << "\n[INFO] HFT Server will start on port " << port << std::endl;
        std::cout << "[INFO] Press Ctrl+C to stop the server" << std::endl;
        
        // Start performance monitoring thread
        std::thread monitorThread([&]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                printPerformanceStats(g_server);
            }
        });
        
        // Start the server
        g_server->start(port);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 