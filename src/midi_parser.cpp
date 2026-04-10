#include "midi_parser.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <vector>
#include <map>
#include <cstring>

namespace midi {

MidiParser::MidiParser() {
    resetMidiFile();
}

MidiParser::~MidiParser() {
}

void MidiParser::resetMidiFile() {
    midiFile.isValid = false;
    midiFile.format = 0;
    midiFile.numTracks = 0;
    midiFile.division = 480;
    midiFile.isSMPTE = false;
    midiFile.smpteFrames = 0;
    midiFile.smpteResolution = 0;
    midiFile.ticksPerQuarterNote = 480;
    midiFile.fileSize = 0;
    midiFile.totalEvents = 0;
    midiFile.totalNotes = 0;
    midiFile.minNote = 127;
    midiFile.maxNote = 0;
    midiFile.minBPM = 1000;
    midiFile.maxBPM = 0;
    midiFile.durationSeconds = 0;
    midiFile.maxPolyphony = 0;
    midiFile.tracks.clear();
    midiFile.tempoEvents.clear();
    midiFile.timeSignatures.clear();
    midiFile.controllerEvents.clear();
}

bool MidiParser::loadFile(const std::string& filename) {
    this->filename = filename;
    resetMidiFile();
    
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        lastError = "Cannot open file: " + filename;
        return false;
    }
    
    file.seekg(0, std::ios::end);
    midiFile.fileSize = static_cast<uint32_t>(file.tellg());
    file.seekg(0, std::ios::beg);
    
    if (midiFile.fileSize < 14) {
        lastError = "File too small to be a valid MIDI file";
        file.close();
        return false;
    }
    
    // Read and validate header
    char magic[4];
    file.read(magic, 4);
    if (std::string(magic, 4) != "MThd") {
        lastError = "Invalid MIDI file: expected 'MThd'";
        file.close();
        return false;
    }
    
    if (!parseHeader(file)) {
        file.close();
        return false;
    }
    
    // Read all tracks
    midiFile.tracks.clear();
    
    // Scan for MTrk chunks
    file.seekg(14, std::ios::beg); // Start after header
    
    int trackCount = 0;
    while (!file.eof() && trackCount < 100) {
        // Save position
        std::streampos pos = file.tellg();
        
        // Peek at next 4 bytes
        char chunkType[5] = {0};
        file.read(chunkType, 4);
        
        if (file.gcount() < 4) {
            break;
        }
        
        if (std::string(chunkType, 4) == "MTrk") {
            // Found a track - rewind and parse
            file.seekg(pos);
            MidiTrack track;
            if (parseTrack(file, track, static_cast<uint32_t>(pos))) {
                midiFile.tracks.push_back(track);
                trackCount++;
            } else {
                // Skip this chunk - move past the track header
                file.seekg(pos + static_cast<std::streamoff>(8), std::ios::beg);
            }
        } else {
            // Not a track, move forward 1 byte and continue
            file.seekg(pos + static_cast<std::streamoff>(1), std::ios::beg);
        }
    }
    
    file.close();
    
    if (midiFile.tracks.empty()) {
        lastError = "No valid tracks found";
        return false;
    }
    
    // Post-processing
    calculateDuration();
    calculatePolyphony();
    
    // Determine if file is valid
    midiFile.isValid = true;
    
    // Check 1: All tracks must have End of Track marker
    for (const auto& track : midiFile.tracks) {
        if (!track.hasEndOfTrack) {
            midiFile.isValid = false;
            lastError = "Missing End of Track marker";
            break;
        }
    }
    
    // Check 2: Note range must be 0-127
    if (midiFile.minNote > 127 || midiFile.maxNote > 127) {
        midiFile.isValid = false;
        lastError = "Invalid note range";
    }
    
    // Check 3: Format must be 0, 1, or 2
    if (midiFile.format > 2) {
        midiFile.isValid = false;
        lastError = "Invalid format";
    }
    
    return true;
}

bool MidiParser::parseHeader(std::ifstream& file) {
    uint32_t headerSize = readUint32(file);
    if (headerSize != 6) {
        lastError = "Invalid header size: " + std::to_string(headerSize);
        return false;
    }
    
    midiFile.format = readUint16(file);
    midiFile.numTracks = readUint16(file);
    midiFile.division = readUint16(file);
    
    if (midiFile.format > 2) {
        lastError = "Unsupported MIDI format: " + std::to_string(midiFile.format);
        return false;
    }
    
    if (midiFile.division & 0x8000) {
        midiFile.isSMPTE = true;
        int8_t smpte = (midiFile.division >> 8) & 0x7F;
        if (smpte & 0x40) smpte = -(smpte & 0x3F);
        midiFile.smpteFrames = smpte;
        midiFile.smpteResolution = midiFile.division & 0xFF;
        midiFile.ticksPerQuarterNote = 480;
    } else {
        midiFile.isSMPTE = false;
        midiFile.ticksPerQuarterNote = midiFile.division;
    }
    
    return true;
}

bool MidiParser::parseTrack(std::ifstream& file, MidiTrack& track, uint32_t trackOffset) {
    track.fileOffset = trackOffset;
    
    // Read chunk type
    char chunkType[4];
    file.read(chunkType, 4);
    
    if (file.gcount() != 4 || std::string(chunkType, 4) != "MTrk") {
        track.errorMessage = "Invalid track chunk";
        track.isValid = false;
        return false;
    }
    
    track.length = readUint32(file);
    
    // Sanity check
    if (track.length == 0 || track.length > 100000000) {
        track.errorMessage = "Invalid track length: " + std::to_string(track.length);
        track.isValid = false;
        return false;
    }
    
    std::streampos trackStart = file.tellg();
    std::streampos trackEnd = trackStart + static_cast<std::streamoff>(track.length);
    
    // Check file bounds
    file.seekg(0, std::ios::end);
    std::streampos fileEnd = file.tellg();
    file.seekg(trackStart);
    
    if (trackEnd > fileEnd) {
        trackEnd = fileEnd;
    }
    
    uint8_t runningStatus = 0;
    track.events.clear();
    track.hasEndOfTrack = false;
    track.trackName = "";
    track.instrumentName = "";
    
    // Temporary storage for building tempo events with correct ticks
    uint32_t currentTick = 0;
    
    while (file.tellg() < trackEnd && !file.eof()) {
        uint32_t eventOffset = static_cast<uint32_t>(file.tellg());
        MidiEvent event;
        
        if (!parseEvent(file, event, runningStatus, eventOffset)) {
            if (track.events.empty()) {
                track.errorMessage = event.errorMessage;
                track.isValid = false;
                return false;
            }
            break;
        }
        
        currentTick += event.deltaTime;
        
        // Check for End of Track
        if (event.type == EventType::MetaEvent && !event.data.empty() && event.data[0] == 0x2F) {
            track.hasEndOfTrack = true;
        }
        
        // Extract metadata
        if (event.type == EventType::MetaEvent && !event.data.empty()) {
            uint8_t metaType = event.data[0];
            if (metaType == 0x03 && event.data.size() > 1) {
                track.trackName = std::string(event.data.begin() + 1, event.data.end());
            } else if (metaType == 0x04 && event.data.size() > 1) {
                track.instrumentName = std::string(event.data.begin() + 1, event.data.end());
            } else if (metaType == 0x51 && event.data.size() >= 4) {
                TempoEvent tempo;
                tempo.tick = currentTick;
                tempo.microsecondsPerQuarterNote = (event.data[1] << 16) | (event.data[2] << 8) | event.data[3];
                if (tempo.microsecondsPerQuarterNote > 0) {
                    tempo.bpm = 60000000.0 / tempo.microsecondsPerQuarterNote;
                    if (tempo.bpm < midiFile.minBPM) midiFile.minBPM = tempo.bpm;
                    if (tempo.bpm > midiFile.maxBPM) midiFile.maxBPM = tempo.bpm;
                } else {
                    tempo.bpm = 120.0;
                }
                midiFile.tempoEvents.push_back(tempo);
            } else if (metaType == 0x58 && event.data.size() >= 5) {
                TimeSignatureEvent ts;
                ts.tick = currentTick;
                ts.numerator = event.data[1];
                ts.denominator = 1 << event.data[2];
                ts.clocksPerMetronomeTick = event.data[3];
                ts.thirtySecondNotesPerQuarter = event.data[4];
                midiFile.timeSignatures.push_back(ts);
            }
        }
        
        // Count notes and validate range
        if (event.type == EventType::NoteOn && event.data.size() >= 2) {
            if (event.data[1] > 0) { // Real Note On (velocity > 0)
                uint8_t note = event.data[0];
                // Note range check: 0-127 is valid according to MIDI spec
                if (note <= 127) {
                    midiFile.totalNotes++;
                    if (note < midiFile.minNote) midiFile.minNote = note;
                    if (note > midiFile.maxNote) midiFile.maxNote = note;
                } else {
                    // Invalid note - mark file as invalid
                    midiFile.isValid = false;
                    lastError = "Note out of range: " + std::to_string(note);
                }
            }
        }
        
        track.events.push_back(event);
        midiFile.totalEvents++;
    }
    
    track.isValid = true;
    return true;
}

bool MidiParser::parseEvent(std::ifstream& file, MidiEvent& event, uint8_t& runningStatus, uint32_t fileOffset) {
    event.fileOffset = fileOffset;
    event.isValid = true;
    event.isRunningStatus = false;
    
    if (file.eof()) {
        event.isValid = false;
        event.errorMessage = "Unexpected end of file";
        return false;
    }
    
    size_t vlqBytes = 0;
    event.deltaTime = readVariableLength(file, vlqBytes);
    
    if (vlqBytes == 0) {
        event.isValid = false;
        event.errorMessage = "Invalid VLQ encoding";
        return false;
    }
    
    if (file.eof()) {
        event.isValid = false;
        event.errorMessage = "Unexpected end of file after delta time";
        return false;
    }
    
    uint8_t statusByte;
    file.read(reinterpret_cast<char*>(&statusByte), 1);
    
    // Running status handling - FIXED
    if ((statusByte & 0x80) == 0) {
        // This byte is actually data, not status (running status case)
        if (runningStatus == 0) {
            event.isValid = false;
            event.errorMessage = "Invalid running status (no previous status)";
            return false;
        }
        // Put back the data byte, we'll read it after setting the status
        file.seekg(-1, std::ios::cur);
        statusByte = runningStatus;
        event.isRunningStatus = true;
    } else {
        runningStatus = statusByte;
    }
    
    event.type = static_cast<EventType>(statusByte & 0xF0);
    event.channel = statusByte & 0x0F;
    
    switch (event.type) {
        case EventType::NoteOff:
        case EventType::NoteOn:
        case EventType::PolyPressure:
        case EventType::ControlChange:
        case EventType::PitchBend: {
            uint8_t data1 = 0, data2 = 0;
            if (!file.read(reinterpret_cast<char*>(&data1), 1)) {
                event.isValid = false;
                event.errorMessage = "Unexpected end of file reading data1";
                return false;
            }
            file.read(reinterpret_cast<char*>(&data2), 1);
            if (file.gcount() < 1) {
                event.data = {data1, 0};
            } else {
                event.data = {data1, data2};
            }
            break;
        }
        
        case EventType::ProgramChange:
        case EventType::ChannelPressure: {
            uint8_t data1;
            if (!file.read(reinterpret_cast<char*>(&data1), 1)) {
                event.isValid = false;
                event.errorMessage = "Unexpected end of file reading data1";
                return false;
            }
            event.data = {data1};
            break;
        }
        
        case EventType::MetaEvent: {
            event.type = EventType::MetaEvent;
            uint8_t metaType;
            if (!file.read(reinterpret_cast<char*>(&metaType), 1)) {
                event.isValid = false;
                event.errorMessage = "Unexpected end of file reading meta type";
                return false;
            }
            event.data.push_back(metaType);
            
            size_t lenBytes = 0;
            uint32_t length = readVariableLength(file, lenBytes);
            
            if (lenBytes == 0) {
                event.isValid = false;
                event.errorMessage = "Invalid meta event length encoding";
                return false;
            }
            
            if (length > 1000000) {
                event.isValid = false;
                event.errorMessage = "Meta event too large: " + std::to_string(length);
                return false;
            }
            
            for (uint32_t i = 0; i < length; ++i) {
                uint8_t byte;
                if (!file.read(reinterpret_cast<char*>(&byte), 1)) {
                    event.isValid = false;
                    event.errorMessage = "Unexpected end of file reading meta data";
                    return false;
                }
                event.data.push_back(byte);
            }
            break;
        }
        
        case EventType::SysEx:
        case EventType::SysExEnd: {
            size_t lenBytes = 0;
            uint32_t length = readVariableLength(file, lenBytes);
            
            if (lenBytes == 0) {
                event.isValid = false;
                event.errorMessage = "Invalid SysEx length encoding";
                return false;
            }
            
            if (length > 1000000) {
                event.isValid = false;
                event.errorMessage = "SysEx too large: " + std::to_string(length);
                return false;
            }
            
            for (uint32_t i = 0; i < length; ++i) {
                uint8_t byte;
                if (!file.read(reinterpret_cast<char*>(&byte), 1)) {
                    event.isValid = false;
                    event.errorMessage = "Unexpected end of file reading SysEx data";
                    return false;
                }
                event.data.push_back(byte);
            }
            break;
        }
        
        default:
            event.isValid = false;
            event.errorMessage = "Unknown event type: 0x" + 
                std::to_string(static_cast<int>(event.type));
            return false;
    }
    
    return true;
}

void MidiParser::calculateDuration() {
    if (midiFile.isSMPTE || midiFile.ticksPerQuarterNote == 0) {
        midiFile.durationSeconds = 0;
        return;
    }
    
    // Find the longest track in ticks
    uint32_t totalTicks = 0;
    for (const auto& track : midiFile.tracks) {
        uint32_t trackTicks = 0;
        for (const auto& event : track.events) {
            trackTicks += event.deltaTime;
        }
        if (trackTicks > totalTicks) totalTicks = trackTicks;
    }
    
    // Build tempo map (tick -> BPM)
    std::map<uint32_t, double> tempoMap;
    tempoMap[0] = 120.0; // default tempo
    
    for (const auto& tempo : midiFile.tempoEvents) {
        tempoMap[tempo.tick] = tempo.bpm;
    }
    
    // Calculate duration with tempo changes
    double totalSeconds = 0.0;
    uint32_t lastTick = 0;
    double currentBPM = 120.0;
    
    for (const auto& pair : tempoMap) {
        uint32_t tickPos = pair.first;
        double bpm = pair.second;
        
        if (tickPos > lastTick) {
            double secondsPerTick = 60.0 / currentBPM / midiFile.ticksPerQuarterNote;
            totalSeconds += (tickPos - lastTick) * secondsPerTick;
        }
        
        lastTick = tickPos;
        currentBPM = bpm;
    }
    
    // Add remaining time from last tempo change to end
    if (totalTicks > lastTick) {
        double secondsPerTick = 60.0 / currentBPM / midiFile.ticksPerQuarterNote;
        totalSeconds += (totalTicks - lastTick) * secondsPerTick;
    }
    
    midiFile.durationSeconds = totalSeconds;
}

void MidiParser::calculatePolyphony() {
    std::map<int, int> activeNotes;
    int maxActive = 0;
    
    for (const auto& track : midiFile.tracks) {
        for (const auto& event : track.events) {
            if (event.type == EventType::NoteOn && event.data.size() >= 2 && event.data[1] > 0) {
                int key = event.channel * 128 + event.data[0];
                activeNotes[key]++;
                if (static_cast<int>(activeNotes.size()) > maxActive) {
                    maxActive = static_cast<int>(activeNotes.size());
                }
            } else if ((event.type == EventType::NoteOff) ||
                       (event.type == EventType::NoteOn && event.data.size() >= 2 && event.data[1] == 0)) {
                int key = event.channel * 128 + event.data[0];
                auto it = activeNotes.find(key);
                if (it != activeNotes.end()) {
                    if (it->second > 1) {
                        it->second--;
                    } else {
                        activeNotes.erase(it);
                    }
                }
            }
        }
    }
    midiFile.maxPolyphony = maxActive;
}

uint16_t MidiParser::readUint16(std::ifstream& file) {
    uint16_t value;
    file.read(reinterpret_cast<char*>(&value), 2);
    return (value >> 8) | ((value & 0xFF) << 8);
}

uint32_t MidiParser::readUint32(std::ifstream& file) {
    uint32_t value;
    file.read(reinterpret_cast<char*>(&value), 4);
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0xFF000000) >> 24);
}

uint32_t MidiParser::readVariableLength(std::ifstream& file, size_t& bytesRead) {
    uint32_t value = 0;
    uint8_t byte;
    bytesRead = 0;
    
    while (bytesRead < 4) {
        if (!file.read(reinterpret_cast<char*>(&byte), 1)) {
            bytesRead = 0;
            return 0;
        }
        bytesRead++;
        
        value = (value << 7) | (byte & 0x7F);
        
        if ((byte & 0x80) == 0) {
            return value;
        }
    }
    
    bytesRead = 0;
    return 0;
}

std::string MidiParser::eventTypeToString(EventType type) const {
    switch (type) {
        case EventType::NoteOff: return "Note Off";
        case EventType::NoteOn: return "Note On";
        case EventType::PolyPressure: return "Poly Pressure";
        case EventType::ControlChange: return "Control Change";
        case EventType::ProgramChange: return "Program Change";
        case EventType::ChannelPressure: return "Channel Pressure";
        case EventType::PitchBend: return "Pitch Bend";
        case EventType::MetaEvent: return "Meta Event";
        case EventType::SysEx: return "System Exclusive";
        case EventType::SysExEnd: return "System Exclusive End";
        default: return "Unknown";
    }
}

std::string MidiParser::metaEventToString(uint8_t metaType) const {
    switch (metaType) {
        case 0x00: return "Sequence Number";
        case 0x01: return "Text";
        case 0x02: return "Copyright";
        case 0x03: return "Track Name";
        case 0x04: return "Instrument Name";
        case 0x05: return "Lyric";
        case 0x06: return "Marker";
        case 0x07: return "Cue Point";
        case 0x20: return "MIDI Channel Prefix";
        case 0x2F: return "End of Track";
        case 0x51: return "Set Tempo";
        case 0x54: return "SMPTE Offset";
        case 0x58: return "Time Signature";
        case 0x59: return "Key Signature";
        case 0x7F: return "Sequencer Specific";
        default: return "Unknown";
    }
}

std::string MidiParser::toJSON() const {
    std::stringstream json;
    json << "{\n";
    json << "  \"file\": \"" << filename << "\",\n";
    json << "  \"size\": " << midiFile.fileSize << ",\n";
    json << "  \"format\": " << midiFile.format << ",\n";
    json << "  \"tracks\": " << midiFile.tracks.size() << ",\n";
    json << "  \"division\": " << midiFile.ticksPerQuarterNote << ",\n";
    json << "  \"totalEvents\": " << midiFile.totalEvents << ",\n";
    json << "  \"totalNotes\": " << midiFile.totalNotes << ",\n";
    json << "  \"durationSeconds\": " << std::fixed << std::setprecision(2) << midiFile.durationSeconds << ",\n";
    json << "  \"maxPolyphony\": " << midiFile.maxPolyphony << ",\n";
    json << "  \"valid\": " << (midiFile.isValid ? "true" : "false") << "\n";
    json << "}\n";
    return json.str();
}

void MidiParser::printAnalysis() const {
    if (!midiFile.isValid && midiFile.tracks.empty()) {
        std::cout << "MIDI file not loaded or invalid.\n";
        if (!lastError.empty()) std::cout << "Error: " << lastError << "\n";
        return;
    }
    
    std::cout << "\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "|                 MIDI FILE ANALYSIS               |\n";
    std::cout << "+--------------------------------------------------+\n\n";
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| FILE HEADER                                      |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Format:          ";
    if (midiFile.format == 0) std::cout << "0 (single track)\n";
    else if (midiFile.format == 1) std::cout << "1 (multi-track, synchronous)\n";
    else if (midiFile.format == 2) std::cout << "2 (multi-track, asynchronous)\n";
    else std::cout << "Unknown\n";
    
    std::cout << "| Tracks declared: " << std::setw(34) << midiFile.numTracks << "\n";
    std::cout << "| Tracks found:    " << std::setw(34) << midiFile.tracks.size();
    if (midiFile.tracks.size() != midiFile.numTracks && midiFile.numTracks > 0) {
        std::cout << " [WARNING: MISMATCH!]";
    }
    std::cout << "\n";
    
    std::cout << "| Division:        ";
    if (midiFile.isSMPTE) {
        std::cout << "SMPTE " << std::abs(midiFile.smpteFrames) 
                  << " fps, " << (int)midiFile.smpteResolution << " subframes\n";
    } else {
        std::cout << midiFile.ticksPerQuarterNote << " ticks per quarter note\n";
    }
    std::cout << "| File size:       " << std::setw(34) << midiFile.fileSize << " bytes\n";
    std::cout << "+--------------------------------------------------+\n\n";
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| TRACK INFORMATION                                |\n";
    std::cout << "+--------------------------------------------------+\n";
    if (midiFile.tracks.empty()) {
        std::cout << "| No tracks found!                                 |\n";
    }
    for (size_t i = 0; i < midiFile.tracks.size(); i++) {
        const auto& track = midiFile.tracks[i];
        std::cout << "| Track " << std::setw(2) << (i + 1) << ": ";
        std::cout << std::setw(5) << track.events.size() << " events, ";
        std::cout << std::setw(5) << track.length << " bytes";
        if (!track.hasEndOfTrack) std::cout << " [WARNING: No EOT]";
        if (!track.trackName.empty()) std::cout << " \"" << track.trackName << "\"";
        std::cout << "\n";
    }
    std::cout << "+--------------------------------------------------+\n\n";
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| EVENT STATISTICS                                 |\n";
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| Total events:    " << std::setw(34) << midiFile.totalEvents << "\n";
    std::cout << "| Note On events:  " << std::setw(34) << midiFile.totalNotes << "\n";
    if (midiFile.totalNotes > 0) {
        std::cout << "| Note range:      " << std::setw(34);
        std::cout << (int)midiFile.minNote << " - " << (int)midiFile.maxNote << "\n";
    }
    if (midiFile.minBPM < 1000 && midiFile.maxBPM > 0) {
        std::cout << "| Tempo range:     " << std::setw(34);
        std::cout << std::fixed << std::setprecision(1) << midiFile.minBPM 
                  << " - " << midiFile.maxBPM << " BPM\n";
    }
    std::cout << "| Duration:        " << std::setw(34);
    int minutes = static_cast<int>(midiFile.durationSeconds) / 60;
    int seconds = static_cast<int>(midiFile.durationSeconds) % 60;
    std::cout << minutes << "m " << seconds << "s (" << std::fixed << std::setprecision(2) << midiFile.durationSeconds << " sec)\n";
    std::cout << "| Max polyphony:   " << std::setw(34) << midiFile.maxPolyphony << " simultaneous notes\n";
    std::cout << "+--------------------------------------------------+\n\n";
    
    std::cout << "+--------------------------------------------------+\n";
    std::cout << "| INTEGRITY CHECKS                                 |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    int tracksWithoutEOT = 0;
    for (const auto& track : midiFile.tracks) {
        if (!track.hasEndOfTrack) tracksWithoutEOT++;
    }
    if (tracksWithoutEOT > 0) {
        std::cout << "| [WARNING] " << tracksWithoutEOT << " track(s) missing End of Track marker\n";
    } else {
        std::cout << "| [OK] All tracks have End of Track markers\n";
    }
    
    // Check note range validity
    if (midiFile.minNote < 0 || midiFile.maxNote > 127) {
        std::cout << "| [ERROR] Notes out of valid range (0-127)\n";
    } else if (midiFile.totalNotes > 0) {
        std::cout << "| [OK] All notes within valid range (0-127)\n";
    }
    
    std::cout << "+--------------------------------------------------+\n";
    
    std::cout << "\n+--------------------------------------------------+\n";
    std::cout << (midiFile.isValid ? "| [OK] FILE IS VALID                                |" : "| [ERROR] FILE IS INVALID                            |") << "\n";
    std::cout << "+--------------------------------------------------+\n";
}

void MidiParser::printDetailed(bool showOffsets) const {
    if (!midiFile.isValid && midiFile.tracks.empty()) {
        std::cout << "MIDI file not loaded or invalid.\nLast error: " << lastError << "\n";
        return;
    }
    
    std::cout << "\n=== MIDI File Analysis ===\n";
    std::cout << "File: " << filename << "\n";
    std::cout << "Size: " << midiFile.fileSize << " bytes\n";
    std::cout << "Format: " << midiFile.format << "\n";
    std::cout << "Tracks: " << midiFile.tracks.size() << "\n";
    std::cout << "Duration: " << static_cast<int>(midiFile.durationSeconds) / 60 << "m " 
              << static_cast<int>(midiFile.durationSeconds) % 60 << "s\n\n";
    
    for (size_t t = 0; t < midiFile.tracks.size(); ++t) {
        const auto& track = midiFile.tracks[t];
        std::cout << "--- Track " << (t + 1);
        if (!track.trackName.empty()) std::cout << ": " << track.trackName;
        std::cout << " (" << track.events.size() << " events) ---\n";
        
        uint32_t cumulativeTime = 0;
        size_t count = 0;
        size_t totalEvents = track.events.size();
        
        for (const auto& event : track.events) {
            cumulativeTime += event.deltaTime;
            count++;
            
            if (count <= 20 || count > totalEvents - 20) {
                if (showOffsets) {
                    std::cout << std::setw(8) << event.fileOffset << " ";
                }
                std::cout << std::setw(6) << cumulativeTime << ": ";
                switch (event.type) {
                    case EventType::NoteOn:
                        if (event.data.size() >= 2) {
                            if (event.data[1] == 0) {
                                std::cout << "Note Off ch=" << (int)event.channel 
                                          << " note=" << (int)event.data[0];
                            } else {
                                std::cout << "Note On  ch=" << (int)event.channel 
                                          << " note=" << (int)event.data[0] 
                                          << " vel=" << (int)event.data[1];
                            }
                        }
                        break;
                    case EventType::NoteOff:
                        std::cout << "Note Off ch=" << (int)event.channel 
                                  << " note=" << (int)event.data[0];
                        break;
                    case EventType::ControlChange:
                        std::cout << "Control  ch=" << (int)event.channel 
                                  << " ctrl=" << (int)event.data[0] 
                                  << " val=" << (int)event.data[1];
                        break;
                    case EventType::ProgramChange:
                        std::cout << "Program  ch=" << (int)event.channel 
                                  << " prog=" << (int)event.data[0];
                        break;
                    case EventType::MetaEvent:
                        if (!event.data.empty()) {
                            std::cout << "Meta: " << metaEventToString(event.data[0]);
                        }
                        break;
                    default:
                        std::cout << "Event type " << (int)event.type;
                }
                if (!event.isValid) std::cout << " [INVALID]";
                std::cout << "\n";
            } else if (count == 21 && totalEvents > 40) {
                std::cout << "  ... (" << (totalEvents - 40) << " more events) ...\n";
            }
        }
        if (!track.hasEndOfTrack) {
            std::cout << "  [WARNING] Missing End of Track\n";
        }
    }
    std::cout << "\n--- End of file ---\n";
}

} // namespace midi