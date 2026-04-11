#ifndef REPORT_FORMATTER_H
#define REPORT_FORMATTER_H

#include "midi_types.h"
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace midi {

// ============================================================================
// Интерфейс для всех форматтеров отчета
// ============================================================================

class IReportFormatter {
public:
    virtual ~IReportFormatter() = default;
    
    virtual std::string format(const AnalysisResult& result, 
                               const std::string& filename) = 0;
    
    virtual std::string getFileExtension() const { return ""; }
    virtual std::string getMimeType() const { return "text/plain"; }
};

// ============================================================================
// Типы форматтеров
// ============================================================================

enum class ReportFormat {
    Console,
    JSON,
    Silent,
    CSV,
    HTML,
    XML
};

// ============================================================================
// Фабрика для создания форматтеров
// ============================================================================

class FormatterFactory {
public:
    static std::unique_ptr<IReportFormatter> create(ReportFormat format);
    static std::unique_ptr<IReportFormatter> create(const std::string& formatName);
    static std::vector<std::string> getAvailableFormats();
};

} // namespace midi

#endif // REPORT_FORMATTER_H