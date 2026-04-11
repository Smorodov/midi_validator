#include "midi_analyzer.h"
#include "midi_meta_events.h"
#include "midi_channel_mode.h"
#include "midi_system_common.h"
#include <set>

namespace midi {

// Дополнительные проверки для расширенной валидации
class ExtendedValidator {
public:
    ExtendedValidator() : currentChannelPrefix(-1) {}
    
    void validateMetaEvent(const MetaEventData& meta, AnalysisResult& result) {
        switch (meta.type) {
            case MetaEventType::SequenceNumber:
                validateSequenceNumber(meta, result);
                break;
            case MetaEventType::MIDIChannelPrefix:
                validateChannelPrefix(meta, result);
                break;
            case MetaEventType::TimeSignature:
                validateTimeSignature(meta, result);
                break;
            case MetaEventType::KeySignature:
                validateKeySignature(meta, result);
                break;
            case MetaEventType::SMPTEOffset:
                validateSMPTEOffset(meta, result);
                break;
            default:
                // Текстовые мета-события всегда валидны
                break;
        }
    }
    
    void validateChannelMode(const ChannelModeMessage& mode, AnalysisResult& result) {
        if (!mode.isValid()) {
            result.addWarning("Invalid channel mode message: " + 
                             std::string(mode.getName()) + 
                             " with value " + std::to_string(mode.value));
        }
        
        // Отслеживаем состояние канала
        switch (mode.type) {
            case ChannelModeType::OmniModeOn:
                channelStates[mode.channel].omniMode = true;
                break;
            case ChannelModeType::OmniModeOff:
                channelStates[mode.channel].omniMode = false;
                break;
            case ChannelModeType::MonoModeOn:
                channelStates[mode.channel].monoMode = true;
                channelStates[mode.channel].monoChannels = mode.value;
                break;
            case ChannelModeType::PolyModeOn:
                channelStates[mode.channel].monoMode = false;
                break;
            case ChannelModeType::AllNotesOff:
                channelStates[mode.channel].allNotesOffReceived = true;
                break;
            default:
                break;
        }
    }
    
    void validateRunningStatusReset(const MidiEvent& event, uint8_t runningStatus) {
        // Согласно спецификации, Meta и SysEx события сбрасывают running status
        if (event.type == EventType::MetaEvent || 
            event.type == EventType::SysEx || 
            event.type == EventType::SysExEnd) {
            runningStatus = 0;
        }
    }
    
private:
    struct ChannelState {
        bool omniMode = true;
        bool monoMode = false;
        int monoChannels = 0;
        bool allNotesOffReceived = false;
    };
    
    int currentChannelPrefix;
    std::map<int, ChannelState> channelStates;
    
    void validateSequenceNumber(const MetaEventData& meta, AnalysisResult& result) {
        if (meta.data.size() != 2) {
            result.addWarning("Invalid sequence number length: " + 
                            std::to_string(meta.data.size()));
        }
    }
    
    void validateChannelPrefix(const MetaEventData& meta, AnalysisResult& result) {
        if (meta.data.size() >= 1) {
            currentChannelPrefix = meta.data[0];
            if (currentChannelPrefix < 0 || currentChannelPrefix > 15) {
                result.addWarning("Invalid MIDI channel prefix: " + 
                                std::to_string(currentChannelPrefix));
            }
        }
    }
    
    void validateTimeSignature(const MetaEventData& meta, AnalysisResult& result) {
        auto ts = meta.getTimeSignature();
        if (ts.numerator < 1 || ts.numerator > 32) {
            result.addWarning("Unusual time signature numerator: " + 
                            std::to_string(ts.numerator));
        }
        if (ts.denominator < 1 || ts.denominator > 32) {
            result.addWarning("Unusual time signature denominator: " + 
                            std::to_string(ts.denominator));
        }
    }
    
    void validateKeySignature(const MetaEventData& meta, AnalysisResult& result) {
        auto ks = meta.getKeySignature();
        if (ks.sharpsFlats < -7 || ks.sharpsFlats > 7) {
            result.addWarning("Invalid key signature: " + 
                            std::to_string(ks.sharpsFlats) + " sharps/flats");
        }
    }
    
    void validateSMPTEOffset(const MetaEventData& meta, AnalysisResult& result) {
        auto smpte = meta.getSMPTEOffset();
        if (smpte.hours > 23 || smpte.minutes > 59 || smpte.seconds > 59) {
            result.addWarning("Invalid SMPTE offset time");
        }
    }
};

// Обновленная функция для извлечения мета-событий
MetaEventData extractMetaEventData(const MidiEvent& event, uint32_t tickPosition) {
    MetaEventData meta;
    meta.deltaTime = event.deltaTime;
    meta.tickPosition = tickPosition;
    
    if (event.type == EventType::MetaEvent && !event.data.empty()) {
        meta.type = static_cast<MetaEventType>(event.data[0]);
        if (event.data.size() > 1) {
            meta.data.assign(event.data.begin() + 1, event.data.end());
        }
    }
    
    return meta;
}

} // namespace midi