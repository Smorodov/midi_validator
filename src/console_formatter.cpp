#include "report_formatter.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace midi {

class ConsoleFormatter : public IReportFormatter {
public:
    std::string format(const AnalysisResult& result, 
                       const std::string& filename) override {
        std::stringstream ss;
        
        // Заголовок
        ss << "\n+--------------------------------------------------+\n";
        ss << "|     MIDI VALIDATOR v7.0.0                        |\n";
        ss << "+--------------------------------------------------+\n";
        
        // Информация о файле
        printFileHeader(ss, result, filename);
        
        // Информация о треках
        printTrackInfo(ss, result);
        
        // Статистика событий
        printEventStatistics(ss, result);
        
        // Информация о темпе и длительности
        printTempoInfo(ss, result);
        
        // Результаты валидации
        printValidationResults(ss, result);
        
        // Предупреждения (если есть)
        if (!result.warnings.empty()) {
            printWarnings(ss, result);
        }
        
        // Итоговый вердикт
        printVerdict(ss, result);
        
        return ss.str();
    }
    
private:
    void printFileHeader(std::stringstream& ss, const AnalysisResult& result, 
                         const std::string& filename) {
        ss << "\n+--------------------------------------------------+\n";
        ss << "| FILE HEADER                                      |\n";
        ss << "+--------------------------------------------------+\n";
        ss << "| Filename:        " << truncateFilename(filename, 34) << "\n";
        ss << "| Format:          " << result.rawData.format;
        
        if (result.rawData.format == 0) {
            ss << " (single track)\n";
        } else if (result.rawData.format == 1) {
            ss << " (multi-track, synchronous)\n";
        } else if (result.rawData.format == 2) {
            ss << " (multi-track, asynchronous)\n";
        } else {
            ss << " (unknown)\n";
        }
        
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
    }
    
    void printTrackInfo(std::stringstream& ss, const AnalysisResult& result) {
        ss << "\n+--------------------------------------------------+\n";
        ss << "| TRACK INFORMATION                                |\n";
        ss << "+--------------------------------------------------+\n";
        
        if (result.rawData.tracks.empty()) {
            ss << "| No tracks found!                                 |\n";
        }
        
        for (size_t i = 0; i < result.rawData.tracks.size() && i < 20; ++i) {
            const auto& track = result.rawData.tracks[i];
            ss << "| Track " << std::setw(2) << (i + 1) << ": ";
            ss << std::setw(5) << track.events.size() << " events, ";
            ss << std::setw(6) << track.length << " bytes";
            
            if (!track.hasEndOfTrack) {
                ss << " [NO EOT!]";
            }
            if (!track.trackName.empty()) {
                std::string name = track.trackName;
                if (name.length() > 20) name = name.substr(0, 17) + "...";
                ss << " \"" << name << "\"";
            }
            ss << "\n";
        }
        
        if (result.rawData.tracks.size() > 20) {
            ss << "| ... and " << (result.rawData.tracks.size() - 20) 
               << " more tracks\n";
        }
        
        ss << "| Tracks with EOT: " << result.tracksWithEOT;
        if (result.tracksWithEOT != result.tracksFound && result.tracksFound > 0) {
            ss << " [WARNING: " << (result.tracksFound - result.tracksWithEOT) << " missing]";
        }
        ss << "\n+--------------------------------------------------+\n";
    }
    
    void printEventStatistics(std::stringstream& ss, const AnalysisResult& result) {
        ss << "\n+--------------------------------------------------+\n";
        ss << "| EVENT STATISTICS                                 |\n";
        ss << "+--------------------------------------------------+\n";
        ss << "| Total events:    " << std::setw(34) << result.totalEvents << "\n";
        ss << "| Note On events:  " << std::setw(34) << result.noteOnCount << "\n";
        ss << "| Note Off events: " << std::setw(34) << result.noteOffCount << "\n";
        ss << "| Control Change:  " << std::setw(34) << result.controlChangeCount << "\n";
        ss << "| Program Change:  " << std::setw(34) << result.programChangeCount << "\n";
        ss << "| Pitch Bend:      " << std::setw(34) << result.pitchBendCount << "\n";
        ss << "| Aftertouch:      " << std::setw(34) << result.aftertouchCount << "\n";
        ss << "| SysEx events:    " << std::setw(34) << result.sysexCount << "\n";
        ss << "| Meta events:     " << std::setw(34) << result.metaEventCount << "\n";
        
        if (result.hasNotes) {
            ss << "| Note range:      " << std::setw(34);
            ss << (int)result.minNote << " - " << (int)result.maxNote << "\n";
        }
        ss << "+--------------------------------------------------+\n";
    }
    
    void printTempoInfo(std::stringstream& ss, const AnalysisResult& result) {
        ss << "\n+--------------------------------------------------+\n";
        ss << "| TEMPO & DURATION                                 |\n";
        ss << "+--------------------------------------------------+\n";
        
        if (result.maxBPM > 0) {
            ss << "| Tempo range:     " << std::setw(34);
            ss << std::fixed << std::setprecision(1) << result.minBPM 
               << " - " << result.maxBPM << " BPM\n";
        } else {
            ss << "| Tempo:           " << std::setw(34) << "120.0 BPM (default)\n";
        }
        
        ss << "| Duration:        " << std::setw(34);
        int minutes = static_cast<int>(result.durationSeconds) / 60;
        int seconds = static_cast<int>(result.durationSeconds) % 60;
        int ms = static_cast<int>((result.durationSeconds - static_cast<int>(result.durationSeconds)) * 1000);
        
        ss << minutes << "m " << seconds << "s";
        if (ms > 0) ss << "." << std::setw(3) << std::setfill('0') << ms << "s";
        ss << "\n";
        
        ss << "| Max polyphony:   " << std::setw(34) 
           << result.maxPolyphony << " simultaneous notes\n";
        ss << "+--------------------------------------------------+\n";
    }
    
    void printValidationResults(std::stringstream& ss, const AnalysisResult& result) {
        ss << "\n+--------------------------------------------------+\n";
        ss << "| INTEGRITY CHECKS                                 |\n";
        ss << "+--------------------------------------------------+\n";
        
        // Проверка EOT
        if (result.tracksWithEOT == result.tracksFound && result.tracksFound > 0) {
            ss << "| [OK] All tracks have End of Track markers\n";
        } else if (result.tracksFound > 0) {
            ss << "| [WARNING] " << (result.tracksFound - result.tracksWithEOT) 
               << " track(s) missing End of Track marker\n";
        }
        
        // Проверка формата
        if (result.rawData.format <= 2) {
            ss << "| [OK] Valid format (" << result.rawData.format << ")\n";
        }
        
        // Проверка соответствия треков
        if (result.tracksFound == result.rawData.numTracks) {
            ss << "| [OK] Track count matches header\n";
        } else if (result.rawData.numTracks > 0) {
            ss << "| [ERROR] Track count mismatch\n";
        }
        
        // Проверка нот
        if (result.hasNotes) {
            if (result.minNote >= 0 && result.maxNote <= 127) {
                ss << "| [OK] All notes within valid range (0-127)\n";
            } else {
                ss << "| [ERROR] Notes out of valid range (0-127)\n";
            }
        }
        
        ss << "+--------------------------------------------------+\n";
    }
    
    void printWarnings(std::stringstream& ss, const AnalysisResult& result) {
        ss << "\n+--------------------------------------------------+\n";
        ss << "| WARNINGS                                         |\n";
        ss << "+--------------------------------------------------+\n";
        
        for (const auto& warning : result.warnings) {
            std::string w = warning;
            if (w.length() > 46) w = w.substr(0, 43) + "...";
            ss << "| [WARN] " << std::left << std::setw(42) << w << "|\n";
        }
        ss << "+--------------------------------------------------+\n";
    }
    
    void printVerdict(std::stringstream& ss, const AnalysisResult& result) {
        ss << "\n+--------------------------------------------------+\n";
        if (result.isValid) {
            ss << "| [OK] FILE IS VALID                                |\n";
        } else {
            std::string error = result.errorMessage;
            if (error.length() > 46) error = error.substr(0, 43) + "...";
            ss << "| [ERROR] " << std::left << std::setw(46) << error << "|\n";
        }
        ss << "+--------------------------------------------------+\n";
        ss << "\nExit code: " << (result.isValid ? 0 : 1) << "\n";
    }
    
    std::string truncateFilename(const std::string& filename, size_t maxLen) {
        if (filename.length() <= maxLen) {
            return filename;
        }
        // Ищем последний разделитель пути
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string name = filename.substr(lastSlash + 1);
            if (name.length() <= maxLen) {
                return "..." + name;
            }
            return "..." + name.substr(name.length() - maxLen + 3);
        }
        return "..." + filename.substr(filename.length() - maxLen + 3);
    }
};

// ============================================================================
// Реализация фабрики
// ============================================================================

std::unique_ptr<IReportFormatter> FormatterFactory::create(ReportFormat format) {
    switch (format) {
        case ReportFormat::Console:
            return std::make_unique<ConsoleFormatter>();
        case ReportFormat::JSON:
            // TODO: реализовать в патче 5
            return std::make_unique<ConsoleFormatter>();
        case ReportFormat::Silent:
            // TODO: реализовать в патче 6
            return std::make_unique<ConsoleFormatter>();
        case ReportFormat::CSV:
        case ReportFormat::HTML:
        case ReportFormat::XML:
            // TODO: реализовать позже
            return std::make_unique<ConsoleFormatter>();
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
    } else if (lower == "csv") {
        return create(ReportFormat::CSV);
    } else if (lower == "html") {
        return create(ReportFormat::HTML);
    } else if (lower == "xml") {
        return create(ReportFormat::XML);
    }
    
    // По умолчанию консольный формат
    return create(ReportFormat::Console);
}

std::vector<std::string> FormatterFactory::getAvailableFormats() {
    return {
        "console",  // ASCII таблицы
        "json",     // JSON формат
        "silent",   // Тихий режим
        "csv",      // CSV статистика
        "html",     // HTML отчет
        "xml"       // XML формат
    };
}

} // namespace midi