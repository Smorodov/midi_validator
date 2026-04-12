#ifndef MIDI_BUILDER_HPP
#define MIDI_BUILDER_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include <fstream>
#include <algorithm>

namespace midi {

//=============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
//=============================================================================

inline std::vector<uint8_t> writeVLQ(uint32_t value) {
    std::vector<uint8_t> result;
    if (value == 0) {
        result.push_back(0);
        return result;
    }
    std::vector<uint8_t> temp;
    while (value > 0) {
        uint8_t byte = value & 0x7F;
        value >>= 7;
        if (!temp.empty()) byte |= 0x80;
        temp.push_back(byte);
    }
    for (auto it = temp.rbegin(); it != temp.rend(); ++it) {
        result.push_back(*it);
    }
    return result;
}

inline uint16_t readUint16(const uint8_t* data) {
    return (data[0] << 8) | data[1];
}

inline void writeUint16(std::vector<uint8_t>& data, uint16_t value) {
    data.push_back((value >> 8) & 0xFF);
    data.push_back(value & 0xFF);
}

inline void writeUint32(std::vector<uint8_t>& data, uint32_t value) {
    data.push_back((value >> 24) & 0xFF);
    data.push_back((value >> 16) & 0xFF);
    data.push_back((value >> 8) & 0xFF);
    data.push_back(value & 0xFF);
}

//=============================================================================
// ТИПЫ ДАННЫХ
//=============================================================================

struct Note {
    uint8_t pitch = 60;        // 0-127, 60 = C4
    uint8_t velocity = 100;     // 0-127
    uint32_t startTick = 0;     // начало в тиках
    uint32_t duration = 480;    // длительность в тиках
    uint8_t channel = 0;        // 0-15
};

struct TempoChange {
    uint32_t tick = 0;
    uint32_t bpm = 120;         // ударов в минуту (40-300)
};

struct TimeSignature {
    uint32_t tick = 0;
    uint8_t numerator = 4;      // числитель
    uint8_t denominator = 4;    // знаменатель (1,2,4,8,16)
    uint8_t clocksPerTick = 24; // 24 по умолчанию
    uint8_t notesPerQuarter = 8; // 8 по умолчанию
};

struct KeySignature {
    uint32_t tick = 0;
    int8_t sharpsFlats = 0;     // -7..7 (отрицательные = бемоли)
    bool isMinor = false;       // false = мажор, true = минор
};

struct TextEvent {
    uint32_t tick = 0;
    std::string text;
    enum Type { TEXT, COPYRIGHT, TRACK_NAME, INSTRUMENT_NAME, LYRIC, MARKER, CUE_POINT };
    Type type = TRACK_NAME;
    
    uint8_t getMetaType() const {
        switch (type) {
            case TEXT: return 0x01;
            case COPYRIGHT: return 0x02;
            case TRACK_NAME: return 0x03;
            case INSTRUMENT_NAME: return 0x04;
            case LYRIC: return 0x05;
            case MARKER: return 0x06;
            case CUE_POINT: return 0x07;
            default: return 0x03;
        }
    }
};

struct ControlChange {
    uint32_t tick = 0;
    uint8_t controller = 7;     // 0-127, 7 = volume
    uint8_t value = 100;        // 0-127
    uint8_t channel = 0;
};

struct ProgramChange {
    uint32_t tick = 0;
    uint8_t program = 0;        // 0-127, 0 = Acoustic Grand Piano
    uint8_t channel = 0;
};

struct PitchBend {
    uint32_t tick = 0;
    uint16_t value = 8192;      // 0-16383, 8192 = центр
    uint8_t channel = 0;
};

struct Sysex {
    uint32_t tick = 0;
    std::vector<uint8_t> data;  // данные без F0 и F7
};

//=============================================================================
// ОСНОВНОЙ КЛАСС ПОСТРОИТЕЛЯ MIDI (HEADER-ONLY)
//=============================================================================

class MidiBuilder {
public:
    MidiBuilder() : m_format(0), m_division(480), m_initialTempo(120) {
        clear();
    }
    
    void setFormat(uint16_t format) { m_format = format; }
    void setDivision(uint16_t ticksPerQuarter) { m_division = ticksPerQuarter; }
    void setTempo(uint32_t bpm) { m_initialTempo = bpm; }
    
    void addNote(const Note& note) {
        m_notes.push_back(note);
    }
    
    void addChord(uint32_t tick, const std::vector<uint8_t>& notes, 
                  uint8_t velocity, uint32_t duration, uint8_t channel = 0) {
        for (uint8_t pitch : notes) {
            Note n;
            n.pitch = pitch;
            n.velocity = velocity;
            n.startTick = tick;
            n.duration = duration;
            n.channel = channel;
            m_notes.push_back(n);
        }
    }
    
    void addArpeggio(uint32_t tick, const std::vector<uint8_t>& notes,
                     uint8_t velocity, uint32_t duration, uint32_t patternLength,
                     uint8_t channel = 0) {
        uint32_t step = patternLength / notes.size();
        for (size_t i = 0; i < notes.size(); i++) {
            Note n;
            n.pitch = notes[i];
            n.velocity = velocity;
            n.startTick = tick + i * step;
            n.duration = duration;
            n.channel = channel;
            m_notes.push_back(n);
        }
    }
    
    void addTempoChange(const TempoChange& tempo) {
        m_tempoChanges.push_back(tempo);
    }
    
    void addTimeSignature(const TimeSignature& ts) {
        m_timeSignatures.push_back(ts);
    }
    
    void addKeySignature(const KeySignature& ks) {
        m_keySignatures.push_back(ks);
    }
    
    void addText(const TextEvent& text) {
        m_textEvents.push_back(text);
    }
    
    void addControlChange(const ControlChange& cc) {
        m_controlChanges.push_back(cc);
    }
    
    void addProgramChange(const ProgramChange& pc) {
        m_programChanges.push_back(pc);
    }
    
    void addPitchBend(const PitchBend& pb) {
        m_pitchBends.push_back(pb);
    }
    
    void addSysex(const Sysex& sysex) {
        m_sysex.push_back(sysex);
    }
    
    void clear() {
        m_notes.clear();
        m_tempoChanges.clear();
        m_timeSignatures.clear();
        m_keySignatures.clear();
        m_textEvents.clear();
        m_controlChanges.clear();
        m_programChanges.clear();
        m_pitchBends.clear();
        m_sysex.clear();
        m_initialTempo = 120;
        m_format = 0;
        m_division = 480;
    }
    
    bool save(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        auto data = getData();
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
        return true;
    }
    
    std::vector<uint8_t> getData() {
        std::vector<uint8_t> result;
        
        // Заголовок
        result.push_back('M'); result.push_back('T'); result.push_back('h'); result.push_back('d');
        writeUint32(result, 6);
        writeUint16(result, m_format);
        
        // Количество треков (для format 0 всегда 1)
        uint16_t numTracks = (m_format == 0) ? 1 : 1;  // упрощенно
        writeUint16(result, numTracks);
        writeUint16(result, m_division);
        
        // Создаем один трек со всеми событиями
        result.push_back('M'); result.push_back('T'); result.push_back('r'); result.push_back('k');
        size_t trackLenPos = result.size();
        writeUint32(result, 0);
        size_t trackStart = result.size();
        
        // Сортируем события по времени
        std::vector<std::pair<uint32_t, std::vector<uint8_t>>> events;
        
        // Добавляем начальный темп
        if (m_initialTempo != 120) {
            TempoChange initTempo;
            initTempo.tick = 0;
            initTempo.bpm = m_initialTempo;
            m_tempoChanges.push_back(initTempo);
        }
        
        // Генерируем события
        generateEvents(events);
        
        // Сортируем
        std::sort(events.begin(), events.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        
        // Записываем события
        uint32_t lastTick = 0;
        for (const auto& ev : events) {
            uint32_t delta = ev.first - lastTick;
            auto deltaBytes = writeVLQ(delta);
            result.insert(result.end(), deltaBytes.begin(), deltaBytes.end());
            result.insert(result.end(), ev.second.begin(), ev.second.end());
            lastTick = ev.first;
        }
        
        // End of Track
        auto eotDelta = writeVLQ(0);
        result.insert(result.end(), eotDelta.begin(), eotDelta.end());
        result.push_back(0xFF);
        result.push_back(0x2F);
        result.push_back(0x00);
        
        // Записываем длину трека
        uint32_t trackLen = static_cast<uint32_t>(result.size() - trackStart);
        result[trackLenPos] = (trackLen >> 24) & 0xFF;
        result[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
        result[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
        result[trackLenPos + 3] = trackLen & 0xFF;
        
        return result;
    }
    
private:
    void generateEvents(std::vector<std::pair<uint32_t, std::vector<uint8_t>>>& events) {
        // Ноты
        for (const auto& note : m_notes) {
            // Note On
            std::vector<uint8_t> on;
            on.push_back(0x90 | (note.channel & 0x0F));
            on.push_back(note.pitch);
            on.push_back(note.velocity);
            events.push_back({note.startTick, on});
            
            // Note Off
            std::vector<uint8_t> off;
            off.push_back(0x80 | (note.channel & 0x0F));
            off.push_back(note.pitch);
            off.push_back(0);
            events.push_back({note.startTick + note.duration, off});
        }
        
        // Контроллеры
        for (const auto& cc : m_controlChanges) {
            std::vector<uint8_t> data;
            data.push_back(0xB0 | (cc.channel & 0x0F));
            data.push_back(cc.controller);
            data.push_back(cc.value);
            events.push_back({cc.tick, data});
        }
        
        // Program Change
        for (const auto& pc : m_programChanges) {
            std::vector<uint8_t> data;
            data.push_back(0xC0 | (pc.channel & 0x0F));
            data.push_back(pc.program);
            events.push_back({pc.tick, data});
        }
        
        // Pitch Bend
        for (const auto& pb : m_pitchBends) {
            std::vector<uint8_t> data;
            data.push_back(0xE0 | (pb.channel & 0x0F));
            data.push_back(pb.value & 0x7F);
            data.push_back((pb.value >> 7) & 0x7F);
            events.push_back({pb.tick, data});
        }
        
        // Темп
        for (const auto& tempo : m_tempoChanges) {
            std::vector<uint8_t> data;
            data.push_back(0xFF);
            data.push_back(0x51);
            uint32_t us = 60000000 / tempo.bpm;
            auto len = writeVLQ(3);
            data.insert(data.end(), len.begin(), len.end());
            data.push_back((us >> 16) & 0xFF);
            data.push_back((us >> 8) & 0xFF);
            data.push_back(us & 0xFF);
            events.push_back({tempo.tick, data});
        }
        
        // Time Signature
        for (const auto& ts : m_timeSignatures) {
            std::vector<uint8_t> data;
            data.push_back(0xFF);
            data.push_back(0x58);
            auto len = writeVLQ(4);
            data.insert(data.end(), len.begin(), len.end());
            data.push_back(ts.numerator);
            uint8_t denomPow = 0;
            uint8_t d = ts.denominator;
            while (d > 1) { d >>= 1; denomPow++; }
            data.push_back(denomPow);
            data.push_back(ts.clocksPerTick);
            data.push_back(ts.notesPerQuarter);
            events.push_back({ts.tick, data});
        }
        
        // Key Signature
        for (const auto& ks : m_keySignatures) {
            std::vector<uint8_t> data;
            data.push_back(0xFF);
            data.push_back(0x59);
            auto len = writeVLQ(2);
            data.insert(data.end(), len.begin(), len.end());
            data.push_back(static_cast<uint8_t>(ks.sharpsFlats));
            data.push_back(ks.isMinor ? 1 : 0);
            events.push_back({ks.tick, data});
        }
        
        // Text Events
        for (const auto& text : m_textEvents) {
            std::vector<uint8_t> data;
            data.push_back(0xFF);
            data.push_back(text.getMetaType());
            auto len = writeVLQ(static_cast<uint32_t>(text.text.size()));
            data.insert(data.end(), len.begin(), len.end());
            for (char c : text.text) data.push_back(static_cast<uint8_t>(c));
            events.push_back({text.tick, data});
        }
    }
    
    uint16_t m_format;
    uint16_t m_division;
    uint32_t m_initialTempo;
    
    std::vector<Note> m_notes;
    std::vector<TempoChange> m_tempoChanges;
    std::vector<TimeSignature> m_timeSignatures;
    std::vector<KeySignature> m_keySignatures;
    std::vector<TextEvent> m_textEvents;
    std::vector<ControlChange> m_controlChanges;
    std::vector<ProgramChange> m_programChanges;
    std::vector<PitchBend> m_pitchBends;
    std::vector<Sysex> m_sysex;
};

//=============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ РАБОТЫ С НОТАМИ
//=============================================================================

inline uint8_t noteNameToNumber(const std::string& name) {
    const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    if (name.length() < 2) return 60;
    int octave = name.back() - '0';
    std::string noteName = name.substr(0, name.length() - 1);
    for (int i = 0; i < 12; i++) {
        if (noteName == notes[i]) {
            return octave * 12 + i + 12;
        }
    }
    return 60;
}

inline std::string noteNumberToName(uint8_t note) {
    const char* notes[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = (note - 12) / 12;
    int index = note % 12;
    return std::string(notes[index]) + std::to_string(octave);
}

inline uint32_t bpmToMicroseconds(uint32_t bpm) {
    return 60000000 / bpm;
}

inline uint32_t microsecondsToBpm(uint32_t us) {
    return 60000000 / us;
}

inline uint32_t beatsToTicks(double beats, uint16_t division) {
    return static_cast<uint32_t>(beats * division);
}

inline std::vector<Note> createMelody(const std::vector<std::string>& noteNames,
                                       uint8_t velocity, uint32_t durationPerNote) {
    std::vector<Note> result;
    uint32_t currentTick = 0;
    for (const auto& name : noteNames) {
        Note note;
        note.pitch = noteNameToNumber(name);
        note.velocity = velocity;
        note.startTick = currentTick;
        note.duration = durationPerNote;
        result.push_back(note);
        currentTick += durationPerNote;
    }
    return result;
}

} // namespace midi

#endif // MIDI_BUILDER_HPP