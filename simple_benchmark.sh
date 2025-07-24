#!/bin/bash

echo "Simple Socket Server/Client Performance Benchmark"
echo "================================================"

# Check if executables exist
if [ ! -f "./bin/server" ] || [ ! -f "./bin/simple_benchmark" ]; then
    echo "Error: Executables not found. Please run ./build.sh first."
    exit 1
fi

# Check if server is already running
if pgrep -f "bin/server" > /dev/null; then
    echo "Server is already running. Using existing server."
else
    echo "Starting server in background..."
    ./bin/server 8080 &
    SERVER_PID=$!
    
    # Wait for server to start
    sleep 3
    
    echo "Server started with PID: $SERVER_PID"
fi

echo ""
echo "Running simple performance benchmarks..."
echo "======================================="

# Run simple benchmark tests
./bin/simple_benchmark 127.0.0.1 8080

echo ""
echo "Simple benchmark tests completed!"

# Stop server if we started it
if [ ! -z "$SERVER_PID" ]; then
    echo "Stopping server..."
    kill $SERVER_PID
    wait $SERVER_PID 2>/dev/null
    echo "Server stopped."
fi

echo ""
echo "Simple Benchmark Summary:"
echo "- Basic Test: 100 sequential ECHO requests"
echo "- Multi-Service Test: All services (ECHO, CAL, READ, WRITE)"
echo "- Concurrent Test: 5 threads, 20 requests each"
echo ""
echo "Results show the performance characteristics of the socket server"
echo "with singleton, service, and interceptor architecture patterns." 