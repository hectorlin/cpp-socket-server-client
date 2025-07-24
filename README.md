# C++11 Socket Server/Client with Singleton, Service & Interceptor Architecture

A complete implementation of a socket-based server and client using C++11, featuring three key design patterns: Singleton, Service Layer, and Interceptor patterns.

## 🚀 Quick Start

### Build
```bash
# Option 1: Using build script (recommended)
./build.sh

# Option 2: Using Make
make

# Option 3: Using CMake
mkdir build && cd build
cmake ..
make
```

### Run
```bash
# Terminal 1: Start server
./bin/server 8080

# Terminal 2: Run client tests
./bin/client 127.0.0.1 8080

# Terminal 2: Or run interactive client
./bin/client 127.0.0.1 8080 --interactive
```

### Test
```bash
./test.sh
```

### Benchmark
```bash
# Run simple performance benchmarks
./simple_benchmark.sh

# Run comprehensive performance benchmarks
./benchmark.sh
```

## 📁 Project Structure

```
cpp_SocketSvr/
├── include/                 # Header files
│   ├── interfaces.hpp      # Base interfaces
│   ├── server.hpp         # Server singleton
│   ├── client.hpp         # Client class
│   ├── services.hpp       # Service implementations
│   └── interceptors.hpp   # Interceptor implementations
├── src/                   # Source files
│   ├── server.cpp         # Server implementation
│   ├── client.cpp         # Client implementation
│   ├── services.cpp       # Service layer
│   ├── interceptors.cpp   # Interceptor layer
│   ├── server_main.cpp    # Server entry point
│   └── client_main.cpp    # Client entry point
├── build.sh              # Build script
├── test.sh               # Test script
├── benchmark.sh          # Comprehensive benchmark script
├── simple_benchmark.sh   # Simple benchmark script
├── benchmark.cpp         # Comprehensive benchmark tool
├── simple_benchmark.cpp  # Simple benchmark tool
├── Makefile              # Make build
└── CMakeLists.txt        # CMake build
```

## 🏗️ Architecture Overview

### Singleton Pattern - Server
```cpp
class SocketServer {
private:
    static SocketServer* instance;
    static std::mutex mutex;
    
public:
    static SocketServer* getInstance();
    void start(int port);
    void stop();
};
```

### Service Layer Pattern
```cpp
class IService {
public:
    virtual std::string processRequest(const std::string& request) = 0;
    virtual void initialize() = 0;
    virtual void cleanup() = 0;
};

// Implementations:
// - EchoService: Echo back messages
// - CalculatorService: Simple math operations
// - FileService: File read/write operations
```

### Interceptor Pattern
```cpp
class IInterceptor {
public:
    virtual bool preProcess(std::string& request) = 0;
    virtual void postProcess(const std::string& request, std::string& response) = 0;
    virtual int getPriority() const = 0;
};

// Implementations:
// - LoggingInterceptor: Request/response logging
// - AuthenticationInterceptor: Token-based auth
// - RateLimitingInterceptor: Request rate limiting
// - ValidationInterceptor: Input validation
```

## 📋 Available Commands

| Command | Description | Example |
|---------|-------------|---------|
| `TOKEN:secret123 ECHO <message>` | Echo service | `TOKEN:secret123 ECHO Hello World` |
| `TOKEN:secret123 CAL <expression>` | Calculator service | `TOKEN:secret123 CAL 2 + 3` |
| `TOKEN:secret123 READ <filename>` | Read file | `TOKEN:secret123 READ test.txt` |
| `TOKEN:secret123 WRITE <filename> <content>` | Write file | `TOKEN:secret123 WRITE test.txt Hello` |

## 🔧 Features

### Core Features
- ✅ **Thread-safe singleton server**
- ✅ **Multi-threaded client handling**
- ✅ **Extensible service architecture**
- ✅ **Interceptor chain processing**
- ✅ **Token-based authentication**
- ✅ **Request rate limiting**
- ✅ **Input validation**
- ✅ **Comprehensive logging**

### Technical Features
- **C++11 Standard**: Modern C++ features
- **RAII**: Resource management
- **Smart Pointers**: Memory safety
- **Exception Handling**: Robust error handling
- **Thread Safety**: Concurrent access protection
- **Socket Programming**: Network communication

## 🎯 Design Patterns Used

1. **Singleton Pattern**
   - Ensures single server instance
   - Thread-safe implementation
   - Global access point

2. **Service Layer Pattern**
   - Business logic encapsulation
   - Pluggable service architecture
   - Clear separation of concerns

3. **Interceptor Pattern**
   - Cross-cutting concerns
   - Pre/post processing
   - Chain of responsibility

## 📊 Example Output

### Server Output
```
Starting Socket Server with Singleton, Service, and Interceptor Architecture
==================================================================
[SETUP] Adding services...
EchoService initialized
CalculatorService initialized
FileService initialized
[SETUP] Adding interceptors...
Server started on port 8080
New connection from 127.0.0.1
[VALID] Request validation passed
[AUTH] Authentication successful
[RATE] Request allowed (1/100)
[LOG] Processing request: TOKEN:secret123 ECHO Hello World
[LOG] Request completed in 2ms
[LOG] Response: ECHO: Hello World
```

### Client Output
```
Socket Client with Interceptor Architecture
===========================================
[SETUP] Adding client interceptors...
[INFO] Connecting to server 127.0.0.1:8080
Connected to server 127.0.0.1:8080
[CLIENT] Sending: TOKEN:secret123 ECHO Hello World
[CLIENT] Received: ECHO: Hello World
```

## 🛠️ Building Options

### Using Build Script (Recommended)
```bash
./build.sh
```

### Using Make
```bash
make all          # Build both server and client
make clean        # Clean build files
make run-server   # Run server
make run-client   # Run client
make test         # Run tests
```

### Using CMake
```bash
mkdir build && cd build
cmake ..
make
```

## 🧪 Testing

### Automated Tests
```bash
./test.sh
```

### Manual Testing
```bash
# Terminal 1
./bin/server 8080

# Terminal 2
./bin/client 127.0.0.1 8080 --interactive
```

### Performance Benchmarking
```bash
# Run simple benchmarks (recommended)
./simple_benchmark.sh

# Run comprehensive benchmarks
./benchmark.sh

# Direct benchmark execution
./bin/simple_benchmark 127.0.0.1 8080
./bin/benchmark 127.0.0.1 8080
```

### Test Commands
```
TOKEN:secret123 ECHO Hello World
TOKEN:secret123 CAL 2 + 3
TOKEN:secret123 WRITE test.txt Hello from client!
TOKEN:secret123 READ test.txt
```

## 🔍 Code Examples

### Adding a New Service
```cpp
class CustomService : public IService {
public:
    void initialize() override {
        std::cout << "CustomService initialized" << std::endl;
    }
    
    void cleanup() override {
        std::cout << "CustomService cleaned up" << std::endl;
    }
    
    std::string processRequest(const std::string& request) override {
        if (request.substr(0, 6) == "CUSTOM") {
            return "Custom response: " + request.substr(7);
        }
        return "";
    }
};

// Register in server
server->addService(std::make_unique<CustomService>());
```

### Adding a New Interceptor
```cpp
class CustomInterceptor : public IInterceptor {
public:
    bool preProcess(std::string& request) override {
        // Pre-processing logic
        return true;
    }
    
    void postProcess(const std::string& request, std::string& response) override {
        // Post-processing logic
    }
    
    int getPriority() const override {
        return 5;
    }
};

// Register in server/client
server->addInterceptor(std::make_unique<CustomInterceptor>());
```

## 🚨 Error Handling

The implementation includes comprehensive error handling:

- **Network errors**: Connection failures, socket errors
- **Authentication errors**: Invalid tokens
- **Validation errors**: Malformed requests
- **Rate limiting**: Too many requests
- **Service errors**: Unavailable services

## 📈 Performance Considerations

- **Thread pool**: Efficient client handling
- **Memory management**: Smart pointers prevent leaks
- **Rate limiting**: Prevents server overload
- **Connection reuse**: Efficient socket management

## 📊 Performance Benchmarks

The project includes comprehensive performance benchmarking tools:

### Simple Benchmark Results
```
=== Basic Performance Test ===
Test Results:
  Total Time: 37 ms
  Requests: 100
  Success Rate: 100.00%
  Throughput: 2702.70 req/sec
  Min Latency: 64 μs
  Max Latency: 4156 μs
  Avg Latency: 378 μs
  Median Latency: 270 μs

=== Concurrent Connection Test ===
Concurrent Test Results:
  Total Time: 19 ms
  Total Requests: 100
  Threads: 5
  Success Rate: 100.00%
  Throughput: 5263.16 req/sec
  Avg Latency: 803 μs
```

### Benchmark Features
- **Latency Testing**: Measures request/response times
- **Throughput Testing**: Requests per second
- **Concurrent Testing**: Multiple simultaneous connections
- **Multi-Service Testing**: All service types
- **Stress Testing**: Sustained load testing

## 🔒 Security Features

- **Token authentication**: Secure access control
- **Input validation**: Prevents malicious input
- **Rate limiting**: Prevents abuse
- **Error sanitization**: No sensitive information leakage

## 📝 Requirements

- **Compiler**: GCC 4.8+ or Clang 3.3+
- **C++ Standard**: C++11
- **OS**: Linux (tested on Fedora)
- **Libraries**: pthread (included)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Add your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is open source and available under the MIT License.

---

**Built with ❤️ using C++11 and modern design patterns** 