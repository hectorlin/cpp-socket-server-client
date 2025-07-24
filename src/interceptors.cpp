#include "../include/interceptors.hpp"
#include <regex>
#include <chrono>

bool LoggingInterceptor::preProcess(std::string& request) {
    startTime = std::chrono::steady_clock::now();
    std::cout << "[LOG] Processing request: " << request << std::endl;
    return true;
}

void LoggingInterceptor::postProcess(const std::string& request, std::string& response) {
    (void)request; // Suppress unused parameter warning
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    std::cout << "[LOG] Request completed in " << duration.count() << "ms" << std::endl;
    std::cout << "[LOG] Response: " << response << std::endl;
}

bool AuthenticationInterceptor::preProcess(std::string& request) {
    // Simple token validation
    std::regex tokenRegex("TOKEN:([^\\s]+)");
    std::smatch match;
    
    if (std::regex_search(request, match, tokenRegex)) {
        std::string token = match[1];
        if (token == validToken) {
            std::cout << "[AUTH] Authentication successful" << std::endl;
            return true;
        }
    }
    
    std::cout << "[AUTH] Authentication failed" << std::endl;
    return false;
}

void AuthenticationInterceptor::postProcess(const std::string& request, std::string& response) {
    (void)request; // Suppress unused parameter warning
    (void)response; // Suppress unused parameter warning
    // Can add response encryption and other post-processing logic here
    std::cout << "[AUTH] Post-processing completed" << std::endl;
}

bool RateLimitingInterceptor::preProcess(std::string& request) {
    (void)request; // Suppress unused parameter warning
    auto now = std::chrono::steady_clock::now();
    auto timeSinceReset = std::chrono::duration_cast<std::chrono::seconds>(now - lastReset);
    
    // Reset counter every minute
    if (timeSinceReset.count() >= 60) {
        currentRequests = 0;
        lastReset = now;
    }
    
    if (currentRequests >= maxRequests) {
        std::cout << "[RATE] Rate limit exceeded (" << currentRequests << "/" << maxRequests << ")" << std::endl;
        return false;
    }
    
    currentRequests++;
    std::cout << "[RATE] Request allowed (" << currentRequests << "/" << maxRequests << ")" << std::endl;
    return true;
}

void RateLimitingInterceptor::postProcess(const std::string& request, std::string& response) {
    (void)request; // Suppress unused parameter warning
    (void)response; // Suppress unused parameter warning
    // Can add rate limiting related response headers here
    std::cout << "[RATE] Rate limiting post-processing completed" << std::endl;
}

bool ValidationInterceptor::preProcess(std::string& request) {
    // Basic request validation
    if (request.empty()) {
        std::cout << "[VALID] Request is empty" << std::endl;
        return false;
    }
    
    if (request.length() > 1000) {
        std::cout << "[VALID] Request too long" << std::endl;
        return false;
    }
    
    // Check for basic command structure (commands come after TOKEN:)
    std::vector<std::string> validCommands = {"ECHO", "CAL", "READ", "WRITE"};
    bool hasValidCommand = false;
    
    for (const auto& cmd : validCommands) {
        if (request.find(cmd) != std::string::npos) {
            hasValidCommand = true;
            break;
        }
    }
    
    if (!hasValidCommand) {
        std::cout << "[VALID] No valid command found in request" << std::endl;
        return false;
    }
    
    std::cout << "[VALID] Request validation passed" << std::endl;
    return true;
}

void ValidationInterceptor::postProcess(const std::string& request, std::string& response) {
    (void)request; // Suppress unused parameter warning
    // Validate response
    if (response.empty()) {
        response = "ERROR: Empty response";
    }
    
    std::cout << "[VALID] Response validation completed" << std::endl;
} 