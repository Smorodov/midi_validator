#ifndef MIDI_READER_SM_H
#define MIDI_READER_SM_H

#include "midi_types.h"
#include <string>
#include <fstream>

namespace midi {

// Состояния парсера
enum class ParserState {
    WAIT_STATUS,        // Ожидание статус-байта
    READ_NOTE,          // Чтение note и velocity (2 байта)
    READ_CONTROL,       // Чтение controller и value (2 байта)
    READ_PROGRAM,       // Чтение program (1 байт)
    READ_PRESSURE,      // Чтение pressure (1 байт)
    READ_PITCH,         // Чтение pitch bend (2 байта)
    READ_SYSEX_LEN,     // Чтение длины SysEx
    READ_SYSEX_DATA,    // Чтение данных SysEx
    READ_META_TYPE,     // Чтение типа мета-события
    READ_META_LEN,      // Чтение длины мета-события
    READ_META_DATA,     // Чтение данных мета-события
    READ_SONG_POS,      // Чтение Song Position Pointer (2 байта)
    READ_SONG_SEL       // Чтение Song Select (1 байт)
};

class MidiParserSM {
public:
    MidiParserSM();
    ~MidiParserSM();
    
    MidiFileData parseFile(const std::string& filename);
    
private:
    ParserState m_state;
    uint8_t m_runningStatus;
    uint8_t m_currentStatus;
    uint8_t m_metaType;
    uint32_t m_sysexLen;
    uint32_t m_metaLen;
    uint32_t m_bytesToRead;
    std::vector<uint8_t> m_buffer;
    
    void reset();
    bool processByte(uint8_t byte, MidiEvent& event, bool& eventReady);
    void addToBuffer(uint8_t byte);
    void clearBuffer();
};

} // namespace midi

#endif