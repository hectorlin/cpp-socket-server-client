#!/bin/bash

echo "HFT Socket Server/Client Benchmark"
echo "=================================="

# Kill any existing server processes
echo "Stopping any existing servers..."
pkill -f "hft_server" 2>/dev/null
pkill -f "server" 2>/dev/null
sleep 2

# Create test file for file service
echo "Creating test file..."
echo "This is a test file for HFT benchmarking" > test.txt

# Start HFT server in background
echo "Starting HFT server..."
./bin/hft_server 8080 &
HFT_SERVER_PID=$!

# Wait for server to start
sleep 3

# Check if server started successfully
if ! kill -0 $HFT_SERVER_PID 2>/dev/null; then
    echo "Error: HFT server failed to start"
    exit 1
fi

echo "HFT server started with PID: $HFT_SERVER_PID"

# Run HFT benchmark
echo "Running HFT benchmark..."
./bin/hft_benchmark 127.0.0.1 8080

# Stop server
echo "Stopping HFT server..."
kill $HFT_SERVER_PID 2>/dev/null
wait $HFT_SERVER_PID 2>/dev/null

echo "HFT benchmark completed!" 