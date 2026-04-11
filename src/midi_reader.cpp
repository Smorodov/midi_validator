#include "midi_reader.h"
#include <cstring>
#include <iostream>

namespace midi {

std::string ParseError::stateToString(ParserState s) {
    switch (s) {
        case ParserState::IDLE: return "IDLE";
        case ParserState::READ_HEADER_MAGIC: return "READ_HEADER_MAGIC";
        case ParserState::READ_HEADER_SIZE: return "READ_HEADER_SIZE";
        case ParserState::READ_HEADER_FORMAT: return "READ_HEADER_FORMAT";
        case ParserState::READ_HEADER_TRACKS: return "READ_HEADER_TRACKS";
        case ParserState::READ_HEADER_DIVISION: return "READ_HEADER_DIVISION";
        case ParserState::READ_TRACK_MAGIC: return "READ_TRACK_MAGIC";
        case ParserState::READ_TRACK_LENGTH: return "READ_TRACK_LENGTH";
        case ParserState::READ_DELTA_TIME: return "READ_DELTA_TIME";
        case ParserState::READ_STATUS: return "READ_STATUS";
        case ParserState::READ_DATA: return "READ_DATA";
        case ParserState::READ_META_TYPE: return "READ_META_TYPE";
        case ParserState::READ_META_LENGTH: return "READ_META_LENGTH";
        case ParserState::READ_META_DATA: return "READ_META_DATA";
        case ParserState::READ_SYSEX_LENGTH: return "READ_SYSEX_LENGTH";
        case ParserState::READ_SYSEX_DATA: return "READ_SYSEX_DATA";
        case ParserState::DONE: return "DONE";
        default: return "UNKNOWN";
    }
}

MidiFileReader::MidiFileReader() {
    reset();
}

MidiFileReader::~MidiFileReader() {}

void MidiFileReader::reset() {
    m_state = ParserState::IDLE;
    m_runningStatus = 0;
    m_currentStatus = 0;
    m_metaType = 0;
    m_sysexLen = 0;
    m_metaLen = 0;
    m_bytesToRead = 0;
    m_bytesRead = 0;
    m_buffer.clear();
    m_currentDelta = 0;
    m_currentTick = 0;
    m_currentOffset = 0;
    m_lastError.clear();
}

MidiFileData MidiFileReader::readFile(const std::string& filename) {
    MidiFileData data;
    reset();
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        data.isValid = false;
        data.errorMessage = "Cannot open file: " + filename;
        return data;
    }
    
    file.seekg(0, std::ios::end);
    data.fileSize = static_cast<uint32_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    if (!readFileInternal(file, data)) {
        data.isValid = false;
        if (data.errorMessage.empty()) {
            data.errorMessage = m_lastParseError.toString();
        }
    }
    
    file.close();
    return data;
}

bool MidiFileReader::readFileInternal(std::ifstream& file, MidiFileData& data) {
    // Состояние: ожидание заголовка
    m_state = ParserState::READ_HEADER_MAGIC;
    m_currentOffset = 0;
    
    // Читаем заголовок
    char magic[4];
    file.read(magic, 4);
    m_currentOffset += 4;
    
    if (std::string(magic, 4) != "MThd") {
        m_lastParseError.state = m_state;
        m_lastParseError.expected = "MThd";
        m_lastParseError.got = std::string(magic, 4);
        m_lastParseError.offset = m_currentOffset - 4;
        data.errorMessage = m_lastParseError.toString();
        return false;
    }
    
    m_state = ParserState::READ_HEADER_SIZE;
    uint32_t headerSize = readUint32(file);
    m_currentOffset += 4;
    
    if (headerSize != 6) {
        m_lastParseError.state = m_state;
        m_lastParseError.expected = "6";
        m_lastParseError.got = std::to_string(headerSize);
        m_lastParseError.offset = m_currentOffset - 4;
        data.errorMessage = m_lastParseError.toString();
        return false;
    }
    
    m_state = ParserState::READ_HEADER_FORMAT;
    data.format = readUint16(file);
    m_currentOffset += 2;
    
    m_state = ParserState::READ_HEADER_TRACKS;
    data.numTracks = readUint16(file);
    m_currentOffset += 2;
    
    m_state = ParserState::READ_HEADER_DIVISION;
    data.division = readUint16(file);
    m_currentOffset += 2;
    
    if (data.division & 0x8000) {
        data.isSMPTE = true;
        data.smpteFrames = (data.division >> 8) & 0x7F;
        data.smpteResolution = data.division & 0xFF;
    }
    
    // Читаем треки
    for (int t = 0; t < data.numTracks; t++) {
        m_state = ParserState::READ_TRACK_MAGIC;
        
        char trackMagic[4];
        file.read(trackMagic, 4);
        m_currentOffset += 4;
        
        if (std::string(trackMagic, 4) != "MTrk") {
            m_lastParseError.state = m_state;
            m_lastParseError.expected = "MTrk";
            m_lastParseError.got = std::string(trackMagic, 4);
            m_lastParseError.offset = m_currentOffset - 4;
            data.errorMessage = m_lastParseError.toString();
            return false;
        }
        
        m_state = ParserState::READ_TRACK_LENGTH;
        uint32_t trackLen = readUint32(file);
        m_currentOffset += 4;
        
        if (trackLen == 0 || trackLen > 100 * 1024 * 1024) {
            m_lastParseError.state = m_state;
            m_lastParseError.expected = "1-104857600";
            m_lastParseError.got = std::to_string(trackLen);
            m_lastParseError.offset = m_currentOffset - 4;
            data.errorMessage = m_lastParseError.toString();
            return false;
        }
        
        std::streampos trackStart = file.tellg();
        std::streampos trackEnd = trackStart + trackLen;
        
        TrackData track;
        track.index = t;
        track.offset = m_currentOffset - 8;
        track.length = trackLen;
        track.hasEndOfTrack = false;
        
        m_runningStatus = 0;
        m_currentDelta = 0;
        m_currentTick = 0;
        m_state = ParserState::READ_DELTA_TIME;
        
        while (file.tellg() < trackEnd && !file.eof()) {
            MidiEvent event;
            bool eventReady = false;
            ParseError eventError;
            
            // Читаем байты до готовности события
            while (!eventReady && file.tellg() < trackEnd && !file.eof()) {
                uint8_t byte;
                file.read(reinterpret_cast<char*>(&byte), 1);
                if (file.gcount() == 0) break;
                m_currentOffset++;
                
                if (!processByte(byte, event, eventReady, eventError)) {
                    m_lastParseError = eventError;
                    data.errorMessage = m_lastParseError.toString();
                    return false;
                }
            }
            
            if (eventReady) {
                event.deltaTime = m_currentDelta;
                m_currentTick += m_currentDelta;
                
                if (event.type == EventType::MetaEvent && !event.data.empty()) {
                    if (event.data[0] == MetaEvent::END_OF_TRACK) {
                        track.hasEndOfTrack = true;
                    } else if (event.data[0] == MetaEvent::TRACK_NAME && event.data.size() > 1) {
                        track.trackName = std::string(event.data.begin() + 1, event.data.end());
                    } else if (event.data[0] == MetaEvent::INSTRUMENT_NAME && event.data.size() > 1) {
                        track.instrumentName = std::string(event.data.begin() + 1, event.data.end());
                    }
                }
                
                track.events.push_back(event);
                m_state = ParserState::READ_DELTA_TIME;
            }
        }
        
        data.tracks.push_back(track);
        
        if (file.tellg() != trackEnd) {
            m_lastParseError.state = m_state;
            m_lastParseError.expected = "End of track at " + std::to_string(trackEnd);
            m_lastParseError.got = "Position " + std::to_string(file.tellg());
            m_lastParseError.offset = m_currentOffset;
            data.errorMessage = m_lastParseError.toString();
            return false;
        }
    }
    
    m_state = ParserState::DONE;
    
    if (data.tracks.size() != data.numTracks) {
        m_lastParseError.state = m_state;
        m_lastParseError.expected = std::to_string(data.numTracks) + " tracks";
        m_lastParseError.got = std::to_string(data.tracks.size()) + " tracks";
        data.errorMessage = m_lastParseError.toString();
        return false;
    }
    
    return true;
}

bool MidiFileReader::processByte(uint8_t byte, MidiEvent& event, bool& eventReady, ParseError& error) {
    eventReady = false;
    error.state = m_state;
    error.offset = m_currentOffset;
    
    switch (m_state) {
        case ParserState::READ_DELTA_TIME: {
            m_currentDelta = (m_currentDelta << 7) | (byte & 0x7F);
            if ((byte & 0x80) == 0) {
                m_state = ParserState::READ_STATUS;
                m_bytesRead = 0;
                m_buffer.clear();
            }
            break;
        }
        
        case ParserState::READ_STATUS: {
            if (byte & 0x80) {
                // Статус-байт
                m_currentStatus = byte;
                m_runningStatus = byte;
                event.type = static_cast<EventType>(byte & 0xF0);
                event.channel = byte & 0x0F;
                
                if (byte == 0xFF) {
                    m_state = ParserState::READ_META_TYPE;
                } else if (byte == 0xF0 || byte == 0xF7) {
                    m_state = ParserState::READ_SYSEX_LENGTH;
                    m_sysexLen = 0;
                } else if (byte >= 0xF8) {
                    event.type = static_cast<EventType>(byte);
                    eventReady = true;
                } else {
                    uint8_t cmd = byte & 0xF0;
                    switch (cmd) {
                        case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0:
                            m_state = ParserState::READ_DATA;
                            m_bytesToRead = 2;
                            m_bytesRead = 0;
                            m_buffer.clear();
                            break;
                        case 0xC0: case 0xD0:
                            m_state = ParserState::READ_DATA;
                            m_bytesToRead = 1;
                            m_bytesRead = 0;
                            m_buffer.clear();
                            break;
                        default:
                            error.expected = "Valid command (0x80-0xEF)";
                            error.got = "0x" + std::to_string(byte);
                            error.details = "Unknown MIDI command";
                            return false;
                    }
                }
            } else {
                // Running status
                if (m_runningStatus == 0) {
                    error.expected = "Status byte (0x80-0xFF) or valid running status";
                    error.got = "0x" + std::to_string(byte);
                    error.details = "No previous status to run";
                    return false;
                }
                event.usesRunningStatus = true;
                m_currentStatus = m_runningStatus;
                event.type = static_cast<EventType>(m_runningStatus & 0xF0);
                event.channel = m_runningStatus & 0x0F;
                
                uint8_t cmd = m_runningStatus & 0xF0;
                switch (cmd) {
                    case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0:
                        m_state = ParserState::READ_DATA;
                        m_bytesToRead = 2;
                        m_bytesRead = 1;
                        m_buffer.clear();
                        m_buffer.push_back(byte);
                        break;
                    case 0xC0: case 0xD0:
                        m_state = ParserState::READ_DATA;
                        m_bytesToRead = 1;
                        m_bytesRead = 1;
                        m_buffer.clear();
                        m_buffer.push_back(byte);
                        break;
                    default:
                        error.expected = "Valid command for running status";
                        error.got = "0x" + std::to_string(cmd);
                        return false;
                }
            }
            break;
        }
        
        case ParserState::READ_DATA: {
            m_buffer.push_back(byte);
            m_bytesRead++;
            if (m_bytesRead >= m_bytesToRead) {
                event.data = m_buffer;
                eventReady = true;
            }
            break;
        }
        
        case ParserState::READ_META_TYPE: {
            m_metaType = byte;
            m_state = ParserState::READ_META_LENGTH;
            m_metaLen = 0;
            break;
        }
        
        case ParserState::READ_META_LENGTH: {
            m_metaLen = (m_metaLen << 7) | (byte & 0x7F);
            if ((byte & 0x80) == 0) {
                if (m_metaLen == 0) {
                    event.type = EventType::MetaEvent;
                    event.data = {m_metaType};
                    eventReady = true;
                } else {
                    m_state = ParserState::READ_META_DATA;
                    m_bytesRead = 0;
                    m_buffer.clear();
                }
            }
            break;
        }
        
        case ParserState::READ_META_DATA: {
            m_buffer.push_back(byte);
            m_bytesRead++;
            if (m_bytesRead >= m_metaLen) {
                event.type = EventType::MetaEvent;
                event.data = {m_metaType};
                event.data.insert(event.data.end(), m_buffer.begin(), m_buffer.end());
                eventReady = true;
            }
            break;
        }
        
        case ParserState::READ_SYSEX_LENGTH: {
            m_sysexLen = (m_sysexLen << 7) | (byte & 0x7F);
            if ((byte & 0x80) == 0) {
                if (m_sysexLen == 0) {
                    event.type = (m_currentStatus == 0xF0) ? EventType::SysEx : EventType::SysExEnd;
                    eventReady = true;
                } else {
                    m_state = ParserState::READ_SYSEX_DATA;
                    m_bytesRead = 0;
                    m_buffer.clear();
                }
            }
            break;
        }
        
        case ParserState::READ_SYSEX_DATA: {
            m_buffer.push_back(byte);
            m_bytesRead++;
            if (m_bytesRead >= m_sysexLen) {
                event.type = (m_currentStatus == 0xF0) ? EventType::SysEx : EventType::SysExEnd;
                event.data = m_buffer;
                eventReady = true;
            }
            break;
        }
        
        default: {
            error.expected = "Valid parser state";
            error.got = std::to_string(static_cast<int>(m_state));
            error.details = "Invalid parser state";
            return false;
        }
    }
    
    return true;
}

uint16_t MidiFileReader::readUint16(std::ifstream& file) {
    uint8_t bytes[2];
    file.read(reinterpret_cast<char*>(bytes), 2);
    return (bytes[0] << 8) | bytes[1];
}

uint32_t MidiFileReader::readUint32(std::ifstream& file) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

uint32_t MidiFileReader::readVLQ(std::ifstream& file, size_t& bytesRead, ParseError& error) {
    uint32_t value = 0;
    uint8_t byte;
    bytesRead = 0;
    
    do {
        if (!file.read(reinterpret_cast<char*>(&byte), 1)) {
            error.expected = "VLQ byte";
            error.got = "EOF";
            return 0;
        }
        bytesRead++;
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    
    return value;
}

} // namespace midi