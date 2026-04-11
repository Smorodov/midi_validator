#include "midi_analyzer.h"
#include "midi_spec.h"
#include <cmath>
#include <algorithm>

namespace midi {

MidiAnalyzer::MidiAnalyzer() {}
MidiAnalyzer::~MidiAnalyzer() {}

AnalysisResult MidiAnalyzer::analyze(const MidiFileData& fileData) {
    AnalysisResult result;
    result.rawData = fileData;
    result.tracksFound = static_cast<int>(fileData.tracks.size());
    std::map<int, int> activeNotes;
    
    // Проверка формата
    if (!MidiSpec::isValidFormat(fileData.format)) {
        result.isValid = false;
        result.errorMessage = "Invalid format: " + std::to_string(fileData.format);
        return result;
    }
    
    // Проверка SMPTE
    if (fileData.isSMPTE && !MidiSpec::isValidSMPTE(fileData.smpteFrames)) {
        result.isValid = false;
        result.errorMessage = "Invalid SMPTE value: " + std::to_string(fileData.smpteFrames);
        return result;
    }
    
    for (const auto& track : fileData.tracks) {
        if (track.hasEndOfTrack) result.tracksWithEOT++;
        for (const auto& event : track.events) {
            result.totalEvents++;
            processEvent(event, result, activeNotes);
            if (!result.isValid) return result;
        }
    }
    
    calculatePolyphony(result, fileData);
    calculateDuration(result, fileData);
    validateRules(fileData, result);
    
    if (result.noteOnCount > 0) result.hasNotes = true;
    return result;
}

AnalysisResult MidiAnalyzer::analyzeFile(const std::string& filename) {
    MidiFileReader reader;
    MidiFileData fileData = reader.readFile(filename);
    
    if (!fileData.isValid) {
        AnalysisResult result;
        result.isValid = false;
        result.errorMessage = fileData.errorMessage;
        m_lastError = fileData.errorMessage;
        return result;
    }
    return analyze(fileData);
}

void MidiAnalyzer::processEvent(const MidiEvent& event, AnalysisResult& result, std::map<int, int>& activeNotes) {
    // Мета-события
    if (event.type == EventType::MetaEvent) {
        result.metaEventCount++;
        if (event.data.empty()) return;
        
        uint8_t metaType = event.data[0];
        
        // Проверка типа мета-события
        if (VALID_META_TYPES.find(metaType) == VALID_META_TYPES.end()) {
            result.isValid = false;
            result.errorMessage = "Invalid meta event type: 0x" + std::to_string(metaType);
            return;
        }
        
        // Обработка Set Tempo
        if (metaType == MetaEvent::SET_TEMPO && event.data.size() >= 4) {
            uint32_t uspq = (event.data[1] << 16) | (event.data[2] << 8) | event.data[3];
            if (uspq > 0) {
                double bpm = 60000000.0 / uspq;
                if (bpm < result.minBPM) result.minBPM = bpm;
                if (bpm > result.maxBPM) result.maxBPM = bpm;
            }
        }
        return;
    }
    
    // System Real-Time - пропускаем
    if (static_cast<uint8_t>(event.type) >= 0xF8 && static_cast<uint8_t>(event.type) < 0xFF) return;
    
    switch (event.type) {
        case EventType::NoteOn: {
            auto ne = extractNoteEvent(event);
            result.noteOnCount++;
            
            if (!MidiSpec::isValidNote(ne.note)) {
                result.isValid = false;
                result.errorMessage = "Note out of range: " + std::to_string(ne.note);
                return;
            }
            if (!MidiSpec::isValidVelocity(ne.velocity)) {
                result.addWarning("Velocity out of range: " + std::to_string(ne.velocity));
            }
            
            if (ne.note < result.minNote) result.minNote = ne.note;
            if (ne.note > result.maxNote) result.maxNote = ne.note;
            
            if (ne.velocity > 0) activeNotes[event.channel * 128 + ne.note]++;
            break;
        }
        case EventType::NoteOff: {
            auto ne = extractNoteEvent(event);
            result.noteOffCount++;
            
            if (!MidiSpec::isValidNote(ne.note)) {
                result.isValid = false;
                result.errorMessage = "Note out of range: " + std::to_string(ne.note);
                return;
            }
            
            int key = event.channel * 128 + ne.note;
            auto it = activeNotes.find(key);
            if (it != activeNotes.end()) {
                if (it->second > 1) it->second--;
                else activeNotes.erase(it);
            }
            break;
        }
        case EventType::ControlChange: {
            result.controlChangeCount++;
            if (event.data.size() >= 2) {
                uint8_t controller = event.data[0];
                uint8_t value = event.data[1];
                
                if (!MidiSpec::isValidController(controller)) {
                    result.addWarning("Controller out of range: " + std::to_string(controller));
                }
                
                // Проверка Channel Mode сообщений
                if (controller >= 120 && controller <= 127) {
                    if (!MidiSpec::isValidChannelMode(controller, value)) {
                        result.addWarning("Invalid channel mode value for controller " + 
                                         std::to_string(controller) + ": " + std::to_string(value));
                    }
                }
            }
            break;
        }
        case EventType::ProgramChange: {
            result.programChangeCount++;
            if (event.data.size() >= 1 && !MidiSpec::isValidProgram(event.data[0])) {
                result.addWarning("Program out of range: " + std::to_string(event.data[0]));
            }
            break;
        }
        case EventType::PitchBend: {
            result.pitchBendCount++;
            if (event.data.size() >= 2) {
                uint16_t value = (event.data[1] << 7) | event.data[0];
                if (!MidiSpec::isValidPitchBend(value)) {
                    result.addWarning("Pitch bend out of range: " + std::to_string(value));
                }
            }
            break;
        }
        case EventType::PolyAftertouch:
        case EventType::ChannelAftertouch: {
            result.aftertouchCount++;
            if (event.data.size() >= 1 && !MidiSpec::isValidPressure(event.data[0])) {
                result.addWarning("Pressure out of range: " + std::to_string(event.data[0]));
            }
            break;
        }
        case EventType::SysEx:
        case EventType::SysExEnd: {
            result.sysexCount++;
            break;
        }
        default:
            break;
    }
}

void MidiAnalyzer::calculatePolyphony(AnalysisResult& result, const MidiFileData& data) {
    std::map<int, int> active;
    int maxActive = 0;
    for (const auto& track : data.tracks) {
        for (const auto& event : track.events) {
            if (event.type == EventType::NoteOn && event.data.size() >= 2 && event.data[1] > 0) {
                int key = event.channel * 128 + event.data[0];
                active[key]++;
                if (static_cast<int>(active.size()) > maxActive) {
                    maxActive = static_cast<int>(active.size());
                }
            } else if ((event.type == EventType::NoteOff) ||
                       (event.type == EventType::NoteOn && event.data.size() >= 2 && event.data[1] == 0)) {
                int key = event.channel * 128 + event.data[0];
                auto it = active.find(key);
                if (it != active.end()) {
                    if (it->second > 1) it->second--;
                    else active.erase(it);
                }
            }
        }
    }
    result.maxPolyphony = maxActive;
}

void MidiAnalyzer::calculateDuration(AnalysisResult& result, const MidiFileData& data) {
    if (data.isSMPTE || data.getTicksPerQuarterNote() == 0) {
        result.durationSeconds = 0.0;
        return;
    }
    
    uint32_t maxTicks = 0;
    for (const auto& track : data.tracks) {
        uint32_t ticks = 0;
        for (const auto& event : track.events) ticks += event.deltaTime;
        if (ticks > maxTicks) maxTicks = ticks;
    }
    
    auto tempoMap = buildTempoMap(data);
    double totalSec = 0.0;
    uint32_t lastTick = 0;
    double currentBPM = 120.0;
    
    for (const auto& pair : tempoMap) {
        if (pair.first > lastTick) {
            double secPerTick = 60.0 / currentBPM / data.getTicksPerQuarterNote();
            totalSec += (pair.first - lastTick) * secPerTick;
        }
        lastTick = pair.first;
        currentBPM = pair.second;
    }
    
    if (maxTicks > lastTick) {
        double secPerTick = 60.0 / currentBPM / data.getTicksPerQuarterNote();
        totalSec += (maxTicks - lastTick) * secPerTick;
    }
    
    result.durationSeconds = totalSec;
}

std::map<uint32_t, double> MidiAnalyzer::buildTempoMap(const MidiFileData& data) {
    std::map<uint32_t, double> tempoMap;
    tempoMap[0] = 120.0;
    uint32_t tick = 0;
    
    for (const auto& track : data.tracks) {
        tick = 0;
        for (const auto& event : track.events) {
            tick += event.deltaTime;
            if (event.type == EventType::MetaEvent && !event.data.empty()) {
                if (event.data[0] == MetaEvent::SET_TEMPO && event.data.size() >= 4) {
                    uint32_t uspq = (event.data[1] << 16) | (event.data[2] << 8) | event.data[3];
                    if (uspq > 0) tempoMap[tick] = 60000000.0 / uspq;
                }
            }
        }
    }
    return tempoMap;
}

void MidiAnalyzer::validateRules(const MidiFileData& data, AnalysisResult& result) {
    if (result.tracksWithEOT != static_cast<int>(data.tracks.size()) && data.tracks.size() > 0) {
        result.isValid = false;
        result.errorMessage = "Missing End of Track markers";
        return;
    }
    
    if (data.format == 0 && data.tracks.size() != 1 && data.tracks.size() > 0) {
        result.isValid = false;
        result.errorMessage = "Format 0 must have exactly 1 track";
        return;
    }
    
    if (data.format == 1 && data.tracks.size() < 1) {
        result.isValid = false;
        result.errorMessage = "Format 1 must have at least 1 track";
        return;
    }
}

NoteEvent MidiAnalyzer::extractNoteEvent(const MidiEvent& event) {
    NoteEvent ne = {0, 0};
    if (event.data.size() >= 2) { ne.note = event.data[0]; ne.velocity = event.data[1]; }
    return ne;
}

ControlChangeEvent MidiAnalyzer::extractControlChange(const MidiEvent& event) {
    ControlChangeEvent cc = {0, 0};
    if (event.data.size() >= 2) { cc.controller = event.data[0]; cc.value = event.data[1]; }
    return cc;
}

PitchBendEvent MidiAnalyzer::extractPitchBend(const MidiEvent& event) {
    PitchBendEvent pb = {8192};
    if (event.data.size() >= 2) pb.value = (event.data[1] << 7) | event.data[0];
    return pb;
}

ProgramChangeEvent MidiAnalyzer::extractProgramChange(const MidiEvent& event) {
    ProgramChangeEvent pc = {0};
    if (event.data.size() >= 1) pc.program = event.data[0];
    return pc;
}

MetaEvent MidiAnalyzer::extractMetaEvent(const MidiEvent& event) {
    MetaEvent meta = {0, {}};
    if (!event.data.empty()) {
        meta.metaType = event.data[0];
        if (event.data.size() > 1) meta.data.assign(event.data.begin() + 1, event.data.end());
    }
    return meta;
}

} // namespace midi