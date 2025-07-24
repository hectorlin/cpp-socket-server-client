#include "../include/client.hpp"
#include "../include/interceptors.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>

class SimpleBenchmark {
private:
    std::string serverIp;
    int serverPort;

public:
    SimpleBenchmark(const std::string& ip, int port) : serverIp(ip), serverPort(port) {}

    void runBasicTest() {
        std::cout << "\n=== Basic Performance Test ===" << std::endl;
        
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Failed to connect for basic test" << std::endl;
            return;
        }

        std::vector<long long> latencies;
        int successCount = 0;
        int failureCount = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Test 100 requests
        for (int i = 0; i < 100; ++i) {
            auto requestStart = std::chrono::high_resolution_clock::now();
            
            std::string request = "TOKEN:secret123 ECHO Test message " + std::to_string(i);
            std::string response = client.sendRequest(request);
            
            auto requestEnd = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(requestEnd - requestStart).count();
            
            latencies.push_back(latency);
            
            if (response.find("ECHO:") != std::string::npos) {
                successCount++;
            } else {
                failureCount++;
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        
        printResults(latencies, successCount, failureCount, totalTime);
        client.disconnect();
    }

    void runMultiServiceTest() {
        std::cout << "\n=== Multi-Service Test ===" << std::endl;
        
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Failed to connect for multi-service test" << std::endl;
            return;
        }

        std::vector<std::string> requests = {
            "TOKEN:secret123 ECHO Hello World",
            "TOKEN:secret123 CAL 2 + 3",
            "TOKEN:secret123 CAL 10 - 5",
            "TOKEN:secret123 READ test.txt",
            "TOKEN:secret123 WRITE benchmark.txt Benchmark data"
        };
        
        std::vector<long long> latencies;
        int successCount = 0;
        int failureCount = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Test each service multiple times
        for (int round = 0; round < 20; ++round) {
            for (const auto& request : requests) {
                auto requestStart = std::chrono::high_resolution_clock::now();
                
                std::string response = client.sendRequest(request);
                
                auto requestEnd = std::chrono::high_resolution_clock::now();
                auto latency = std::chrono::duration_cast<std::chrono::microseconds>(requestEnd - requestStart).count();
                
                latencies.push_back(latency);
                
                if (!response.empty() && response.find("ERROR:") == std::string::npos) {
                    successCount++;
                } else {
                    failureCount++;
                }
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        
        printResults(latencies, successCount, failureCount, totalTime);
        client.disconnect();
    }

    void runConcurrentTest() {
        std::cout << "\n=== Concurrent Connection Test ===" << std::endl;
        
        std::atomic<int> successCount{0};
        std::atomic<int> failureCount{0};
        std::atomic<long long> totalLatency{0};
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        int numThreads = 5;
        int requestsPerThread = 20;
        
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back([this, i, requestsPerThread, &successCount, &failureCount, &totalLatency]() {
                SocketClient client(serverIp, serverPort);
                if (!client.connect()) {
                    std::cerr << "Thread " << i << " failed to connect" << std::endl;
                    return;
                }
                
                for (int j = 0; j < requestsPerThread; ++j) {
                    auto requestStart = std::chrono::high_resolution_clock::now();
                    
                    std::string request = "TOKEN:secret123 ECHO Concurrent test " + std::to_string(i) + "-" + std::to_string(j);
                    std::string response = client.sendRequest(request);
                    
                    auto requestEnd = std::chrono::high_resolution_clock::now();
                    auto latency = std::chrono::duration_cast<std::chrono::microseconds>(requestEnd - requestStart).count();
                    
                    totalLatency += latency;
                    
                    if (response.find("ECHO:") != std::string::npos) {
                        successCount++;
                    } else {
                        failureCount++;
                    }
                }
                
                client.disconnect();
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        
        int totalRequests = successCount + failureCount;
        long long avgLatency = totalRequests > 0 ? totalLatency / totalRequests : 0;
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Concurrent Test Results:" << std::endl;
        std::cout << "  Total Time: " << totalTime << " ms" << std::endl;
        std::cout << "  Total Requests: " << totalRequests << std::endl;
        std::cout << "  Threads: " << numThreads << std::endl;
        std::cout << "  Success Rate: " << (successCount * 100.0 / totalRequests) << "%" << std::endl;
        std::cout << "  Throughput: " << (totalRequests * 1000.0 / totalTime) << " req/sec" << std::endl;
        std::cout << "  Avg Latency: " << avgLatency << " μs" << std::endl;
    }

private:
    void printResults(const std::vector<long long>& latencies, int successCount, int failureCount, long long totalTime) {
        if (latencies.empty()) return;
        
        std::vector<long long> sortedLatencies = latencies;
        std::sort(sortedLatencies.begin(), sortedLatencies.end());
        
        long long minLatency = sortedLatencies.front();
        long long maxLatency = sortedLatencies.back();
        long long totalLatency = 0;
        for (auto latency : latencies) {
            totalLatency += latency;
        }
        long long avgLatency = totalLatency / latencies.size();
        long long medianLatency = sortedLatencies[sortedLatencies.size() / 2];
        
        int totalRequests = successCount + failureCount;
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Test Results:" << std::endl;
        std::cout << "  Total Time: " << totalTime << " ms" << std::endl;
        std::cout << "  Requests: " << totalRequests << std::endl;
        std::cout << "  Success Rate: " << (successCount * 100.0 / totalRequests) << "%" << std::endl;
        std::cout << "  Throughput: " << (totalRequests * 1000.0 / totalTime) << " req/sec" << std::endl;
        std::cout << "  Min Latency: " << minLatency << " μs" << std::endl;
        std::cout << "  Max Latency: " << maxLatency << " μs" << std::endl;
        std::cout << "  Avg Latency: " << avgLatency << " μs" << std::endl;
        std::cout << "  Median Latency: " << medianLatency << " μs" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string serverIp = "127.0.0.1";
    int serverPort = 8080;
    
    if (argc > 1) serverIp = argv[1];
    if (argc > 2) serverPort = std::stoi(argv[2]);
    
    std::cout << "Simple Socket Server/Client Benchmark" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cout << "Server: " << serverIp << ":" << serverPort << std::endl;
    std::cout << "Starting simple benchmark tests..." << std::endl;
    
    SimpleBenchmark benchmark(serverIp, serverPort);
    
    // Run different benchmark tests
    benchmark.runBasicTest();
    benchmark.runMultiServiceTest();
    benchmark.runConcurrentTest();
    
    std::cout << "\n=== Simple Benchmark Complete ===" << std::endl;
    
    return 0;
} 