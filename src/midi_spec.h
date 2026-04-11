#ifndef MIDI_SPEC_H
#define MIDI_SPEC_H

#include <cstdint>
#include <set>

namespace midi {

// Допустимые значения для различных MIDI параметров
struct MidiSpec {
    // Диапазоны
    static constexpr uint8_t MIN_NOTE = 0;
    static constexpr uint8_t MAX_NOTE = 127;
    static constexpr uint8_t MIN_VELOCITY = 0;
    static constexpr uint8_t MAX_VELOCITY = 127;
    static constexpr uint8_t MIN_CONTROLLER = 0;
    static constexpr uint8_t MAX_CONTROLLER = 127;
    static constexpr uint8_t MIN_PROGRAM = 0;
    static constexpr uint8_t MAX_PROGRAM = 127;
    static constexpr uint16_t MIN_PITCH_BEND = 0;
    static constexpr uint16_t MAX_PITCH_BEND = 16383;
    static constexpr uint8_t MIN_PRESSURE = 0;
    static constexpr uint8_t MAX_PRESSURE = 127;
    
    // Channel Mode контроллеры
    static constexpr uint8_t ALL_SOUND_OFF = 120;
    static constexpr uint8_t RESET_ALL_CONTROLLERS = 121;
    static constexpr uint8_t LOCAL_CONTROL = 122;
    static constexpr uint8_t ALL_NOTES_OFF = 123;
    static constexpr uint8_t OMNI_MODE_OFF = 124;
    static constexpr uint8_t OMNI_MODE_ON = 125;
    static constexpr uint8_t MONO_MODE_ON = 126;
    static constexpr uint8_t POLY_MODE_ON = 127;
    
    // Допустимые форматы
    static constexpr uint16_t FORMAT_0 = 0;
    static constexpr uint16_t FORMAT_1 = 1;
    static constexpr uint16_t FORMAT_2 = 2;
    
    // Допустимые SMPTE значения
    static constexpr int8_t SMPTE_24 = -24;
    static constexpr int8_t SMPTE_25 = -25;
    static constexpr int8_t SMPTE_29 = -29;
    static constexpr int8_t SMPTE_30 = -30;
    
    // Проверки
    static bool isValidNote(uint8_t note) {
        return note >= MIN_NOTE && note <= MAX_NOTE;
    }
    
    static bool isValidVelocity(uint8_t vel) {
        return vel >= MIN_VELOCITY && vel <= MAX_VELOCITY;
    }
    
    static bool isValidController(uint8_t ctrl) {
        return ctrl >= MIN_CONTROLLER && ctrl <= MAX_CONTROLLER;
    }
    
    static bool isValidProgram(uint8_t prog) {
        return prog >= MIN_PROGRAM && prog <= MAX_PROGRAM;
    }
    
    static bool isValidPitchBend(uint16_t bend) {
        return bend >= MIN_PITCH_BEND && bend <= MAX_PITCH_BEND;
    }
    
    static bool isValidPressure(uint8_t press) {
        return press >= MIN_PRESSURE && press <= MAX_PRESSURE;
    }
    
    static bool isValidFormat(uint16_t format) {
        return format == FORMAT_0 || format == FORMAT_1 || format == FORMAT_2;
    }
    
    static bool isValidSMPTE(int8_t smpte) {
        return smpte == SMPTE_24 || smpte == SMPTE_25 || 
               smpte == SMPTE_29 || smpte == SMPTE_30;
    }
    
    static bool isValidChannelMode(uint8_t controller, uint8_t value) {
        switch (controller) {
            case ALL_SOUND_OFF:
            case RESET_ALL_CONTROLLERS:
            case ALL_NOTES_OFF:
            case OMNI_MODE_OFF:
            case OMNI_MODE_ON:
                return value == 0;
            case LOCAL_CONTROL:
                return value == 0 || value == 127;
            case MONO_MODE_ON:
                return value >= 1 && value <= 16;
            case POLY_MODE_ON:
                return value == 0;
            default:
                return true;
        }
    }
};

// Допустимые типы мета-событий
const std::set<uint8_t> VALID_META_TYPES = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x20, 0x2F, 0x51, 0x54, 0x58, 0x59, 0x7F
};

// Допустимые значения времени (SMPTE)
const std::set<int8_t> VALID_SMPTE_VALUES = {-24, -25, -29, -30};

} // namespace midi

#endif