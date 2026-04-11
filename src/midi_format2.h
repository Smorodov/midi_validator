#ifndef MIDI_FORMAT2_H
#define MIDI_FORMAT2_H

#include "midi_types.h"
#include <vector>

namespace midi {

// Song для Format 2 (независимая последовательность треков)
struct Song {
    uint16_t songNumber;
    std::vector<TrackData> tracks;
    uint32_t totalTicks;
    double durationSeconds;
    bool hasEndOfSong;
};

// Расширенный анализ для Format 2
class Format2Analyzer {
public:
    Format2Analyzer();
    
    // Разделяет треки на песни (каждая песня начинается с Sequence Number)
    std::vector<Song> splitIntoSongs(const MidiFileData& fileData);
    
    // Анализирует каждую песню отдельно
    std::vector<AnalysisResult> analyzeSongs(const MidiFileData& fileData);
    
private:
    bool isSequenceNumberTrack(const TrackData& track);
    uint16_t getSequenceNumber(const TrackData& track);
};

} // namespace midi

#endif // MIDI_FORMAT2_H