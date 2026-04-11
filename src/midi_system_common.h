#ifndef MIDI_SYSTEM_COMMON_H
#define MIDI_SYSTEM_COMMON_H

#include <cstdint>

namespace midi {

// System Common Messages
enum class SystemCommonType : uint8_t {
    SysEx              = 0xF0,
    SongPositionPointer = 0xF2,
    SongSelect         = 0xF3,
    TuneRequest        = 0xF6,
    SysExEnd           = 0xF7
};

struct SongPositionPointer {
    uint16_t position;  // Number of MIDI beats (16th notes) from start
    
    static SongPositionPointer fromData(const uint8_t* data) {
        SongPositionPointer spp;
        spp.position = (data[1] << 7) | data[0];
        return spp;
    }
};

struct SongSelect {
    uint8_t songNumber;  // 0-127
};

// System Real-Time Messages
enum class SystemRealTimeType : uint8_t {
    TimingClock        = 0xF8,
    Start              = 0xFA,
    Continue           = 0xFB,
    Stop               = 0xFC,
    ActiveSensing      = 0xFE,
    Reset              = 0xFF
};

} // namespace midi

#endif // MIDI_SYSTEM_COMMON_H