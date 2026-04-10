#include "midi_analyzer.h"
#include <iostream>
#include <fstream>
#include <cstring>

uint16_t MidiAnalyzer::read_be16(const uint8_t* p) {
    return (p[0] << 8) | p[1];
}

uint32_t MidiAnalyzer::read_be32(const uint8_t* p) {
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

uint32_t MidiAnalyzer::read_varlen(std::ifstream& file) {
    uint32_t value = 0;
    uint8_t byte;
    do {
        if (!file.read(reinterpret_cast<char*>(&byte), 1)) {
            return value;
        }
        value = (value << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    return value;
}

MidiInfo MidiAnalyzer::analyze(const std::string& filename) {
    MidiInfo info;
    memset(&info, 0, sizeof(info));
    info.is_valid = true;
    info.min_note = 127;
    info.max_note = 0;
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        info.is_valid = false;
        info.error_msg = "Cannot open file";
        return info;
    }
    
    file.seekg(0, std::ios::end);
    info.file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    uint8_t header[14];
    file.read(reinterpret_cast<char*>(header), 14);
    if (file.gcount() != 14) {
        info.is_valid = false;
        info.error_msg = "File too small";
        return info;
    }
    
    if (memcmp(header, "MThd", 4) != 0) {
        info.is_valid = false;
        info.error_msg = "Not a MIDI file";
        return info;
    }
    
    if (read_be32(header + 4) != 6) {
        info.is_valid = false;
        info.error_msg = "Invalid header length";
        return info;
    }
    
    info.format = read_be16(header + 8);
    info.num_tracks = read_be16(header + 10);
    info.division = read_be16(header + 12);
    
    for (int t = 0; t < info.num_tracks && info.is_valid; t++) {
        uint8_t track_header[8];
        file.read(reinterpret_cast<char*>(track_header), 8);
        if (file.gcount() != 8) {
            info.is_valid = false;
            info.error_msg = "Cannot read track header";
            break;
        }
        
        if (memcmp(track_header, "MTrk", 4) != 0) {
            info.is_valid = false;
            info.error_msg = "Invalid track signature";
            break;
        }
        
        uint32_t track_len = read_be32(track_header + 4);
        info.tracks_found++;
        
        std::streampos track_start = file.tellg();
        uint32_t pos = 0;
        uint8_t running_status = 0;
        bool has_eot = false;
        
        while (pos < track_len && file && info.is_valid) {
            // ===== READ DELTA TIME (FIXED) =====
            uint32_t delta_time = 0;
            uint8_t byte;
            do {
                if (!file.read(reinterpret_cast<char*>(&byte), 1)) {
                    break;
                }
                pos++;
                delta_time = (delta_time << 7) | (byte & 0x7F);
            } while (byte & 0x80);
            // delta_time is now correctly stored (not used further but needed for positioning)
            
            // ===== READ STATUS =====
            uint8_t status;
            if (!file.read(reinterpret_cast<char*>(&status), 1)) {
                break;
            }
            pos++;
            
            // Running status
            if (status < 0x80) {
                if (running_status == 0) {
                    info.is_valid = false;
                    info.error_msg = "Running status without previous event";
                    break;
                }
                // Put back the data byte and use running status
                file.seekg(-1, std::ios::cur);
                pos--;
                status = running_status;
            } else {
                running_status = status;
            }
            
            uint8_t cmd = status & 0xF0;
            
            // ===== META EVENT =====
            if (status == 0xFF) {
                uint8_t meta_type;
                if (!file.read(reinterpret_cast<char*>(&meta_type), 1)) {
                    break;
                }
                pos++;
                
                uint32_t meta_len = read_varlen(file);
                // Skip varlen bytes count (approximate)
                uint32_t temp = meta_len;
                do {
                    pos++;
                    temp >>= 7;
                } while (temp > 0);
                
                if (meta_type == 0x2F) { // End of Track
                    has_eot = true;
                }
                
                file.seekg(meta_len, std::ios::cur);
                pos += meta_len;
            }
            // ===== SYSEX EVENT =====
            else if (status == 0xF0 || status == 0xF7) {
                uint32_t sysex_len = read_varlen(file);
                uint32_t temp = sysex_len;
                do {
                    pos++;
                    temp >>= 7;
                } while (temp > 0);
                file.seekg(sysex_len, std::ios::cur);
                pos += sysex_len;
            }
            // ===== CHANNEL VOICE MESSAGES =====
            else {
                info.total_events++;
                
                // Note Off
                if (cmd == 0x80) {
                    uint8_t note, vel;
                    if (!file.read(reinterpret_cast<char*>(&note), 1) ||
                        !file.read(reinterpret_cast<char*>(&vel), 1)) {
                        break;
                    }
                    pos += 2;
                    
                    if (note > 127) {
                        info.is_valid = false;
                        info.error_msg = "Note out of range (0-127)";
                        break;
                    }
                    if (vel > 127) {
                        info.is_valid = false;
                        info.error_msg = "Velocity out of range (0-127)";
                        break;
                    }
                    info.note_off++;
                }
                // Note On
                else if (cmd == 0x90) {
                    uint8_t note, vel;
                    if (!file.read(reinterpret_cast<char*>(&note), 1) ||
                        !file.read(reinterpret_cast<char*>(&vel), 1)) {
                        break;
                    }
                    pos += 2;
                    
                    if (note > 127) {
                        info.is_valid = false;
                        info.error_msg = "Note out of range (0-127)";
                        break;
                    }
                    if (vel > 127) {
                        info.is_valid = false;
                        info.error_msg = "Velocity out of range (0-127)";
                        break;
                    }
                    
                    // Track note range (0 is valid)
                    if (note <= 127) {
                        if (note < info.min_note) info.min_note = note;
                        if (note > info.max_note) info.max_note = note;
                    }
                    
                    if (vel > 0) {
                        info.note_on++;
                    } else {
                        info.note_off++;
                    }
                }
                // Poly Aftertouch
                else if (cmd == 0xA0) {
                    uint8_t note, pressure;
                    if (!file.read(reinterpret_cast<char*>(&note), 1) ||
                        !file.read(reinterpret_cast<char*>(&pressure), 1)) {
                        break;
                    }
                    pos += 2;
                    
                    if (note > 127) {
                        info.is_valid = false;
                        info.error_msg = "Note out of range (0-127)";
                        break;
                    }
                    if (pressure > 127) {
                        info.is_valid = false;
                        info.error_msg = "Pressure out of range (0-127)";
                        break;
                    }
                }
                // Control Change
                else if (cmd == 0xB0) {
                    uint8_t controller, value;
                    if (!file.read(reinterpret_cast<char*>(&controller), 1) ||
                        !file.read(reinterpret_cast<char*>(&value), 1)) {
                        break;
                    }
                    pos += 2;
                    
                    if (controller > 127) {
                        info.is_valid = false;
                        info.error_msg = "Controller number out of range (0-127)";
                        break;
                    }
                    if (value > 127) {
                        info.is_valid = false;
                        info.error_msg = "Control value out of range (0-127)";
                        break;
                    }
                }
                // Program Change
                else if (cmd == 0xC0) {
                    uint8_t program;
                    if (!file.read(reinterpret_cast<char*>(&program), 1)) {
                        break;
                    }
                    pos++;
                    
                    if (program > 127) {
                        info.is_valid = false;
                        info.error_msg = "Program number out of range (0-127)";
                        break;
                    }
                }
                // Channel Aftertouch
                else if (cmd == 0xD0) {
                    uint8_t pressure;
                    if (!file.read(reinterpret_cast<char*>(&pressure), 1)) {
                        break;
                    }
                    pos++;
                    
                    if (pressure > 127) {
                        info.is_valid = false;
                        info.error_msg = "Pressure out of range (0-127)";
                        break;
                    }
                }
                // Pitch Bend
                else if (cmd == 0xE0) {
                    uint8_t lsb, msb;
                    if (!file.read(reinterpret_cast<char*>(&lsb), 1) ||
                        !file.read(reinterpret_cast<char*>(&msb), 1)) {
                        break;
                    }
                    pos += 2;
                    
                    uint16_t pitch_bend = (msb << 7) | lsb;
                    if (pitch_bend > 16383) {
                        info.is_valid = false;
                        info.error_msg = "Pitch bend out of range (0-16383)";
                        break;
                    }
                }
                else {
                    info.is_valid = false;
                    info.error_msg = "Unknown MIDI command";
                    break;
                }
            }
        }
        
        if (!info.is_valid) break;
        
        if (has_eot) info.tracks_with_eot++;
        file.seekg(track_start + static_cast<std::streamoff>(track_len), std::ios::beg);
    }
    
    file.close();
    
    if (info.is_valid) {
        if (info.tracks_found != info.num_tracks) {
            info.is_valid = false;
            info.error_msg = "Track count mismatch";
        }
        else if (info.tracks_with_eot != info.tracks_found) {
            info.is_valid = false;
            info.error_msg = "Missing EOT markers";
        }
    }
    
    return info;
}

void MidiAnalyzer::printReport(const std::string& filename, const MidiInfo& info) {
    std::cout << "\n+--------------------------------------------------+\n";
    std::cout << "| FILE HEADER                                      |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Filename:        " << filename << "\n";
    std::cout << "| Format:          " << info.format;
    if (info.format == 0) std::cout << " (single track)\n";
    else if (info.format == 1) std::cout << " (multi-track)\n";
    else if (info.format == 2) std::cout << " (multi-song)\n";
    else std::cout << " (unknown)\n";
    std::cout << "| Tracks declared: " << info.num_tracks << "\n";
    std::cout << "| Tracks found:    " << info.tracks_found << "\n";
    if (info.division & 0x8000) {
        std::cout << "| Division:        SMPTE format\n";
    } else {
        std::cout << "| Division:        " << info.division << " ticks per quarter note\n";
    }
    std::cout << "| File size:       " << info.file_size << " bytes\n";
    std::cout << "+--------------------------------------------------+\n";
    
    std::cout << "\n+--------------------------------------------------+\n";
    std::cout << "| TRACK INFORMATION                                |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Tracks with EOT: " << info.tracks_with_eot;
    if (info.tracks_with_eot != info.tracks_found && info.tracks_found > 0) {
        std::cout << " [WARNING: " << (info.tracks_found - info.tracks_with_eot) << " missing EOT]";
    }
    std::cout << "\n+--------------------------------------------------+\n";
    
    std::cout << "\n+--------------------------------------------------+\n";
    std::cout << "| EVENT STATISTICS                                 |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Total events:    " << info.total_events << "\n";
    std::cout << "| Note On events:  " << info.note_on << "\n";
    std::cout << "| Note Off events: " << info.note_off << "\n";
    if (info.note_on + info.note_off > 0) {
        std::cout << "| Note range:      " << (int)info.min_note << " - " << (int)info.max_note << "\n";
    }
    std::cout << "+--------------------------------------------------+\n";
    
    std::cout << "\n+--------------------------------------------------+\n";
    std::cout << "| INTEGRITY CHECKS                                 |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    if (!info.error_msg.empty()) {
        std::cout << "| [ERROR] " << info.error_msg << "\n";
    }
    
    if (info.is_valid) {
        std::cout << "| [OK] All integrity checks passed\n";
        std::cout << "+--------------------------------------------------+\n";
        std::cout << "\n+--------------------------------------------------+\n";
        std::cout << "| [OK] FILE IS VALID                                |\n";
        std::cout << "+--------------------------------------------------+\n";
    } else {
        if (info.tracks_with_eot != info.tracks_found && info.error_msg.empty()) {
            std::cout << "| [WARNING] " << (info.tracks_found - info.tracks_with_eot) 
                      << " track(s) missing End of Track marker\n";
        }
        std::cout << "+--------------------------------------------------+\n";
        std::cout << "\n+--------------------------------------------------+\n";
        std::cout << "| [ERROR] FILE IS INVALID                            |\n";
        std::cout << "+--------------------------------------------------+\n";
    }
}