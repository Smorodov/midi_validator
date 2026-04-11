#include "midi_reader_sm.h"
#include <fstream>
#include <iostream>

namespace midi {

MidiParserSM::MidiParserSM() {
    reset();
}

MidiParserSM::~MidiParserSM() {}

void MidiParserSM::reset() {
    m_state = ParserState::WAIT_STATUS;
    m_runningStatus = 0;
    m_currentStatus = 0;
    m_metaType = 0;
    m_sysexLen = 0;
    m_metaLen = 0;
    m_bytesToRead = 0;
    m_buffer.clear();
}

void MidiParserSM::clearBuffer() {
    m_buffer.clear();
}

void MidiParserSM::addToBuffer(uint8_t byte) {
    m_buffer.push_back(byte);
}

bool MidiParserSM::processByte(uint8_t byte, MidiEvent& event, bool& eventReady) {
    eventReady = false;
    
    switch (m_state) {
        case ParserState::WAIT_STATUS:
            if (byte & 0x80) {
                // Это статус-байт
                m_currentStatus = byte;
                m_runningStatus = byte;
                event.type = static_cast<EventType>(byte & 0xF0);
                event.channel = byte & 0x0F;
                
                if (byte == 0xFF) { // Meta event
                    m_state = ParserState::READ_META_TYPE;
                } else if (byte == 0xF0 || byte == 0xF7) { // SysEx
                    m_state = ParserState::READ_SYSEX_LEN;
                    m_sysexLen = 0;
                } else if (byte >= 0xF8) { // System Real-Time
                    event.type = static_cast<EventType>(byte);
                    eventReady = true;
                } else {
                    // Channel voice message
                    uint8_t cmd = byte & 0xF0;
                    switch (cmd) {
                        case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0:
                            m_state = ParserState::READ_NOTE;
                            m_bytesToRead = 2;
                            clearBuffer();
                            break;
                        case 0xC0: case 0xD0:
                            m_state = ParserState::READ_PROGRAM;
                            m_bytesToRead = 1;
                            clearBuffer();
                            break;
                        default:
                            // Unknown, try to recover
                            m_state = ParserState::WAIT_STATUS;
                            break;
                    }
                }
            } else {
                // Это байт данных - используем running status
                if (m_runningStatus == 0) {
                    return false; // Invalid running status
                }
                event.usesRunningStatus = true;
                m_currentStatus = m_runningStatus;
                event.type = static_cast<EventType>(m_runningStatus & 0xF0);
                event.channel = m_runningStatus & 0x0F;
                
                // Обрабатываем как данные для текущего статуса
                uint8_t cmd = m_runningStatus & 0xF0;
                switch (cmd) {
                    case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0:
                        m_state = ParserState::READ_NOTE;
                        m_bytesToRead = 2;
                        clearBuffer();
                        addToBuffer(byte); // Первый байт данных
                        break;
                    case 0xC0: case 0xD0:
                        m_state = ParserState::READ_PROGRAM;
                        m_bytesToRead = 1;
                        clearBuffer();
                        addToBuffer(byte);
                        break;
                    default:
                        m_state = ParserState::WAIT_STATUS;
                        break;
                }
            }
            break;
            
        case ParserState::READ_NOTE:
        case ParserState::READ_CONTROL:
        case ParserState::READ_PITCH:
            addToBuffer(byte);
            if (m_buffer.size() >= m_bytesToRead) {
                event.data = m_buffer;
                eventReady = true;
                m_state = ParserState::WAIT_STATUS;
            }
            break;
            
        case ParserState::READ_PROGRAM:
        case ParserState::READ_PRESSURE:
            addToBuffer(byte);
            if (m_buffer.size() >= m_bytesToRead) {
                event.data = m_buffer;
                eventReady = true;
                m_state = ParserState::WAIT_STATUS;
            }
            break;
            
        case ParserState::READ_META_TYPE:
            m_metaType = byte;
            m_state = ParserState::READ_META_LEN;
            m_metaLen = 0;
            break;
            
        case ParserState::READ_META_LEN:
            m_metaLen = (m_metaLen << 7) | (byte & 0x7F);
            if ((byte & 0x80) == 0) {
                if (m_metaLen == 0) {
                    // Пустое мета-событие (например, End of Track)
                    event.type = EventType::MetaEvent;
                    event.data = {m_metaType};
                    eventReady = true;
                    m_state = ParserState::WAIT_STATUS;
                } else {
                    m_state = ParserState::READ_META_DATA;
                    clearBuffer();
                }
            }
            break;
            
        case ParserState::READ_META_DATA:
            addToBuffer(byte);
            if (m_buffer.size() >= m_metaLen) {
                event.type = EventType::MetaEvent;
                event.data = {m_metaType};
                event.data.insert(event.data.end(), m_buffer.begin(), m_buffer.end());
                eventReady = true;
                m_state = ParserState::WAIT_STATUS;
            }
            break;
            
        case ParserState::READ_SYSEX_LEN:
            m_sysexLen = (m_sysexLen << 7) | (byte & 0x7F);
            if ((byte & 0x80) == 0) {
                if (m_sysexLen == 0) {
                    event.type = (m_currentStatus == 0xF0) ? EventType::SysEx : EventType::SysExEnd;
                    eventReady = true;
                    m_state = ParserState::WAIT_STATUS;
                } else {
                    m_state = ParserState::READ_SYSEX_DATA;
                    clearBuffer();
                }
            }
            break;
            
        case ParserState::READ_SYSEX_DATA:
            addToBuffer(byte);
            if (m_buffer.size() >= m_sysexLen) {
                event.type = (m_currentStatus == 0xF0) ? EventType::SysEx : EventType::SysExEnd;
                event.data = m_buffer;
                eventReady = true;
                m_state = ParserState::WAIT_STATUS;
            }
            break;
            
        default:
            m_state = ParserState::WAIT_STATUS;
            break;
    }
    
    return true;
}

MidiFileData MidiParserSM::parseFile(const std::string& filename) {
    MidiFileData data;
    reset();
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        data.isValid = false;
        data.errorMessage = "Cannot open file";
        return data;
    }
    
    file.seekg(0, std::ios::end);
    data.fileSize = static_cast<uint32_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    // Читаем заголовок (как в обычном парсере)
    char magic[4];
    file.read(magic, 4);
    if (std::string(magic, 4) != "MThd") {
        data.isValid = false;
        data.errorMessage = "Not a MIDI file";
        file.close();
        return data;
    }
    
    uint32_t headerSize = 0;
    file.read(reinterpret_cast<char*>(&headerSize), 4);
    headerSize = ((headerSize & 0x000000FF) << 24) |
                 ((headerSize & 0x0000FF00) << 8) |
                 ((headerSize & 0x00FF0000) >> 8) |
                 ((headerSize & 0xFF000000) >> 24);
    
    if (headerSize != 6) {
        data.isValid = false;
        data.errorMessage = "Invalid header size";
        file.close();
        return data;
    }
    
    uint16_t format = 0;
    file.read(reinterpret_cast<char*>(&format), 2);
    format = ((format & 0xFF00) >> 8) | ((format & 0x00FF) << 8);
    data.format = format;
    
    uint16_t numTracks = 0;
    file.read(reinterpret_cast<char*>(&numTracks), 2);
    numTracks = ((numTracks & 0xFF00) >> 8) | ((numTracks & 0x00FF) << 8);
    data.numTracks = numTracks;
    
    uint16_t division = 0;
    file.read(reinterpret_cast<char*>(&division), 2);
    division = ((division & 0xFF00) >> 8) | ((division & 0x00FF) << 8);
    data.division = division;
    
    if (division & 0x8000) {
        data.isSMPTE = true;
        data.smpteFrames = (division >> 8) & 0x7F;
        data.smpteResolution = division & 0xFF;
    }
    
    // Читаем треки
    for (int t = 0; t < data.numTracks; t++) {
        file.read(magic, 4);
        if (std::string(magic, 4) != "MTrk") {
            data.isValid = false;
            data.errorMessage = "Invalid track chunk";
            file.close();
            return data;
        }
        
        TrackData track;
        track.index = t;
        
        uint32_t trackLen = 0;
        file.read(reinterpret_cast<char*>(&trackLen), 4);
        trackLen = ((trackLen & 0x000000FF) << 24) |
                   ((trackLen & 0x0000FF00) << 8) |
                   ((trackLen & 0x00FF0000) >> 8) |
                   ((trackLen & 0xFF000000) >> 24);
        track.length = trackLen;
        
        std::streampos trackStart = file.tellg();
        std::streampos trackEnd = trackStart + trackLen;
        
        reset();
        uint32_t currentDelta = 0;
        
        while (file.tellg() < trackEnd && !file.eof()) {
            // Читаем delta time
            uint32_t delta = 0;
            uint8_t byte;
            do {
                file.read(reinterpret_cast<char*>(&byte), 1);
                delta = (delta << 7) | (byte & 0x7F);
            } while (byte & 0x80);
            
            currentDelta += delta;
            
            // Читаем событие через конечный автомат
            bool eventReady = false;
            MidiEvent event;
            event.deltaTime = delta;
            
            while (!eventReady && file.tellg() < trackEnd && !file.eof()) {
                file.read(reinterpret_cast<char*>(&byte), 1);
                if (!processByte(byte, event, eventReady)) {
                    data.isValid = false;
                    data.errorMessage = "Parse error";
                    file.close();
                    return data;
                }
            }
            
            if (eventReady) {
                if (event.type == EventType::MetaEvent && !event.data.empty()) {
                    if (event.data[0] == 0x2F) track.hasEndOfTrack = true;
                    if (event.data[0] == 0x03 && event.data.size() > 1) {
                        track.trackName = std::string(event.data.begin() + 1, event.data.end());
                    }
                    if (event.data[0] == 0x04 && event.data.size() > 1) {
                        track.instrumentName = std::string(event.data.begin() + 1, event.data.end());
                    }
                }
                track.events.push_back(event);
            }
        }
        
        data.tracks.push_back(track);
    }
    
    file.close();
    
    if (data.tracks.size() != data.numTracks) {
        data.isValid = false;
        data.errorMessage = "Track count mismatch";
    }
    
    return data;
}

} // namespace midi