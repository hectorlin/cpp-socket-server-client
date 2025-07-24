#pragma once
#include <string>

// Service Layer Interface
class IService {
public:
    virtual ~IService() = default;
    virtual std::string processRequest(const std::string& request) = 0;
    virtual void initialize() = 0;
    virtual void cleanup() = 0;
};

// Interceptor Interface
class IInterceptor {
public:
    virtual ~IInterceptor() = default;
    virtual bool preProcess(std::string& request) = 0;
    virtual void postProcess(const std::string& request, std::string& response) = 0;
    virtual int getPriority() const = 0;
}; 