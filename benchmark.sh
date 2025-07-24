#!/bin/bash

echo "Socket Server/Client Performance Benchmark"
echo "========================================="

# Check if executables exist
if [ ! -f "./bin/server" ] || [ ! -f "./bin/benchmark" ]; then
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
echo "Running comprehensive performance benchmarks..."
echo "=============================================="

# Run benchmark tests
./bin/benchmark 127.0.0.1 8080

echo ""
echo "Benchmark tests completed!"

# Stop server if we started it
if [ ! -z "$SERVER_PID" ]; then
    echo "Stopping server..."
    kill $SERVER_PID
    wait $SERVER_PID 2>/dev/null
    echo "Server stopped."
fi

echo ""
echo "Benchmark Summary:"
echo "- Latency Test: Measures request/response latency"
echo "- Throughput Test: Measures requests per second"
echo "- Concurrent Connection Test: Tests multiple simultaneous connections"
echo "- Stress Test: Sustained load testing"
echo ""
echo "Results show the performance characteristics of the socket server"
echo "with singleton, service, and interceptor architecture patterns." 