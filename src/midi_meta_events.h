#ifndef MIDI_META_EVENTS_H
#define MIDI_META_EVENTS_H

#include <string>
#include <vector>
#include <cstdint>

namespace midi {

// Все типы мета-событий согласно MIDI 1.0 Specification
enum class MetaEventType : uint8_t {
    SequenceNumber       = 0x00,
    TextEvent           = 0x01,
    CopyrightNotice     = 0x02,
    TrackName           = 0x03,
    InstrumentName      = 0x04,
    Lyric               = 0x05,
    Marker              = 0x06,
    CuePoint            = 0x07,
    ProgramName         = 0x08,
    DeviceName          = 0x09,
    MIDIChannelPrefix   = 0x20,
    EndOfTrack          = 0x2F,
    SetTempo            = 0x51,
    SMPTEOffset         = 0x54,
    TimeSignature       = 0x58,
    KeySignature        = 0x59,
    SequencerSpecific   = 0x7F
};

struct MetaEventData {
    MetaEventType type;
    uint32_t deltaTime;
    uint32_t tickPosition;
    std::vector<uint8_t> data;
    
    // Удобные методы для разных типов
    std::string getText() const {
        return std::string(data.begin(), data.end());
    }
    
    uint16_t getSequenceNumber() const {
        if (data.size() >= 2) {
            return (data[0] << 8) | data[1];
        }
        return 0;
    }
    
    uint32_t getTempo() const {
        if (type == MetaEventType::SetTempo && data.size() >= 3) {
            return (data[0] << 16) | (data[1] << 8) | data[2];
        }
        return 500000;
    }
    
    double getBPM() const {
        uint32_t uspq = getTempo();
        return (uspq > 0) ? 60000000.0 / uspq : 120.0;
    }
    
    struct TimeSignatureInfo {
        uint8_t numerator;
        uint8_t denominator;  // 2 = quarter, 4 = eighth, etc.
        uint8_t clocksPerMetronomeTick;
        uint8_t thirtySecondNotesPerQuarter;
    };
    
    TimeSignatureInfo getTimeSignature() const {
        TimeSignatureInfo ts = {4, 4, 24, 8};
        if (type == MetaEventType::TimeSignature && data.size() >= 4) {
            ts.numerator = data[0];
            ts.denominator = 1 << data[1];  // 2^denominator
            ts.clocksPerMetronomeTick = data[2];
            ts.thirtySecondNotesPerQuarter = data[3];
        }
        return ts;
    }
    
    struct KeySignatureInfo {
        int8_t sharpsFlats;  // positive = sharps, negative = flats
        bool isMinor;
    };
    
    KeySignatureInfo getKeySignature() const {
        KeySignatureInfo ks = {0, false};
        if (type == MetaEventType::KeySignature && data.size() >= 2) {
            ks.sharpsFlats = static_cast<int8_t>(data[0]);
            ks.isMinor = (data[1] == 1);
        }
        return ks;
    }
    
    struct SMPTEInfo {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
        uint8_t frames;
        uint8_t subframes;
    };
    
    SMPTEInfo getSMPTEOffset() const {
        SMPTEInfo smpte = {0, 0, 0, 0, 0};
        if (type == MetaEventType::SMPTEOffset && data.size() >= 5) {
            smpte.hours = data[0] & 0x1F;
            smpte.minutes = data[1];
            smpte.seconds = data[2];
            smpte.frames = data[3];
            smpte.subframes = data[4];
        }
        return smpte;
    }
};

} // namespace midi

#endif // MIDI_META_EVENTS_H