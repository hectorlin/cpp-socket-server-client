#include "../include/services.hpp"
#include <iostream>
#include <sstream>
#include <regex>
#include <fstream>
#include <filesystem>

void EchoService::initialize() {
    std::cout << "EchoService initialized" << std::endl;
}

void EchoService::cleanup() {
    std::cout << "EchoService cleaned up" << std::endl;
}

std::string EchoService::processRequest(const std::string& request) {
    if (request.find("ECHO") != std::string::npos) {
        size_t echoPos = request.find("ECHO");
        return "ECHO: " + request.substr(echoPos + 5);
    }
    return ""; // Don't handle this request
}

void CalculatorService::initialize() {
    std::cout << "CalculatorService initialized" << std::endl;
}

void CalculatorService::cleanup() {
    std::cout << "CalculatorService cleaned up" << std::endl;
}

std::string CalculatorService::processRequest(const std::string& request) {
    if (request.find("CAL") != std::string::npos) {
        size_t calPos = request.find("CAL");
        std::string expression = request.substr(calPos + 4);
        try {
            double result = evaluateExpression(expression);
            return "RESULT: " + std::to_string(result);
        } catch (const std::exception& e) {
            return "ERROR: " + std::string(e.what());
        }
    }
    return ""; // Don't handle this request
}

double CalculatorService::evaluateExpression(const std::string& expression) {
    std::istringstream iss(expression);
    double result;
    char op;
    
    iss >> result;
    while (iss >> op >> result) {
        // Simple expression evaluation - can be extended for more complex operations
        switch (op) {
            case '+':
                // result is already the next number
                break;
            case '-':
                result = -result;
                break;
            case '*':
                // Would need to implement proper operator precedence
                break;
            case '/':
                if (result == 0) {
                    throw std::runtime_error("Division by zero");
                }
                // Would need to implement proper operator precedence
                break;
            default:
                throw std::runtime_error("Unknown operator: " + std::string(1, op));
        }
    }
    
    return result;
}

void FileService::initialize() {
    std::cout << "FileService initialized" << std::endl;
}

void FileService::cleanup() {
    std::cout << "FileService cleaned up" << std::endl;
}

std::string FileService::processRequest(const std::string& request) {
    if (request.find("READ") != std::string::npos) {
        size_t readPos = request.find("READ");
        std::string filename = request.substr(readPos + 5);
        return "FILE_CONTENT: " + readFile(filename);
    } else if (request.find("WRITE") != std::string::npos) {
        size_t writePos = request.find("WRITE");
        size_t spacePos = request.find(' ', writePos + 6);
        if (spacePos != std::string::npos) {
            std::string filename = request.substr(writePos + 6, spacePos - (writePos + 6));
            std::string content = request.substr(spacePos + 1);
            if (writeFile(filename, content)) {
                return "SUCCESS: File written successfully";
            } else {
                return "ERROR: Failed to write file";
            }
        }
        return "ERROR: Invalid write command format";
    }
    return ""; // Don't handle this request
}

std::string FileService::readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "ERROR: Could not open file " + filename;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}

bool FileService::writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return true;
} 