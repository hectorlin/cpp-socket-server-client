#include "../include/client.hpp"
#include "../include/interceptors.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

void runClientTests(SocketClient& client) {
    std::cout << "\n[TEST] Running client tests..." << std::endl;
    
    // Test requests
    std::vector<std::string> requests = {
        "TOKEN:secret123 ECHO Hello World",
        "TOKEN:secret123 CAL 2 + 3",
        "TOKEN:secret123 CAL 10 - 5",
        "TOKEN:secret123 ECHO Test Message",
        "TOKEN:secret123 READ test.txt",
        "TOKEN:secret123 WRITE test.txt Hello from client!",
        "TOKEN:secret123 READ test.txt"
    };
    
    for (const auto& request : requests) {
        std::cout << "\n[CLIENT] Sending: " << request << std::endl;
        std::string response = client.sendRequest(request);
        std::cout << "[CLIENT] Received: " << response << std::endl;
        
        // Small delay between requests
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void interactiveMode(SocketClient& client) {
    std::cout << "\n[INFO] Entering interactive mode. Type 'quit' to exit." << std::endl;
    std::cout << "[INFO] Available commands:" << std::endl;
    std::cout << "  TOKEN:secret123 ECHO <message>" << std::endl;
    std::cout << "  TOKEN:secret123 CAL <expression>" << std::endl;
    std::cout << "  TOKEN:secret123 READ <filename>" << std::endl;
    std::cout << "  TOKEN:secret123 WRITE <filename> <content>" << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "\n[CLIENT] Enter command: ";
        std::getline(std::cin, input);
        
        if (input == "quit" || input == "exit") {
            break;
        }
        
        if (!input.empty()) {
            std::string response = client.sendRequest(input);
            std::cout << "[CLIENT] Response: " << response << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::string serverIp = "127.0.0.1";
    int serverPort = 8080;
    bool interactive = false;
    
    // Parse command line arguments
    if (argc > 1) {
        serverIp = argv[1];
    }
    if (argc > 2) {
        serverPort = std::stoi(argv[2]);
    }
    if (argc > 3 && std::string(argv[3]) == "--interactive") {
        interactive = true;
    }
    
    std::cout << "Socket Client with Interceptor Architecture" << std::endl;
    std::cout << "===========================================" << std::endl;
    
    SocketClient client(serverIp, serverPort);
    
    // Add interceptors
    std::cout << "[SETUP] Adding client interceptors..." << std::endl;
    client.addInterceptor(std::unique_ptr<LoggingInterceptor>(new LoggingInterceptor()));
    client.addInterceptor(std::unique_ptr<AuthenticationInterceptor>(new AuthenticationInterceptor("secret123")));
    client.addInterceptor(std::unique_ptr<ValidationInterceptor>(new ValidationInterceptor()));
    
    // Connect to server
    std::cout << "[INFO] Connecting to server " << serverIp << ":" << serverPort << std::endl;
    if (!client.connect()) {
        std::cerr << "[ERROR] Failed to connect to server" << std::endl;
        return 1;
    }
    
    try {
        if (interactive) {
            interactiveMode(client);
        } else {
            runClientTests(client);
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Client error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n[INFO] Client shutting down..." << std::endl;
    client.disconnect();
    
    return 0;
} 