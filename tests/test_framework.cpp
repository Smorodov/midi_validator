#include "test_framework.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <ctime>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

namespace midi_test {

TestFramework::TestFramework() : totalPassed(0), totalFailed(0) {
}

TestFramework::~TestFramework() {
}

void TestFramework::addTest(const std::string& name, std::function<bool(std::string&)> testFunc) {
    tests.emplace_back(name, testFunc);
}

void TestFramework::runAll() {
    results.clear();
    totalPassed = 0;
    totalFailed = 0;
    
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "MIDI VALIDATOR TEST SUITE\n";
    std::cout << "========================================\n";
    std::cout << "Running " << tests.size() << " tests...\n";
    std::cout << "========================================\n\n";
    
    for (const auto& [name, testFunc] : tests) {
        TestResult result;
        result.testName = name;
        result.passed = false;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            std::string errorMsg;
            result.passed = testFunc(errorMsg);
            result.message = errorMsg;
        } catch (const std::exception& e) {
            result.passed = false;
            result.message = "Exception: " + std::string(e.what());
        } catch (...) {
            result.passed = false;
            result.message = "Unknown exception";
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
        
        results.push_back(result);
        
        if (result.passed) {
            totalPassed++;
            std::cout << "[PASS] " << name << " (" << std::fixed << std::setprecision(2) << result.durationMs << " ms)\n";
        } else {
            totalFailed++;
            std::cout << "[FAIL] " << name << " (" << std::fixed << std::setprecision(2) << result.durationMs << " ms)\n";
            std::cout << "       Reason: " << result.message << "\n";
        }
    }
    
    printReport();
}

void TestFramework::printReport() const {
    std::cout << "\n========================================\n";
    std::cout << "TEST SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "Total tests: " << (totalPassed + totalFailed) << "\n";
    std::cout << "Passed: " << totalPassed << " (" << std::fixed << std::setprecision(1) 
              << (100.0 * totalPassed / (totalPassed + totalFailed)) << "%)\n";
    std::cout << "Failed: " << totalFailed << "\n";
    std::cout << "========================================\n";
}

bool TestFramework::saveReport(const std::string& filename) const {
    std::ofstream report(filename);
    if (!report.is_open()) {
        return false;
    }
    
    report << "MIDI Validator Test Report\n";
    report << "Generated: " << generateTimestamp() << "\n";
    report << "========================================\n\n";
    
    for (const auto& result : results) {
        report << (result.passed ? "[PASS]" : "[FAIL]") << " " << result.testName << "\n";
        if (!result.passed) {
            report << "  Reason: " << result.message << "\n";
        }
    }
    
    report << "\n========================================\n";
    report << "Summary:\n";
    report << "  Total: " << (totalPassed + totalFailed) << "\n";
    report << "  Passed: " << totalPassed << "\n";
    report << "  Failed: " << totalFailed << "\n";
    report << "========================================\n";
    
    report.close();
    return true;
}

std::string TestFramework::generateTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool createDirectory(const std::string& path) {
    std::error_code ec;
    return std::filesystem::create_directories(path, ec);
}

bool deleteFile(const std::string& path) {
    std::error_code ec;
    return std::filesystem::remove(path, ec);
}

bool fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
    return true;
}

std::string getExePath() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    return path.substr(0, path.find_last_of("\\/"));
#else
    return ".";
#endif
}

}