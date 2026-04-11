#ifndef MIDI_TYPES_H
#define MIDI_TYPES_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>

namespace midi {

enum class EventType : uint8_t {
    NoteOff             = 0x80,
    NoteOn              = 0x90,
    PolyAftertouch      = 0xA0,
    ControlChange       = 0xB0,
    ProgramChange       = 0xC0,
    ChannelAftertouch   = 0xD0,
    PitchBend           = 0xE0,
    SysEx               = 0xF0,
    SongPositionPointer = 0xF2,
    SongSelect          = 0xF3,
    TuneRequest         = 0xF6,
    SysExEnd            = 0xF7,
    TimingClock         = 0xF8,
    Start               = 0xFA,
    Continue            = 0xFB,
    Stop                = 0xFC,
    ActiveSensing       = 0xFE,
    Reset               = 0xFF,
    MetaEvent           = 0xFF
};

struct MidiEvent {
    uint32_t deltaTime = 0;
    EventType type = EventType::NoteOff;
    uint8_t channel = 0;
    std::vector<uint8_t> data;
    uint32_t fileOffset = 0;
    bool isValid = true;
    bool usesRunningStatus = false;
    std::string errorMessage;
    
    bool isChannelVoice() const {
        uint8_t cmd = static_cast<uint8_t>(type);
        return (cmd >= 0x80 && cmd <= 0xEF);
    }
    
    bool isSystemCommon() const {
        uint8_t cmd = static_cast<uint8_t>(type);
        return (cmd >= 0xF0 && cmd <= 0xF7);
    }
    
    bool isSystemRealTime() const {
        uint8_t cmd = static_cast<uint8_t>(type);
        return (cmd >= 0xF8);
    }
};

struct NoteEvent {
    uint8_t note;
    uint8_t velocity;
    bool isNoteOn() const { return velocity > 0; }
    bool isNoteOff() const { return velocity == 0; }
};

struct ControlChangeEvent {
    uint8_t controller;
    uint8_t value;
    static constexpr uint8_t ALL_NOTES_OFF = 123;
    static constexpr uint8_t OMNI_MODE_OFF = 124;
    static constexpr uint8_t OMNI_MODE_ON = 125;
    static constexpr uint8_t MONO_MODE_ON = 126;
    static constexpr uint8_t POLY_MODE_ON = 127;
    bool isChannelMode() const {
        return controller >= 120 && controller <= 127;
    }
};

struct PitchBendEvent {
    uint16_t value;
    int16_t signedValue() const {
        return static_cast<int16_t>(value) - 8192;
    }
};

struct ProgramChangeEvent {
    uint8_t program;
};

struct MetaEvent {
    uint8_t metaType;
    std::vector<uint8_t> data;
    
    static constexpr uint8_t SEQUENCE_NUMBER = 0x00;
    static constexpr uint8_t TEXT_EVENT = 0x01;
    static constexpr uint8_t COPYRIGHT = 0x02;
    static constexpr uint8_t TRACK_NAME = 0x03;
    static constexpr uint8_t INSTRUMENT_NAME = 0x04;
    static constexpr uint8_t LYRIC = 0x05;
    static constexpr uint8_t MARKER = 0x06;
    static constexpr uint8_t CUE_POINT = 0x07;
    static constexpr uint8_t PROGRAM_NAME = 0x08;
    static constexpr uint8_t DEVICE_NAME = 0x09;
    static constexpr uint8_t MIDI_CHANNEL_PREFIX = 0x20;
    static constexpr uint8_t END_OF_TRACK = 0x2F;
    static constexpr uint8_t SET_TEMPO = 0x51;
    static constexpr uint8_t SMPTE_OFFSET = 0x54;
    static constexpr uint8_t TIME_SIGNATURE = 0x58;
    static constexpr uint8_t KEY_SIGNATURE = 0x59;
    static constexpr uint8_t SEQUENCER_SPECIFIC = 0x7F;
    
    bool isEndOfTrack() const { return metaType == END_OF_TRACK; }
    bool isTempo() const { return metaType == SET_TEMPO; }
    bool isTimeSignature() const { return metaType == TIME_SIGNATURE; }
    bool isKeySignature() const { return metaType == KEY_SIGNATURE; }
    bool isSequenceNumber() const { return metaType == SEQUENCE_NUMBER; }
    
    std::string getText() const {
        return std::string(data.begin(), data.end());
    }
    
    uint32_t getTempo() const {
        if (metaType != SET_TEMPO || data.size() < 3) return 500000;
        return (data[0] << 16) | (data[1] << 8) | data[2];
    }
    
    double getBPM() const {
        uint32_t uspq = getTempo();
        return (uspq > 0) ? 60000000.0 / uspq : 120.0;
    }
    
    uint16_t getSequenceNumber() const {
        if (metaType != SEQUENCE_NUMBER || data.size() < 2) return 0;
        return (data[0] << 8) | data[1];
    }
};

struct TrackData {
    uint32_t index = 0;
    uint32_t offset = 0;
    uint32_t length = 0;
    std::vector<MidiEvent> events;
    bool hasEndOfTrack = false;
    std::string trackName;
    std::string instrumentName;
    std::string errorMessage;
    
    uint32_t getTotalTicks() const {
        uint32_t ticks = 0;
        for (const auto& event : events) {
            ticks += event.deltaTime;
        }
        return ticks;
    }
};

struct MidiFileData {
    uint16_t format = 0;
    uint16_t numTracks = 0;
    uint16_t division = 480;
    bool isSMPTE = false;
    int8_t smpteFrames = 0;
    uint8_t smpteResolution = 0;
    uint32_t fileSize = 0;
    std::vector<TrackData> tracks;
    bool isValid = true;
    std::string errorMessage;
    uint16_t songNumber = 0;
    
    uint16_t getTicksPerQuarterNote() const {
        return isSMPTE ? 480 : division;
    }
    
    bool isFormat0() const { return format == 0; }
    bool isFormat1() const { return format == 1; }
    bool isFormat2() const { return format == 2; }
};

struct AnalysisResult {
    bool isValid = true;
    std::string errorMessage;
    std::vector<std::string> warnings;
    uint32_t totalEvents = 0;
    uint32_t noteOnCount = 0;
    uint32_t noteOffCount = 0;
    uint32_t controlChangeCount = 0;
    uint32_t programChangeCount = 0;
    uint32_t pitchBendCount = 0;
    uint32_t aftertouchCount = 0;
    uint32_t sysexCount = 0;
    uint32_t metaEventCount = 0;
    uint8_t minNote = 127;
    uint8_t maxNote = 0;
    bool hasNotes = false;
    double durationSeconds = 0.0;
    double minBPM = 1000.0;
    double maxBPM = 0.0;
    int maxPolyphony = 0;
    int tracksFound = 0;
    int tracksWithEOT = 0;
    MidiFileData rawData;
    
    void addWarning(const std::string& warning) {
        warnings.push_back(warning);
    }
};

} // namespace midi

#endif