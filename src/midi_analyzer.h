#ifndef MIDI_ANALYZER_H
#define MIDI_ANALYZER_H

#include "midi_types.h"
#include "midi_reader.h"
#include <string>
#include <map>

namespace midi {

class MidiAnalyzer {
public:
    MidiAnalyzer();
    ~MidiAnalyzer();
    
    AnalysisResult analyze(const MidiFileData& fileData);
    AnalysisResult analyzeFile(const std::string& filename);
    const std::string& getLastError() const { return m_lastError; }
    
private:
    void processEvent(const MidiEvent& event, AnalysisResult& result, std::map<int, int>& activeNotes);
    void calculateDuration(AnalysisResult& result, const MidiFileData& data);
    void calculatePolyphony(AnalysisResult& result, const MidiFileData& data);
    void validateRules(const MidiFileData& data, AnalysisResult& result);
    
    NoteEvent extractNoteEvent(const MidiEvent& event);
    ControlChangeEvent extractControlChange(const MidiEvent& event);
    PitchBendEvent extractPitchBend(const MidiEvent& event);
    ProgramChangeEvent extractProgramChange(const MidiEvent& event);
    MetaEvent extractMetaEvent(const MidiEvent& event);
    
    bool validateNoteRange(uint8_t note, AnalysisResult& result);
    bool validateVelocityRange(uint8_t velocity, AnalysisResult& result);
    bool validateControllerRange(uint8_t controller, AnalysisResult& result);
    bool validateProgramRange(uint8_t program, AnalysisResult& result);
    bool validatePitchBendRange(uint16_t value, AnalysisResult& result);
    bool validatePressureRange(uint8_t pressure, AnalysisResult& result);
    
    std::map<uint32_t, double> buildTempoMap(const MidiFileData& data);
    
    std::string m_lastError;
};

} // namespace midi

#endif