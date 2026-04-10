#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace midi_test {

class MidiGenerator {
public:
    MidiGenerator();
    ~MidiGenerator();
    
    // Existing valid generators
    bool generateValidFormat0(const std::string& filename);
    bool generateValidFormat1(const std::string& filename);
    bool generateValidWithTempo(const std::string& filename);
    bool generateValidWithTimeSignature(const std::string& filename);
    bool generateValidWithMultipleTracks(const std::string& filename);
    bool generateValidWithSysEx(const std::string& filename);
    
    // Existing invalid generators
    bool generateInvalidHeader(const std::string& filename);
    bool generateInvalidFormat(const std::string& filename);
    bool generateMissingEndOfTrack(const std::string& filename);
    bool generateInvalidNoteRange(const std::string& filename);
    bool generateInvalidRunningStatus(const std::string& filename);
    bool generateCorruptVLQ(const std::string& filename);
    bool generateEmptyFile(const std::string& filename);
    bool generateTruncatedFile(const std::string& filename);
    bool generateInvalidMetaEvent(const std::string& filename);
    bool generateInvalidTrackChunk(const std::string& filename);
    
    // NEW: Generators for specific test cases
    bool generateNoteOnWithParams(const std::string& filename, uint8_t note, uint8_t velocity);
    bool generateNoteOffWithParams(const std::string& filename, uint8_t note, uint8_t velocity);
    bool generateControlChangeWithParams(const std::string& filename, uint8_t controller, uint8_t value);
    bool generateProgramChangeWithParams(const std::string& filename, uint8_t program);
    bool generatePitchBendWithParams(const std::string& filename, uint16_t pitchBend);
    bool generatePolyAftertouchWithParams(const std::string& filename, uint8_t note, uint8_t pressure);
    bool generateChannelAftertouchWithParams(const std::string& filename, uint8_t pressure);
    
    // NEW: Out-of-range test generators
    bool generateNoteOutOfRange(const std::string& filename, uint8_t note);
    bool generateVelocityOutOfRange(const std::string& filename, uint8_t velocity);
    bool generatePressureOutOfRange(const std::string& filename, uint8_t pressure);
    bool generateControllerOutOfRange(const std::string& filename, uint8_t controller);
    bool generateControlValueOutOfRange(const std::string& filename, uint8_t value);
    bool generateProgramOutOfRange(const std::string& filename, uint8_t program);
    bool generatePitchBendOutOfRange(const std::string& filename, uint16_t pitchBend);
    
    // NEW: Generate all test files (including all boundary tests)
    bool generateAllTestFiles(const std::string& outputDir);
    
    // NEW: Generate only boundary test files (valid and invalid)
    bool generateBoundaryTestFiles(const std::string& outputDir);
    
private:
    std::vector<uint8_t> writeVLQ(uint32_t value);
    std::vector<uint8_t> writeUint16(uint16_t value);
    std::vector<uint8_t> writeUint32(uint32_t value);
    
    std::vector<uint8_t> makeNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    std::vector<uint8_t> makeNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    std::vector<uint8_t> makeMetaEvent(uint8_t metaType, const std::vector<uint8_t>& data);
    std::vector<uint8_t> makeEndOfTrack();
    std::vector<uint8_t> makeTempoEvent(uint32_t microsecondsPerQuarter);
    std::vector<uint8_t> makeTimeSignature(uint8_t numerator, uint8_t denominatorPower, 
                                            uint8_t clocksPerTick, uint8_t thirtySecondNotes);
};

}