#include "report_formatter.h"
#include <sstream>

namespace midi {

class SilentFormatter : public IReportFormatter {
public:
    std::string format(const AnalysisResult& result, 
                       const std::string& /*filename*/) override {
        // Silent форматтер не выводит ничего, кроме кода возврата
        // Код возврата обрабатывается в main.cpp
        return "";
    }
    
    std::string getFileExtension() const override {
        return "";
    }
    
    std::string getMimeType() const override {
        return "text/plain";
    }
};

// Silent форматтер зарегистрирован в фабрике

} // namespace midi