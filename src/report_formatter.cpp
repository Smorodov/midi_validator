#include "report_formatter.h"
#include <sstream>
#include <iomanip>
#include <map>

namespace midi {

// Forward declarations of formatter classes
class ConsoleFormatter;
class JsonFormatter;
class SilentFormatter;

// ============================================================================
// Console Formatter Implementation
// ============================================================================

class ConsoleFormatter : public IReportFormatter {
public:
    std::string format(const AnalysisResult& result, const std::string& filename) override {
        std::stringstream ss;
        
        ss << "\n+--------------------------------------------------+\n";
        ss << "|     MIDI VALIDATOR v7.0.0                        |\n";
        ss << "+--------------------------------------------------+\n";
        
        // File Header
        ss << "\n+--------------------------------------------------+\n";
        ss << "| FILE HEADER                                      |\n";
        ss << "+--------------------------------------------------+\n";
        ss << "| Filename:        " << truncateFilename(filename, 34) << "\n";
        ss << "| Format:          " << result.rawData.format;
        if (result.rawData.format == 0) ss << " (single track)\n";
        else if (result.rawData.format == 1) ss << " (multi-track)\n";
        else ss << "\n";
        ss << "| Tracks declared: " << std::setw(34) << result.rawData.numTracks << "\n";
        ss << "| Tracks found:    " << std::setw(34) << result.tracksFound;
        if (result.tracksFound != result.rawData.numTracks && result.rawData.numTracks > 0) {
            ss << " [WARNING!]";
        }
        ss << "\n";
        ss << "| Division:        ";
        if (result.rawData.isSMPTE) {
            ss << "SMPTE " << std::abs(result.rawData.smpteFrames) 
               << " fps, " << (int)result.rawData.smpteResolution << " subframes\n";
        } else {
            ss << result.rawData.getTicksPerQuarterNote() << " ticks per quarter note\n";
        }
        ss << "| File size:       " << std::setw(34) << result.rawData.fileSize << " bytes\n";
        ss << "+--------------------------------------------------+\n";
        
        // Track Info
        ss << "\n+--------------------------------------------------+\n";
        ss << "| TRACK INFORMATION                                |\n";
        ss << "+--------------------------------------------------+\n";
        ss << "| Tracks with EOT: " << result.tracksWithEOT;
        if (result.tracksWithEOT != result.tracksFound && result.tracksFound > 0) {
            ss << " [WARNING: " << (result.tracksFound - result.tracksWithEOT) << " missing]";
        }
        ss << "\n+--------------------------------------------------+\n";
        
        // Event Statistics
        ss << "\n+--------------------------------------------------+\n";
        ss << "| EVENT STATISTICS                                 |\n";
        ss << "+--------------------------------------------------+\n";
        ss << "| Total events:    " << std::setw(34) << result.totalEvents << "\n";
        ss << "| Note On events:  " << std::setw(34) << result.noteOnCount << "\n";
        ss << "| Note Off events: " << std::setw(34) << result.noteOffCount << "\n";
        ss << "| Control Change:  " << std::setw(34) << result.controlChangeCount << "\n";
        ss << "| Program Change:  " << std::setw(34) << result.programChangeCount << "\n";
        ss << "| Pitch Bend:      " << std::setw(34) << result.pitchBendCount << "\n";
        
        if (result.hasNotes) {
            ss << "| Note range:      " << std::setw(34);
            ss << (int)result.minNote << " - " << (int)result.maxNote << "\n";
        }
        
        // Duration
        int minutes = static_cast<int>(result.durationSeconds) / 60;
        int seconds = static_cast<int>(result.durationSeconds) % 60;
        ss << "| Duration:        " << std::setw(34);
        ss << minutes << "m " << seconds << "s\n";
        ss << "| Max polyphony:   " << std::setw(34) 
           << result.maxPolyphony << " simultaneous notes\n";
        ss << "+--------------------------------------------------+\n";
        
        // Validation
        ss << "\n+--------------------------------------------------+\n";
        ss << "| INTEGRITY CHECKS                                 |\n";
        ss << "+--------------------------------------------------+\n";
        
        if (result.isValid) {
            ss << "| [OK] All integrity checks passed\n";
            ss << "+--------------------------------------------------+\n";
            ss << "\n+--------------------------------------------------+\n";
            ss << "| [OK] FILE IS VALID                                |\n";
        } else {
            ss << "| [ERROR] " << result.errorMessage << "\n";
            ss << "+--------------------------------------------------+\n";
            ss << "\n+--------------------------------------------------+\n";
            ss << "| [ERROR] FILE IS INVALID                            |\n";
        }
        ss << "+--------------------------------------------------+\n";
        
        return ss.str();
    }
    
private:
    std::string truncateFilename(const std::string& filename, size_t maxLen) {
        if (filename.length() <= maxLen) return filename;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string name = filename.substr(lastSlash + 1);
            if (name.length() <= maxLen) return "..." + name;
            return "..." + name.substr(name.length() - maxLen + 3);
        }
        return "..." + filename.substr(filename.length() - maxLen + 3);
    }
};

// ============================================================================
// JSON Formatter Implementation
// ============================================================================

class JsonFormatter : public IReportFormatter {
public:
    std::string format(const AnalysisResult& result, const std::string& filename) override {
        std::stringstream ss;
        
        ss << "{\n";
        ss << "  \"version\": \"7.0.0\",\n";
        ss << "  \"filename\": \"" << escapeJson(filename) << "\",\n";
        ss << "  \"format\": " << result.rawData.format << ",\n";
        ss << "  \"tracks_declared\": " << result.rawData.numTracks << ",\n";
        ss << "  \"tracks_found\": " << result.tracksFound << ",\n";
        ss << "  \"division\": " << result.rawData.division << ",\n";
        ss << "  \"file_size_bytes\": " << result.rawData.fileSize << ",\n";
        ss << "  \"total_events\": " << result.totalEvents << ",\n";
        ss << "  \"note_on\": " << result.noteOnCount << ",\n";
        ss << "  \"note_off\": " << result.noteOffCount << ",\n";
        ss << "  \"control_change\": " << result.controlChangeCount << ",\n";
        ss << "  \"program_change\": " << result.programChangeCount << ",\n";
        ss << "  \"pitch_bend\": " << result.pitchBendCount << ",\n";
        
        if (result.hasNotes) {
            ss << "  \"note_min\": " << (int)result.minNote << ",\n";
            ss << "  \"note_max\": " << (int)result.maxNote << ",\n";
        }
        
        ss << "  \"duration_seconds\": " << std::fixed << std::setprecision(3) << result.durationSeconds << ",\n";
        ss << "  \"max_polyphony\": " << result.maxPolyphony << ",\n";
        ss << "  \"tracks_with_eot\": " << result.tracksWithEOT << ",\n";
        ss << "  \"is_valid\": " << (result.isValid ? "true" : "false");
        
        if (!result.isValid && !result.errorMessage.empty()) {
            ss << ",\n  \"error\": \"" << escapeJson(result.errorMessage) << "\"";
        }
        
        if (!result.warnings.empty()) {
            ss << ",\n  \"warnings\": [\n";
            for (size_t i = 0; i < result.warnings.size(); ++i) {
                ss << "    \"" << escapeJson(result.warnings[i]) << "\"";
                if (i < result.warnings.size() - 1) ss << ",";
                ss << "\n";
            }
            ss << "  ]";
        }
        
        ss << "\n}\n";
        return ss.str();
    }
    
    std::string getFileExtension() const override { return ".json"; }
    std::string getMimeType() const override { return "application/json"; }
    
private:
    std::string escapeJson(const std::string& str) const {
        std::stringstream ss;
        for (char c : str) {
            switch (c) {
                case '"': ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default: ss << c; break;
            }
        }
        return ss.str();
    }
};

// ============================================================================
// Silent Formatter Implementation
// ============================================================================

class SilentFormatter : public IReportFormatter {
public:
    std::string format(const AnalysisResult& /*result*/, const std::string& /*filename*/) override {
        return "";
    }
};

// ============================================================================
// Formatter Factory Implementation
// ============================================================================

std::unique_ptr<IReportFormatter> FormatterFactory::create(ReportFormat format) {
    switch (format) {
        case ReportFormat::Console:
            return std::make_unique<ConsoleFormatter>();
        case ReportFormat::JSON:
            return std::make_unique<JsonFormatter>();
        case ReportFormat::Silent:
            return std::make_unique<SilentFormatter>();
        default:
            return std::make_unique<ConsoleFormatter>();
    }
}

std::unique_ptr<IReportFormatter> FormatterFactory::create(const std::string& formatName) {
    std::string lower = formatName;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "console" || lower == "text") {
        return create(ReportFormat::Console);
    } else if (lower == "json") {
        return create(ReportFormat::JSON);
    } else if (lower == "silent" || lower == "quiet") {
        return create(ReportFormat::Silent);
    }
    
    return create(ReportFormat::Console);
}

std::vector<std::string> FormatterFactory::getAvailableFormats() {
    return {"console", "json", "silent"};
}

} // namespace midi