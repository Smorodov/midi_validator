#ifndef MIDI_CHANNEL_MODE_H
#define MIDI_CHANNEL_MODE_H

#include <cstdint>

namespace midi {

// Channel Mode Messages (Control Change with controller 120-127)
enum class ChannelModeType : uint8_t {
    AllSoundOff        = 120,
    ResetAllControllers = 121,
    LocalControl       = 122,
    AllNotesOff        = 123,
    OmniModeOff        = 124,
    OmniModeOn         = 125,
    MonoModeOn         = 126,
    PolyModeOn         = 127
};

struct ChannelModeMessage {
    ChannelModeType type;
    uint8_t channel;
    uint8_t value;  // Для некоторых сообщений имеет специальное значение
    
    bool isValid() const {
        switch (type) {
            case ChannelModeType::AllSoundOff:
            case ChannelModeType::ResetAllControllers:
            case ChannelModeType::AllNotesOff:
            case ChannelModeType::OmniModeOff:
            case ChannelModeType::OmniModeOn:
                return value == 0;
            case ChannelModeType::LocalControl:
                return value == 0 || value == 127;
            case ChannelModeType::MonoModeOn:
                return value >= 1 && value <= 16;
            case ChannelModeType::PolyModeOn:
                return value == 0;
            default:
                return false;
        }
    }
    
    const char* getName() const {
        switch (type) {
            case ChannelModeType::AllSoundOff: return "All Sound Off";
            case ChannelModeType::ResetAllControllers: return "Reset All Controllers";
            case ChannelModeType::LocalControl: return "Local Control";
            case ChannelModeType::AllNotesOff: return "All Notes Off";
            case ChannelModeType::OmniModeOff: return "Omni Mode Off";
            case ChannelModeType::OmniModeOn: return "Omni Mode On";
            case ChannelModeType::MonoModeOn: return "Mono Mode On";
            case ChannelModeType::PolyModeOn: return "Poly Mode On";
            default: return "Unknown";
        }
    }
};

} // namespace midi

#endif // MIDI_CHANNEL_MODE_H