#ifndef MIDI_PARSER_H
#define MIDI_PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

namespace midi {

enum class EventType : uint8_t {
    NoteOff = 0x80,
    NoteOn = 0x90,
    PolyPressure = 0xA0,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
    ChannelPressure = 0xD0,
    PitchBend = 0xE0,
    MetaEvent = 0xFF,
    SysEx = 0xF0,
    SysExEnd = 0xF7
};

struct MidiEvent {
    uint32_t deltaTime = 0;
    uint32_t fileOffset = 0;
    EventType type = EventType::NoteOff;
    uint8_t channel = 0;
    std::vector<uint8_t> data;
    bool isValid = true;
    bool isRunningStatus = false;
    std::string errorMessage;
};

struct TempoEvent {
    uint32_t tick = 0;
    uint32_t microsecondsPerQuarterNote = 500000;
    double bpm = 120.0;
};

struct TimeSignatureEvent {
    uint32_t tick = 0;
    uint8_t numerator = 4;
    uint8_t denominator = 4;
    uint8_t clocksPerMetronomeTick = 24;
    uint8_t thirtySecondNotesPerQuarter = 8;
};

struct MidiTrack {
    uint32_t fileOffset = 0;
    uint32_t length = 0;
    std::vector<MidiEvent> events;
    std::string trackName;
    std::string instrumentName;
    bool hasEndOfTrack = false;
    bool isValid = false;
    std::string errorMessage;
};

struct MidiFileInfo {
    bool isValid = false;
    uint16_t format = 0;
    uint16_t numTracks = 0;
    uint16_t division = 480;
    bool isSMPTE = false;
    int8_t smpteFrames = 0;
    uint8_t smpteResolution = 0;
    uint16_t ticksPerQuarterNote = 480;
    uint32_t fileSize = 0;
    uint32_t totalEvents = 0;
    uint32_t totalNotes = 0;
    uint8_t minNote = 127;
    uint8_t maxNote = 0;
    double minBPM = 1000.0;
    double maxBPM = 0.0;
    double durationSeconds = 0.0;
    int maxPolyphony = 0;
    std::vector<MidiTrack> tracks;
    std::vector<TempoEvent> tempoEvents;
    std::vector<TimeSignatureEvent> timeSignatures;
    std::vector<MidiEvent> controllerEvents;
};

class MidiParser {
public:
    MidiParser();
    ~MidiParser();
    
    bool loadFile(const std::string& filename);
    void resetMidiFile();
    
    const MidiFileInfo& getMidiInfo() const { return midiFile; }
    const std::string& getLastError() const { return lastError; }
    
    std::string toJSON() const;
    void printAnalysis() const;
    void printDetailed(bool showOffsets = false) const;
    
    std::string eventTypeToString(EventType type) const;
    std::string metaEventToString(uint8_t metaType) const;

private:
    bool parseHeader(std::ifstream& file);
    bool parseTrack(std::ifstream& file, MidiTrack& track, uint32_t trackOffset);
    bool parseEvent(std::ifstream& file, MidiEvent& event, uint8_t& runningStatus, uint32_t fileOffset);
    
    uint16_t readUint16(std::ifstream& file);
    uint32_t readUint32(std::ifstream& file);
    uint32_t readVariableLength(std::ifstream& file, size_t& bytesRead);
    
    void calculateDuration();
    void calculatePolyphony();
    
    MidiFileInfo midiFile;
    std::string filename;
    std::string lastError;
};

} // namespace midi

#endif // MIDI_PARSER_H