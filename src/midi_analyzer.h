#ifndef MIDI_ANALYZER_H
#define MIDI_ANALYZER_H

#include <string>
#include <cstdint>

struct MidiInfo {
    bool is_valid = true;
    std::string error_msg;
    
    // Header info
    uint16_t format = 0;
    uint16_t num_tracks = 0;
    uint16_t division = 0;
    uint32_t file_size = 0;
    
    // Track info
    int tracks_found = 0;
    int tracks_with_eot = 0;
    
    // Event stats
    uint32_t total_events = 0;
    uint32_t note_on = 0;
    uint32_t note_off = 0;
    
    // Note range (0-127, both inclusive)
    uint8_t min_note = 127;
    uint8_t max_note = 0;
};

class MidiAnalyzer {
public:
    MidiInfo analyze(const std::string& filename);
    void printReport(const std::string& filename, const MidiInfo& info);
    
private:
    uint16_t read_be16(const uint8_t* p);
    uint32_t read_be32(const uint8_t* p);
    uint32_t read_varlen(std::ifstream& file);
};

#endif