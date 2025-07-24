#!/bin/bash

echo "Testing Socket Server/Client Architecture"
echo "========================================"

# Check if executables exist
if [ ! -f "./bin/server" ] || [ ! -f "./bin/client" ]; then
    echo "Error: Executables not found. Please run ./build.sh first."
    exit 1
fi

echo "Starting server in background..."
./bin/server 8080 &
SERVER_PID=$!

# Wait for server to start
sleep 2

echo "Server started with PID: $SERVER_PID"
echo ""

echo "Running client tests..."
echo "======================"

# Test 1: Echo service
echo "Test 1: Echo Service"
./bin/client 127.0.0.1 8080 <<< "TOKEN:secret123 ECHO Hello World"
echo ""

# Test 2: Calculator service
echo "Test 2: Calculator Service"
./bin/client 127.0.0.1 8080 <<< "TOKEN:secret123 CAL 2 + 3"
echo ""

# Test 3: File service
echo "Test 3: File Service"
./bin/client 127.0.0.1 8080 <<< "TOKEN:secret123 WRITE test.txt Hello from test script!"
./bin/client 127.0.0.1 8080 <<< "TOKEN:secret123 READ test.txt"
echo ""

# Test 4: Invalid token
echo "Test 4: Invalid Token (should fail)"
./bin/client 127.0.0.1 8080 <<< "TOKEN:wrongtoken ECHO Hello"
echo ""

# Test 5: Invalid command
echo "Test 5: Invalid Command (should fail)"
./bin/client 127.0.0.1 8080 <<< "TOKEN:secret123 INVALID Hello"
echo ""

echo "Tests completed!"
echo "Stopping server..."

# Stop server
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

echo "Server stopped."
echo ""
echo "Test results:"
echo "- Echo service: Should echo back the message"
echo "- Calculator service: Should calculate simple expressions"
echo "- File service: Should write and read files"
echo "- Authentication: Should reject invalid tokens"
echo "- Validation: Should reject invalid commands" 