#pragma once
#include "interfaces.hpp"
#include <string>
#include <chrono>
#include <iostream>
#include <regex>

class LoggingInterceptor : public IInterceptor {
public:
    bool preProcess(std::string& request) override;
    void postProcess(const std::string& request, std::string& response) override;
    int getPriority() const override { return 1; }
    
private:
    std::chrono::steady_clock::time_point startTime;
};

class AuthenticationInterceptor : public IInterceptor {
private:
    std::string validToken;
    
public:
    AuthenticationInterceptor(const std::string& token) : validToken(token) {}
    
    bool preProcess(std::string& request) override;
    void postProcess(const std::string& request, std::string& response) override;
    int getPriority() const override { return 0; }
};

class RateLimitingInterceptor : public IInterceptor {
private:
    int maxRequests;
    int currentRequests;
    std::chrono::steady_clock::time_point lastReset;
    
public:
    RateLimitingInterceptor(int max) : maxRequests(max), currentRequests(0) {
        lastReset = std::chrono::steady_clock::now();
    }
    
    bool preProcess(std::string& request) override;
    void postProcess(const std::string& request, std::string& response) override;
    int getPriority() const override { return 2; }
};

class ValidationInterceptor : public IInterceptor {
public:
    bool preProcess(std::string& request) override;
    void postProcess(const std::string& request, std::string& response) override;
    int getPriority() const override { return 3; }
}; 