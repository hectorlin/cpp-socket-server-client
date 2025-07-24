#include "../include/client.hpp"
#include "../include/interceptors.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <random>
#include <algorithm>
#include <mutex>

class HFTBenchmark {
private:
    std::string serverIp;
    int serverPort;
    std::atomic<uint64_t> totalRequests{0};
    std::atomic<uint64_t> totalLatency{0};
    std::atomic<uint64_t> failedRequests{0};
    std::vector<uint64_t> latencies;
    std::mutex latenciesMutex;

public:
    HFTBenchmark(const std::string& ip, int port) : serverIp(ip), serverPort(port) {}

    void runLatencyTest(int numRequests) {
        std::cout << "\n=== HFT Latency Test ===" << std::endl;
        std::cout << "Testing " << numRequests << " ultra-low latency requests..." << std::endl;
        
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Failed to connect for HFT latency test" << std::endl;
            return;
        }

        latencies.clear();
        latencies.reserve(numRequests);
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numRequests; ++i) {
            auto requestStart = std::chrono::high_resolution_clock::now();
            
            std::string request = "TOKEN:secret123 ECHO HFT_" + std::to_string(i);
            std::string response = client.sendRequest(request);
            
            auto requestEnd = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(requestEnd - requestStart).count();
            
            latencies.push_back(latency);
            totalRequests++;
            totalLatency += latency;
            
            if (response.find("ECHO:") == std::string::npos) {
                failedRequests++;
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        
        printHFTResults(numRequests, totalTime);
        client.disconnect();
    }

    void runThroughputTest(int numRequests, int numThreads) {
        std::cout << "\n=== HFT Throughput Test ===" << std::endl;
        std::cout << "Testing " << numRequests << " requests with " << numThreads << " threads..." << std::endl;
        
        totalRequests = 0;
        totalLatency = 0;
        failedRequests = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        int requestsPerThread = numRequests / numThreads;
        
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(&HFTBenchmark::hftWorker, this, requestsPerThread, i);
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        
        printHFTThroughputResults(numRequests, totalTime, numThreads);
    }

    void runStressTest(int durationSeconds, int requestsPerSecond) {
        std::cout << "\n=== HFT Stress Test ===" << std::endl;
        std::cout << "Running for " << durationSeconds << " seconds at " << requestsPerSecond << " req/sec..." << std::endl;
        
        totalRequests = 0;
        totalLatency = 0;
        failedRequests = 0;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        auto endTime = startTime + std::chrono::seconds(durationSeconds);
        
        std::vector<std::thread> threads;
        int numThreads = std::min(32, requestsPerSecond / 100); // More threads for HFT
        int requestsPerThread = requestsPerSecond / numThreads;
        
        for (int i = 0; i < numThreads; ++i) {
            threads.emplace_back(&HFTBenchmark::hftStressWorker, this, requestsPerThread, endTime, i);
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto actualEndTime = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::microseconds>(actualEndTime - startTime).count();
        
        printHFTStressResults(totalTime);
    }

    void runMicrosecondTest(int numRequests) {
        std::cout << "\n=== HFT Microsecond Precision Test ===" << std::endl;
        std::cout << "Testing " << numRequests << " requests with microsecond precision..." << std::endl;
        
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Failed to connect for microsecond test" << std::endl;
            return;
        }

        std::vector<uint64_t> microLatencies;
        microLatencies.reserve(numRequests);
        
        for (int i = 0; i < numRequests; ++i) {
            auto requestStart = std::chrono::high_resolution_clock::now();
            
            std::string request = "TOKEN:secret123 ECHO MICRO_" + std::to_string(i);
            std::string response = client.sendRequest(request);
            
            auto requestEnd = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::microseconds>(requestEnd - requestStart).count();
            
            microLatencies.push_back(latency);
        }
        
        printMicrosecondResults(microLatencies);
        client.disconnect();
    }

private:
    void hftWorker(int numRequests, int threadId) {
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Thread " << threadId << " failed to connect" << std::endl;
            return;
        }
        
        for (int i = 0; i < numRequests; ++i) {
            auto requestStart = std::chrono::high_resolution_clock::now();
            
            std::string request = "TOKEN:secret123 ECHO HFT_THREAD_" + std::to_string(threadId) + "_" + std::to_string(i);
            std::string response = client.sendRequest(request);
            
            auto requestEnd = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(requestEnd - requestStart).count();
            
            totalRequests++;
            totalLatency += latency;
            
            if (response.find("ECHO:") == std::string::npos) {
                failedRequests++;
            }
        }
        
        client.disconnect();
    }

    void hftStressWorker(int requestsPerThread, std::chrono::high_resolution_clock::time_point endTime, int threadId) {
        (void)requestsPerThread; // Suppress unused parameter warning
        SocketClient client(serverIp, serverPort);
        if (!client.connect()) {
            std::cerr << "Stress thread " << threadId << " failed to connect" << std::endl;
            return;
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 2);
        
        while (std::chrono::high_resolution_clock::now() < endTime) {
            auto requestStart = std::chrono::high_resolution_clock::now();
            
            std::string request;
            int requestType = dis(gen);
            
            switch (requestType) {
                case 0:
                    request = "TOKEN:secret123 ECHO HFT_STRESS";
                    break;
                case 1:
                    request = "TOKEN:secret123 CAL 1 + 1";
                    break;
                case 2:
                    request = "TOKEN:secret123 READ test.txt";
                    break;
            }
            
            std::string response = client.sendRequest(request);
            
            auto requestEnd = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(requestEnd - requestStart).count();
            
            totalRequests++;
            totalLatency += latency;
            
            if (response.find("ERROR:") != std::string::npos) {
                failedRequests++;
            }
        }
        
        client.disconnect();
    }

    void printHFTResults(int numRequests, long long totalTime) {
        if (latencies.empty()) return;
        
        std::sort(latencies.begin(), latencies.end());
        
        uint64_t minLatency = latencies.front();
        uint64_t maxLatency = latencies.back();
        uint64_t avgLatency = totalLatency / numRequests;
        uint64_t medianLatency = latencies[latencies.size() / 2];
        uint64_t p95Latency = latencies[static_cast<size_t>(latencies.size() * 0.95)];
        uint64_t p99Latency = latencies[static_cast<size_t>(latencies.size() * 0.99)];
        uint64_t p999Latency = latencies[static_cast<size_t>(latencies.size() * 0.999)];
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "HFT Latency Results (nanoseconds):" << std::endl;
        std::cout << "  Total Time: " << totalTime << " μs" << std::endl;
        std::cout << "  Requests: " << numRequests << std::endl;
        std::cout << "  Success Rate: " << ((numRequests - failedRequests) * 100.0 / numRequests) << "%" << std::endl;
        std::cout << "  Throughput: " << (numRequests * 1000000.0 / totalTime) << " req/sec" << std::endl;
        std::cout << "  Min Latency: " << minLatency << " ns" << std::endl;
        std::cout << "  Max Latency: " << maxLatency << " ns" << std::endl;
        std::cout << "  Avg Latency: " << avgLatency << " ns" << std::endl;
        std::cout << "  Median Latency: " << medianLatency << " ns" << std::endl;
        std::cout << "  95th Percentile: " << p95Latency << " ns" << std::endl;
        std::cout << "  99th Percentile: " << p99Latency << " ns" << std::endl;
        std::cout << "  99.9th Percentile: " << p999Latency << " ns" << std::endl;
    }

    void printHFTThroughputResults(int numRequests, long long totalTime, int numThreads) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "HFT Throughput Results:" << std::endl;
        std::cout << "  Total Time: " << totalTime << " μs" << std::endl;
        std::cout << "  Requests: " << numRequests << std::endl;
        std::cout << "  Threads: " << numThreads << std::endl;
        std::cout << "  Success Rate: " << ((numRequests - failedRequests) * 100.0 / numRequests) << "%" << std::endl;
        std::cout << "  Throughput: " << (numRequests * 1000000.0 / totalTime) << " req/sec" << std::endl;
        std::cout << "  Avg Latency: " << (totalLatency / numRequests) << " ns" << std::endl;
    }

    void printHFTStressResults(long long totalTime) {
        uint64_t totalRequestsCount = totalRequests.load();
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "HFT Stress Test Results:" << std::endl;
        std::cout << "  Total Time: " << totalTime << " μs" << std::endl;
        std::cout << "  Total Requests: " << totalRequestsCount << std::endl;
        std::cout << "  Success Rate: " << ((totalRequestsCount - failedRequests) * 100.0 / totalRequestsCount) << "%" << std::endl;
        std::cout << "  Throughput: " << (totalRequestsCount * 1000000.0 / totalTime) << " req/sec" << std::endl;
        std::cout << "  Avg Latency: " << (totalLatency / totalRequestsCount) << " ns" << std::endl;
    }

    void printMicrosecondResults(const std::vector<uint64_t>& microLatencies) {
        if (microLatencies.empty()) return;
        
        std::vector<uint64_t> sorted = microLatencies;
        std::sort(sorted.begin(), sorted.end());
        
        uint64_t minLatency = sorted.front();
        uint64_t maxLatency = sorted.back();
        uint64_t avgLatency = 0;
        for (auto latency : microLatencies) {
            avgLatency += latency;
        }
        avgLatency /= microLatencies.size();
        
        uint64_t medianLatency = sorted[sorted.size() / 2];
        uint64_t p95Latency = sorted[static_cast<size_t>(sorted.size() * 0.95)];
        uint64_t p99Latency = sorted[static_cast<size_t>(sorted.size() * 0.99)];
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Microsecond Precision Results:" << std::endl;
        std::cout << "  Requests: " << microLatencies.size() << std::endl;
        std::cout << "  Min Latency: " << minLatency << " μs" << std::endl;
        std::cout << "  Max Latency: " << maxLatency << " μs" << std::endl;
        std::cout << "  Avg Latency: " << avgLatency << " μs" << std::endl;
        std::cout << "  Median Latency: " << medianLatency << " μs" << std::endl;
        std::cout << "  95th Percentile: " << p95Latency << " μs" << std::endl;
        std::cout << "  99th Percentile: " << p99Latency << " μs" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::string serverIp = "127.0.0.1";
    int serverPort = 8080;
    
    if (argc > 1) serverIp = argv[1];
    if (argc > 2) serverPort = std::stoi(argv[2]);
    
    std::cout << "HFT Socket Server/Client Benchmark Tool" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "Server: " << serverIp << ":" << serverPort << std::endl;
    std::cout << "Starting HFT benchmark tests..." << std::endl;
    
    HFTBenchmark benchmark(serverIp, serverPort);
    
    // Run HFT-specific benchmark tests
    benchmark.runLatencyTest(10000);           // 10K requests for latency
    benchmark.runThroughputTest(100000, 16);   // 100K requests with 16 threads
    benchmark.runStressTest(30, 10000);        // 30 seconds at 10K req/sec
    benchmark.runMicrosecondTest(5000);        // 5K requests with μs precision
    
    std::cout << "\n=== HFT Benchmark Complete ===" << std::endl;
    
    return 0;
} 