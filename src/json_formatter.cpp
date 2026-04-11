#include "report_formatter.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>

namespace midi {

class JsonFormatter : public IReportFormatter {
public:
    std::string format(const AnalysisResult& result, 
                       const std::string& filename) override {
        std::stringstream ss;
        
        ss << "{\n";
        ss << "  \"version\": \"7.0.0\",\n";
        ss << "  \"filename\": \"" << escapeJson(filename) << "\",\n";
        
        // Информация о файле
        printFileInfo(ss, result);
        ss << ",\n";
        
        // Статистика
        printStatistics(ss, result);
        ss << ",\n";
        
        // Информация о треках
        printTracks(ss, result);
        ss << ",\n";
        
        // Темп и длительность
        printTempoInfo(ss, result);
        ss << ",\n";
        
        // Валидация
        printValidation(ss, result);
        ss << ",\n";
        
        // Предупреждения
        printWarnings(ss, result);
        
        ss << "\n}\n";
        
        return ss.str();
    }
    
    std::string getFileExtension() const override {
        return ".json";
    }
    
    std::string getMimeType() const override {
        return "application/json";
    }
    
private:
    void printFileInfo(std::stringstream& ss, const AnalysisResult& result) {
        ss << "  \"file_info\": {\n";
        ss << "    \"format\": " << result.rawData.format << ",\n";
        ss << "    \"format_description\": \"" << getFormatDescription(result.rawData.format) << "\",\n";
        ss << "    \"tracks_declared\": " << result.rawData.numTracks << ",\n";
        ss << "    \"tracks_found\": " << result.tracksFound << ",\n";
        ss << "    \"division\": " << result.rawData.division << ",\n";
        
        if (result.rawData.isSMPTE) {
            ss << "    \"division_type\": \"smpte\",\n";
            ss << "    \"smpte_frames\": " << (int)result.rawData.smpteFrames << ",\n";
            ss << "    \"smpte_resolution\": " << (int)result.rawData.smpteResolution << ",\n";
        } else {
            ss << "    \"division_type\": \"ticks\",\n";
            ss << "    \"ticks_per_quarter_note\": " << result.rawData.getTicksPerQuarterNote() << ",\n";
        }
        
        ss << "    \"file_size_bytes\": " << result.rawData.fileSize << "\n";
        ss << "  }";
    }
    
    void printStatistics(std::stringstream& ss, const AnalysisResult& result) {
        ss << "  \"statistics\": {\n";
        ss << "    \"total_events\": " << result.totalEvents << ",\n";
        ss << "    \"note_on\": " << result.noteOnCount << ",\n";
        ss << "    \"note_off\": " << result.noteOffCount << ",\n";
        ss << "    \"control_change\": " << result.controlChangeCount << ",\n";
        ss << "    \"program_change\": " << result.programChangeCount << ",\n";
        ss << "    \"pitch_bend\": " << result.pitchBendCount << ",\n";
        ss << "    \"aftertouch\": " << result.aftertouchCount << ",\n";
        ss << "    \"sysex\": " << result.sysexCount << ",\n";
        ss << "    \"meta_events\": " << result.metaEventCount << ",\n";
        
        if (result.hasNotes) {
            ss << "    \"note_range\": {\n";
            ss << "      \"min\": " << (int)result.minNote << ",\n";
            ss << "      \"max\": " << (int)result.maxNote << "\n";
            ss << "    },\n";
        } else {
            ss << "    \"note_range\": null,\n";
        }
        
        ss << "    \"has_notes\": " << (result.hasNotes ? "true" : "false") << "\n";
        ss << "  }";
    }
    
    void printTracks(std::stringstream& ss, const AnalysisResult& result) {
        ss << "  \"tracks\": [\n";
        
        for (size_t i = 0; i < result.rawData.tracks.size(); ++i) {
            const auto& track = result.rawData.tracks[i];
            
            ss << "    {\n";
            ss << "      \"index\": " << track.index << ",\n";
            ss << "      \"offset\": " << track.offset << ",\n";
            ss << "      \"length_bytes\": " << track.length << ",\n";
            ss << "      \"event_count\": " << track.events.size() << ",\n";
            ss << "      \"has_end_of_track\": " << (track.hasEndOfTrack ? "true" : "false") << ",\n";
            
            if (!track.trackName.empty()) {
                ss << "      \"track_name\": \"" << escapeJson(track.trackName) << "\",\n";
            }
            if (!track.instrumentName.empty()) {
                ss << "      \"instrument_name\": \"" << escapeJson(track.instrumentName) << "\",\n";
            }
            
            // Краткая статистика по событиям в треке
            ss << "      \"track_statistics\": {\n";
            ss << "        \"note_on\": " << countEventsInTrack(track, EventType::NoteOn) << ",\n";
            ss << "        \"note_off\": " << countEventsInTrack(track, EventType::NoteOff) << ",\n";
            ss << "        \"control_change\": " << countEventsInTrack(track, EventType::ControlChange) << ",\n";
            ss << "        \"program_change\": " << countEventsInTrack(track, EventType::ProgramChange) << ",\n";
            ss << "        \"pitch_bend\": " << countEventsInTrack(track, EventType::PitchBend) << "\n";
            ss << "      }\n";
            
            ss << "    }";
            if (i < result.rawData.tracks.size() - 1) ss << ",";
            ss << "\n";
        }
        
        ss << "  ]";
    }
    
    void printTempoInfo(std::stringstream& ss, const AnalysisResult& result) {
        ss << "  \"tempo_and_duration\": {\n";
        
        if (result.maxBPM > 0) {
            ss << "    \"bpm_range\": {\n";
            ss << "      \"min\": " << std::fixed << std::setprecision(2) << result.minBPM << ",\n";
            ss << "      \"max\": " << result.maxBPM << "\n";
            ss << "    },\n";
        } else {
            ss << "    \"bpm_range\": null,\n";
            ss << "    \"default_tempo\": 120.0,\n";
        }
        
        ss << "    \"duration_seconds\": " << std::fixed << std::setprecision(3) << result.durationSeconds << ",\n";
        
        // Длительность в разных форматах
        int minutes = static_cast<int>(result.durationSeconds) / 60;
        int seconds = static_cast<int>(result.durationSeconds) % 60;
        int ms = static_cast<int>((result.durationSeconds - static_cast<int>(result.durationSeconds)) * 1000);
        
        ss << "    \"duration_formatted\": \"" << minutes << "m " << seconds << "s";
        if (ms > 0) ss << "." << std::setw(3) << std::setfill('0') << ms << "s";
        ss << "\",\n";
        
        ss << "    \"max_polyphony\": " << result.maxPolyphony << "\n";
        ss << "  }";
    }
    
    void printValidation(std::stringstream& ss, const AnalysisResult& result) {
        ss << "  \"validation\": {\n";
        ss << "    \"is_valid\": " << (result.isValid ? "true" : "false") << ",\n";
        
        if (!result.isValid && !result.errorMessage.empty()) {
            ss << "    \"error\": \"" << escapeJson(result.errorMessage) << "\",\n";
        }
        
        // Детальные проверки
        ss << "    \"checks\": {\n";
        ss << "      \"valid_format\": " << (result.rawData.format <= 2 ? "true" : "false") << ",\n";
        ss << "      \"tracks_match_header\": " << (result.tracksFound == result.rawData.numTracks ? "true" : "false") << ",\n";
        ss << "      \"all_tracks_have_eot\": " << (result.tracksWithEOT == result.tracksFound ? "true" : "false") << ",\n";
        ss << "      \"notes_in_range\": " << (!result.hasNotes || (result.minNote >= 0 && result.maxNote <= 127) ? "true" : "false") << "\n";
        ss << "    }\n";
        ss << "  }";
    }
    
    void printWarnings(std::stringstream& ss, const AnalysisResult& result) {
        ss << "  \"warnings\": [\n";
        
        for (size_t i = 0; i < result.warnings.size(); ++i) {
            ss << "    \"" << escapeJson(result.warnings[i]) << "\"";
            if (i < result.warnings.size() - 1) ss << ",";
            ss << "\n";
        }
        
        ss << "  ]";
    }
    
    std::string getFormatDescription(uint16_t format) const {
        switch (format) {
            case 0: return "single track";
            case 1: return "multi-track, synchronous";
            case 2: return "multi-track, asynchronous";
            default: return "unknown";
        }
    }
    
    int countEventsInTrack(const TrackData& track, EventType type) const {
        int count = 0;
        for (const auto& event : track.events) {
            if (event.type == type) {
                count++;
            }
        }
        return count;
    }
    
    std::string escapeJson(const std::string& str) const {
        std::stringstream ss;
        for (char c : str) {
            switch (c) {
                case '"':  ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\b': ss << "\\b"; break;
                case '\f': ss << "\\f"; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') 
                           << static_cast<int>(c);
                    } else {
                        ss << c;
                    }
                    break;
            }
        }
        return ss.str();
    }
};

// Регистрируем JSON форматтер в фабрике
// Обновляем функцию create в report_formatter.h
// (это будет сделано в следующем патче, сейчас просто добавляем реализацию)

} // namespace midi