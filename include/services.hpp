#pragma once
#include "interfaces.hpp"
#include <string>
#include <map>

class EchoService : public IService {
public:
    void initialize() override;
    void cleanup() override;
    std::string processRequest(const std::string& request) override;
};

class CalculatorService : public IService {
private:
    std::map<std::string, double> variables;
    
public:
    void initialize() override;
    void cleanup() override;
    std::string processRequest(const std::string& request) override;
    
private:
    double evaluateExpression(const std::string& expression);
};

class FileService : public IService {
public:
    void initialize() override;
    void cleanup() override;
    std::string processRequest(const std::string& request) override;
    
private:
    std::string readFile(const std::string& filename);
    bool writeFile(const std::string& filename, const std::string& content);
}; 