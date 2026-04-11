#ifndef MIDI_READER_H
#define MIDI_READER_H

#include "midi_types.h"
#include <string>
#include <fstream>

namespace midi {

// Состояния парсера
enum class ParserState {
    IDLE,                   // Начальное состояние
    READ_HEADER_MAGIC,      // Чтение "MThd"
    READ_HEADER_SIZE,       // Чтение размера заголовка
    READ_HEADER_FORMAT,     // Чтение формата
    READ_HEADER_TRACKS,     // Чтение количества треков
    READ_HEADER_DIVISION,   // Чтение division
    READ_TRACK_MAGIC,       // Чтение "MTrk"
    READ_TRACK_LENGTH,      // Чтение длины трека
    READ_DELTA_TIME,        // Чтение delta time (VLQ)
    READ_STATUS,            // Чтение статус-байта
    READ_DATA,              // Чтение данных события
    READ_META_TYPE,         // Чтение типа мета-события
    READ_META_LENGTH,       // Чтение длины мета-события (VLQ)
    READ_META_DATA,         // Чтение данных мета-события
    READ_SYSEX_LENGTH,      // Чтение длины SysEx (VLQ)
    READ_SYSEX_DATA,        // Чтение данных SysEx
    DONE                    // Завершение
};

// Диагностическая информация об ошибке
struct ParseError {
    ParserState state;           // Состояние в момент ошибки
    std::string expected;        // Что ожидалось
    std::string got;             // Что получено
    uint32_t offset;             // Смещение в файле
    std::string details;         // Дополнительные детали
    
    std::string toString() const {
        std::string result = "Parse error at offset " + std::to_string(offset) + "\n";
        result += "  State: " + stateToString(state) + "\n";
        result += "  Expected: " + expected + "\n";
        result += "  Got: " + got + "\n";
        if (!details.empty()) result += "  Details: " + details + "\n";
        return result;
    }
    
    static std::string stateToString(ParserState s);
};

class MidiFileReader {
public:
    MidiFileReader();
    ~MidiFileReader();
    
    MidiFileData readFile(const std::string& filename);
    const std::string& getLastError() const { return m_lastError; }
    const ParseError& getLastParseError() const { return m_lastParseError; }
    
private:
    // Основные методы
    bool readFileInternal(std::ifstream& file, MidiFileData& data);
    bool processByte(uint8_t byte, MidiEvent& event, bool& eventReady, ParseError& error);
    void reset();
    
    // Вспомогательные методы
    uint16_t readUint16(std::ifstream& file);
    uint32_t readUint32(std::ifstream& file);
    uint32_t readVLQ(std::ifstream& file, size_t& bytesRead, ParseError& error);
    
    // Состояние парсера
    ParserState m_state;
    uint8_t m_runningStatus;
    uint8_t m_currentStatus;
    uint8_t m_metaType;
    uint32_t m_sysexLen;
    uint32_t m_metaLen;
    uint32_t m_bytesToRead;
    uint32_t m_bytesRead;
    std::vector<uint8_t> m_buffer;
    uint32_t m_currentDelta;
    uint32_t m_currentTick;
    
    // Для диагностики
    uint32_t m_currentOffset;
    std::string m_lastError;
    ParseError m_lastParseError;
};

} // namespace midi

#endif