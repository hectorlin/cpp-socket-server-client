#pragma once
#include "interfaces.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <string.h>
#include <chrono>
#include <mutex>

// HFT-optimized buffer sizes
#define HFT_BUFFER_SIZE 4096
#define HFT_MAX_EVENTS 10000
#define HFT_THREAD_POOL_SIZE 16

// Pre-allocated response buffers for common operations
struct HFTResponseBuffer {
    char data[HFT_BUFFER_SIZE];
    size_t length;
    
    HFTResponseBuffer() : length(0) {
        memset(data, 0, HFT_BUFFER_SIZE);
    }
    
    void reset() {
        length = 0;
        memset(data, 0, HFT_BUFFER_SIZE);
    }
};

// Lock-free request queue for HFT
template<typename T>
class LockFreeQueue {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        Node() : next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    std::vector<std::unique_ptr<Node>> pool;
    std::atomic<size_t> pool_index{0};
    
public:
    LockFreeQueue(size_t pool_size = 10000) {
        pool.resize(pool_size);
        for (auto& node : pool) {
            node = std::unique_ptr<Node>(new Node());
        }
        head = tail = pool[0].get();
    }
    
    bool enqueue(const T& item) {
        Node* node = get_node();
        if (!node) return false;
        
        node->data = item;
        node->next = nullptr;
        
        Node* old_tail = tail.exchange(node);
        old_tail->next = node;
        return true;
    }
    
    bool dequeue(T& item) {
        Node* old_head = head.load();
        Node* new_head = old_head->next.load();
        
        if (!new_head) return false;
        
        item = new_head->data;
        head = new_head;
        return true;
    }
    
private:
    Node* get_node() {
        size_t index = pool_index.fetch_add(1) % pool.size();
        return pool[index].get();
    }
};

// HFT-optimized server
class HFTServer {
private:
    static HFTServer* instance;
    static std::mutex mutex;
    
    int serverSocket;
    int epollFd;
    std::atomic<bool> running;
    std::vector<std::thread> workerThreads;
    std::vector<std::unique_ptr<IService>> services;
    std::vector<std::unique_ptr<IInterceptor>> interceptors;
    
    // HFT optimizations
    LockFreeQueue<std::pair<int, std::string>> requestQueue;
    std::vector<HFTResponseBuffer> responseBuffers;
    std::atomic<size_t> bufferIndex{0};
    
    // Performance metrics
    std::atomic<uint64_t> totalRequests{0};
    std::atomic<uint64_t> totalLatency{0};
    std::chrono::high_resolution_clock::time_point startTime;
    
    HFTServer();
    ~HFTServer();
    HFTServer(const HFTServer&) = delete;
    HFTServer& operator=(const HFTServer&) = delete;
    
    void setupEpoll();
    void acceptConnections();
    void handleClient(int clientSocket);
    std::string processRequest(const std::string& request);
    void workerThread();
    HFTResponseBuffer* getResponseBuffer();
    void setNonBlocking(int sock);
    
public:
    static HFTServer* getInstance();
    void start(int port);
    void stop();
    void addService(std::unique_ptr<IService> service);
    void addInterceptor(std::unique_ptr<IInterceptor> interceptor);
    
    // HFT-specific methods
    uint64_t getTotalRequests() const { return totalRequests.load(); }
    uint64_t getAverageLatency() const { 
        auto requests = totalRequests.load();
        return requests > 0 ? totalLatency.load() / requests : 0;
    }
    void resetMetrics();
}; 