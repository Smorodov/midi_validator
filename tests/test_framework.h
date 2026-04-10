#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>

namespace midi_test {

struct TestResult {
    std::string testName;
    bool passed;
    std::string message;
    double durationMs;
};

struct TestSuite {
    std::string name;
    std::vector<TestResult> results;
    int totalPassed;
    int totalFailed;
    double totalDuration;
};

class TestFramework {
public:
    TestFramework();
    ~TestFramework();
    
    void addTest(const std::string& name, std::function<bool(std::string&)> testFunc);
    void runAll();
    void printReport() const;
    bool saveReport(const std::string& filename) const;
    int getTotalPassed() const { return totalPassed; }
    int getTotalFailed() const { return totalFailed; }
    
private:
    std::vector<std::pair<std::string, std::function<bool(std::string&)>>> tests;
    std::vector<TestResult> results;
    int totalPassed;
    int totalFailed;
    
    std::string generateTimestamp() const;
};

bool createDirectory(const std::string& path);
bool deleteFile(const std::string& path);
bool fileExists(const std::string& path);
std::string readFile(const std::string& path);
bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data);
std::string getExePath();

}