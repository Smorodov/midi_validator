#include "midi_format2.h"
#include "midi_analyzer.h"
#include <algorithm>

namespace midi {

Format2Analyzer::Format2Analyzer() {
}

bool Format2Analyzer::isSequenceNumberTrack(const TrackData& track) {
    for (const auto& event : track.events) {
        if (event.type == EventType::MetaEvent && !event.data.empty()) {
            if (event.data[0] == MetaEvent::SEQUENCE_NUMBER) {
                return true;
            }
        }
    }
    return false;
}

uint16_t Format2Analyzer::getSequenceNumber(const TrackData& track) {
    for (const auto& event : track.events) {
        if (event.type == EventType::MetaEvent && !event.data.empty()) {
            if (event.data[0] == MetaEvent::SEQUENCE_NUMBER && event.data.size() >= 3) {
                return (event.data[1] << 8) | event.data[2];
            }
        }
    }
    return 0xFFFF;
}

std::vector<Song> Format2Analyzer::splitIntoSongs(const MidiFileData& fileData) {
    std::vector<Song> songs;
    Song currentSong;
    bool inSong = false;
    
    for (const auto& track : fileData.tracks) {
        if (isSequenceNumberTrack(track)) {
            // Начинаем новую песню
            if (inSong) {
                songs.push_back(currentSong);
            }
            currentSong = Song();
            currentSong.songNumber = getSequenceNumber(track);
            currentSong.hasEndOfSong = false;
            inSong = true;
        } else if (inSong) {
            currentSong.tracks.push_back(track);
            
            // Проверяем наличие End of Song (обычно последний трек)
            for (const auto& event : track.events) {
                if (event.type == EventType::MetaEvent && !event.data.empty()) {
                    if (event.data[0] == MetaEvent::END_OF_TRACK) {
                        // В Format 2, EOT в последнем треке означает конец песни
                        if (currentSong.tracks.size() >= 1) {
                            currentSong.hasEndOfSong = true;
                        }
                    }
                }
            }
        }
    }
    
    if (inSong) {
        songs.push_back(currentSong);
    }
    
    return songs;
}

std::vector<AnalysisResult> Format2Analyzer::analyzeSongs(const MidiFileData& fileData) {
    std::vector<AnalysisResult> results;
    auto songs = splitIntoSongs(fileData);
    
    MidiAnalyzer analyzer;
    
    for (const auto& song : songs) {
        // Создаем временную структуру для анализа песни
        MidiFileData songData;
        songData.format = 2;
        songData.numTracks = static_cast<uint16_t>(song.tracks.size());
        songData.division = fileData.division;
        songData.isSMPTE = fileData.isSMPTE;
        songData.smpteFrames = fileData.smpteFrames;
        songData.smpteResolution = fileData.smpteResolution;
        songData.tracks = song.tracks;
        songData.fileSize = fileData.fileSize; // Приблизительно
        
        auto result = analyzer.analyze(songData);
        result.rawData.songNumber = song.songNumber; // Добавляем номер песни
        results.push_back(result);
    }
    
    return results;
}

} // namespace midi