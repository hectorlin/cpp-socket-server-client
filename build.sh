#!/bin/bash

echo "Building Socket Server/Client with Singleton, Service, and Interceptor Architecture"
echo "================================================================================"

# Check if we have the required tools
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ compiler not found. Please install g++."
    exit 1
fi

# Create build directories
mkdir -p bin obj

echo "Compiling server..."
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude -c src/server.cpp -o obj/server.o
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude -c src/services.cpp -o obj/services.o
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude -c src/interceptors.cpp -o obj/interceptors.o
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude -c src/server_main.cpp -o obj/server_main.o
g++ obj/server.o obj/services.o obj/interceptors.o obj/server_main.o -o bin/server -pthread

echo "Compiling client..."
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude -c src/client.cpp -o obj/client.o
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude -c src/client_main.cpp -o obj/client_main.o
g++ obj/client.o obj/interceptors.o obj/client_main.o -o bin/client -pthread

echo "Compiling benchmark..."
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude -c benchmark.cpp -o obj/benchmark.o
g++ obj/client.o obj/interceptors.o obj/benchmark.o -o bin/benchmark -pthread

echo "Compiling simple benchmark..."
g++ -std=c++11 -Wall -Wextra -O2 -Iinclude -c simple_benchmark.cpp -o obj/simple_benchmark.o
g++ obj/client.o obj/interceptors.o obj/simple_benchmark.o -o bin/simple_benchmark -pthread

echo "Compiling HFT server..."
g++ -std=c++11 -Wall -Wextra -O3 -Iinclude -c src/hft_server.cpp -o obj/hft_server.o
g++ -std=c++11 -Wall -Wextra -O3 -Iinclude -c src/hft_server_main.cpp -o obj/hft_server_main.o
g++ obj/hft_server.o obj/services.o obj/interceptors.o obj/hft_server_main.o -o bin/hft_server -pthread

echo "Compiling HFT benchmark..."
g++ -std=c++11 -Wall -Wextra -O3 -Iinclude -c hft_benchmark.cpp -o obj/hft_benchmark.o
g++ obj/client.o obj/interceptors.o obj/hft_benchmark.o -o bin/hft_benchmark -pthread

echo "Build completed successfully!"
echo ""
echo "Usage:"
echo "  ./bin/server [port]                    - Start server (default port: 8080)"
echo "  ./bin/client [ip] [port] [--interactive] - Start client"
echo "  ./bin/benchmark [ip] [port]            - Run comprehensive performance benchmarks"
echo "  ./bin/simple_benchmark [ip] [port]     - Run simple performance benchmarks"
echo ""
echo "Examples:"
echo "  ./bin/server 8080                      - Start server on port 8080"
echo "  ./bin/client 127.0.0.1 8080            - Run client tests"
echo "  ./bin/client 127.0.0.1 8080 --interactive - Run client in interactive mode"
echo "  ./bin/benchmark 127.0.0.1 8080         - Run comprehensive performance benchmarks"
echo "  ./bin/simple_benchmark 127.0.0.1 8080  - Run simple performance benchmarks"
echo ""
echo "Available commands:"
echo "  TOKEN:secret123 ECHO <message>         - Echo service"
echo "  TOKEN:secret123 CAL <expression>       - Calculator service"
echo "  TOKEN:secret123 READ <filename>        - File read service"
echo "  TOKEN:secret123 WRITE <filename> <content> - File write service" 