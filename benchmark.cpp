#include "../include/client.hpp"
#include "../include/interceptors.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <random>
#include <mutex>

class Benchmark {
private:
    std::string serverIp;
    int serverPort;
    std::atomic<int> successCount{0};
    std::atomic<int> failureCount{0};
    std::atomic<long long> totalLatency{0};
    std::vector<long long> latencies;
    std::mutex latenciesMutex;

public:
    Benchmark(const std::string& ip, int port) : serverIp(ip), serverPort(port) {}

    void runLatencyTest(int numRequests) {
        std::cout << "\n=== Latency Test ===" << std::endl;
        std::cout << "Testing " << numRequests << " sequential requests..." << std::endl;
        
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Failed to connect for latency test" << std::endl;
            return;
        }

        latencies.clear();
        latencies.reserve(numRequests);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numRequests; ++i) {
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
        
        printLatencyResults(numRequests, totalTime);
        client.disconnect();
    }

    void runThroughputTest(int numRequests, int numThreads) {
        std::cout << "\n=== Throughput Test ===" << std::endl;
        std::cout << "Testing " << numRequests << " requests with " << numThreads << " threads..." << std::endl;
        
        successCount = 0;
        failureCount = 0;
        totalLatency = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        int requestsPerThread = numRequests / numThreads;
        
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(&Benchmark::throughputWorker, this, requestsPerThread, i);
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        
        printThroughputResults(numRequests, totalTime, numThreads);
    }

    void runConcurrentConnectionTest(int numConnections) {
        std::cout << "\n=== Concurrent Connection Test ===" << std::endl;
        std::cout << "Testing " << numConnections << " concurrent connections..." << std::endl;
        
        std::vector<std::unique_ptr<SocketClient>> clients;
        std::vector<std::thread> threads;
        std::atomic<int> connectedCount{0};
        std::atomic<int> failedCount{0};
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numConnections; ++i) {
            threads.emplace_back([this, i, &connectedCount, &failedCount]() {
                SocketClient client(serverIp, serverPort);
                if (client.connect()) {
                    connectedCount++;
                    // Send a test request
                    std::string response = client.sendRequest("TOKEN:secret123 ECHO Connection test " + std::to_string(i));
                    if (response.find("ECHO:") != std::string::npos) {
                        // Success
                    }
                    client.disconnect();
                } else {
                    failedCount++;
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        
        printConcurrentResults(connectedCount.load(), failedCount.load(), totalTime);
    }

    void runStressTest(int durationSeconds, int requestsPerSecond) {
        std::cout << "\n=== Stress Test ===" << std::endl;
        std::cout << "Running for " << durationSeconds << " seconds at " << requestsPerSecond << " req/sec..." << std::endl;
        
        successCount = 0;
        failureCount = 0;
        totalLatency = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        auto endTime = startTime + std::chrono::seconds(durationSeconds);
        
        std::vector<std::thread> threads;
        int numThreads = std::min(10, requestsPerSecond / 10); // Limit threads
        int requestsPerThread = requestsPerSecond / numThreads;
        
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(&Benchmark::stressWorker, this, requestsPerThread, endTime, i);
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto actualEndTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(actualEndTime - startTime).count();
        
        printStressResults(totalTime);
    }

private:
    void throughputWorker(int numRequests, int threadId) {
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Thread " << threadId << " failed to connect" << std::endl;
            return;
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 2);
        
        for (int i = 0; i < numRequests; ++i) {
            auto requestStart = std::chrono::high_resolution_clock::now();
            
            std::string request;
            int requestType = dis(gen);
            
            switch (requestType) {
                case 0:
                    request = "TOKEN:secret123 ECHO Throughput test " + std::to_string(i);
                    break;
                case 1:
                    request = "TOKEN:secret123 CAL " + std::to_string(i) + " + " + std::to_string(i);
                    break;
                case 2:
                    request = "TOKEN:secret123 READ test.txt";
                    break;
            }
            
            std::string response = client.sendRequest(request);
            
            auto requestEnd = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(requestEnd - requestStart).count();
            
            totalLatency += latency;
            
            if (!response.empty() && response.find("ERROR:") == std::string::npos) {
                successCount++;
            } else {
                failureCount++;
            }
        }
        
        client.disconnect();
    }

    void stressWorker(int requestsPerThread, std::chrono::high_resolution_clock::time_point endTime, int threadId) {
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Stress thread " << threadId << " failed to connect" << std::endl;
            return;
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 3);
        
        while (std::chrono::high_resolution_clock::now() < endTime) {
            auto requestStart = std::chrono::high_resolution_clock::now();
            
            std::string request;
            int requestType = dis(gen);
            
            switch (requestType) {
                case 0:
                    request = "TOKEN:secret123 ECHO Stress test";
                    break;
                case 1:
                    request = "TOKEN:secret123 CAL 1 + 1";
                    break;
                case 2:
                    request = "TOKEN:secret123 READ test.txt";
                    break;
                case 3:
                    request = "TOKEN:secret123 WRITE stress.txt Stress test data";
                    break;
            }
            
            std::string response = client.sendRequest(request);
            
            auto requestEnd = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(requestEnd - requestStart).count();
            
            totalLatency += latency;
            
            if (!response.empty() && response.find("ERROR:") == std::string::npos) {
                successCount++;
            } else {
                failureCount++;
            }
            
            // Small delay to control rate
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / requestsPerThread));
        }
        
        client.disconnect();
    }

    void printLatencyResults(int numRequests, long long totalTime) {
        if (latencies.empty()) return;
        
        std::sort(latencies.begin(), latencies.end());
        
        long long minLatency = latencies.front();
        long long maxLatency = latencies.back();
        long long avgLatency = totalLatency / numRequests;
        long long medianLatency = latencies[latencies.size() / 2];
        long long p95Latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
        long long p99Latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Latency Results:" << std::endl;
        std::cout << "  Total Time: " << totalTime << " ms" << std::endl;
        std::cout << "  Requests: " << numRequests << std::endl;
        std::cout << "  Success Rate: " << (successCount * 100.0 / numRequests) << "%" << std::endl;
        std::cout << "  Throughput: " << (numRequests * 1000.0 / totalTime) << " req/sec" << std::endl;
        std::cout << "  Min Latency: " << minLatency << " μs" << std::endl;
        std::cout << "  Max Latency: " << maxLatency << " μs" << std::endl;
        std::cout << "  Avg Latency: " << avgLatency << " μs" << std::endl;
        std::cout << "  Median Latency: " << medianLatency << " μs" << std::endl;
        std::cout << "  95th Percentile: " << p95Latency << " μs" << std::endl;
        std::cout << "  99th Percentile: " << p99Latency << " μs" << std::endl;
    }

    void printThroughputResults(int numRequests, long long totalTime, int numThreads) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Throughput Results:" << std::endl;
        std::cout << "  Total Time: " << totalTime << " ms" << std::endl;
        std::cout << "  Requests: " << numRequests << std::endl;
        std::cout << "  Threads: " << numThreads << std::endl;
        std::cout << "  Success Rate: " << (successCount * 100.0 / numRequests) << "%" << std::endl;
        std::cout << "  Throughput: " << (numRequests * 1000.0 / totalTime) << " req/sec" << std::endl;
        std::cout << "  Avg Latency: " << (totalLatency / numRequests) << " μs" << std::endl;
    }

    void printConcurrentResults(int connected, int failed, long long totalTime) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Concurrent Connection Results:" << std::endl;
        std::cout << "  Total Time: " << totalTime << " ms" << std::endl;
        std::cout << "  Successful Connections: " << connected << std::endl;
        std::cout << "  Failed Connections: " << failed << std::endl;
        std::cout << "  Success Rate: " << (connected * 100.0 / (connected + failed)) << "%" << std::endl;
    }

    void printStressResults(long long totalTime) {
        int totalRequests = successCount + failureCount;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Stress Test Results:" << std::endl;
        std::cout << "  Total Time: " << totalTime << " ms" << std::endl;
        std::cout << "  Total Requests: " << totalRequests << std::endl;
        std::cout << "  Success Rate: " << (successCount * 100.0 / totalRequests) << "%" << std::endl;
        std::cout << "  Throughput: " << (totalRequests * 1000.0 / totalTime) << " req/sec" << std::endl;
        std::cout << "  Avg Latency: " << (totalLatency / totalRequests) << " μs" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string serverIp = "127.0.0.1";
    int serverPort = 8080;
    
    if (argc > 1) serverIp = argv[1];
    if (argc > 2) serverPort = std::stoi(argv[2]);
    
    std::cout << "Socket Server/Client Benchmark Tool" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "Server: " << serverIp << ":" << serverPort << std::endl;
    std::cout << "Starting benchmark tests..." << std::endl;
    
    Benchmark benchmark(serverIp, serverPort);
    
    // Run different benchmark tests
    benchmark.runLatencyTest(1000);
    benchmark.runThroughputTest(5000, 5);
    benchmark.runConcurrentConnectionTest(50);
    benchmark.runStressTest(10, 100); // 10 seconds, 100 req/sec
    
    std::cout << "\n=== Benchmark Complete ===" << std::endl;
    
    return 0;
} 