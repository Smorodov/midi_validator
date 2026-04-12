//=============================================================================
// ДЕМОНСТРАЦИОННЫЙ ПРИМЕР MIDI BUILDER API
//=============================================================================
// Компиляция: g++ -std=c++17 demo.cpp -o demo.exe
// Запуск: demo.exe
//=============================================================================

#include "midi_builder.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace midi;

void printSeparator() {
    std::cout << "========================================" << std::endl;
}

void printNoteInfo(const Note& note) {
    std::cout << "  Note: " << noteNumberToName(note.pitch) 
              << " (" << (int)note.pitch << ")"
              << " ch=" << (int)note.channel
              << " vel=" << (int)note.velocity
              << " start=" << note.startTick
              << " dur=" << note.duration << std::endl;
}

int main() {
    std::cout << "\n";
    printSeparator();
    std::cout << "MIDI BUILDER API - ДЕМОНСТРАЦИОННЫЙ ПРИМЕР" << std::endl;
    printSeparator();
    std::cout << std::endl;

    //=========================================================================
    // 1. БАЗОВЫЙ ПРИМЕР - ОДНА НОТА
    //=========================================================================
    std::cout << "1. БАЗОВЫЙ ПРИМЕР - ОДНА НОТА" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        Note note;
        note.pitch = noteNameToNumber("C4");
        note.velocity = 100;
        note.startTick = 0;
        note.duration = 480;
        builder.addNote(note);
        
        builder.save("01_single_note.mid");
        std::cout << "  Создан: 01_single_note.mid" << std::endl;
        printNoteInfo(note);
    }
    std::cout << std::endl;

    //=========================================================================
    // 2. ПРОСТАЯ МЕЛОДИЯ
    //=========================================================================
    std::cout << "2. ПРОСТАЯ МЕЛОДИЯ" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        std::vector<std::string> melodyNames = {
            "C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"
        };
        
        auto notes = createMelody(melodyNames, 100, 240);
        for (const auto& note : notes) {
            builder.addNote(note);
            printNoteInfo(note);
        }
        
        builder.save("02_simple_melody.mid");
        std::cout << "  Создан: 02_simple_melody.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 3. АККОРДЫ
    //=========================================================================
    std::cout << "3. АККОРДЫ" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        // C major
        std::vector<uint8_t> cMajor = {
            noteNameToNumber("C4"),
            noteNameToNumber("E4"),
            noteNameToNumber("G4")
        };
        builder.addChord(0, cMajor, 80, 960, 0);
        std::cout << "  Аккорд C major: C4, E4, G4 (tick=0)" << std::endl;
        
        // F major
        std::vector<uint8_t> fMajor = {
            noteNameToNumber("F4"),
            noteNameToNumber("A4"),
            noteNameToNumber("C5")
        };
        builder.addChord(960, fMajor, 80, 960, 0);
        std::cout << "  Аккорд F major: F4, A4, C5 (tick=960)" << std::endl;
        
        // G major
        std::vector<uint8_t> gMajor = {
            noteNameToNumber("G4"),
            noteNameToNumber("B4"),
            noteNameToNumber("D5")
        };
        builder.addChord(1920, gMajor, 80, 960, 0);
        std::cout << "  Аккорд G major: G4, B4, D5 (tick=1920)" << std::endl;
        
        builder.save("03_chords.mid");
        std::cout << "  Создан: 03_chords.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 4. АРПЕДЖИО
    //=========================================================================
    std::cout << "4. АРПЕДЖИО" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        std::vector<uint8_t> arpeggioNotes = {
            noteNameToNumber("C4"),
            noteNameToNumber("E4"),
            noteNameToNumber("G4"),
            noteNameToNumber("C5")
        };
        builder.addArpeggio(0, arpeggioNotes, 90, 120, 480, 0);
        
        std::cout << "  Арпеджио C major: ";
        for (auto n : arpeggioNotes) {
            std::cout << noteNumberToName(n) << " ";
        }
        std::cout << std::endl;
        
        builder.save("04_arpeggio.mid");
        std::cout << "  Создан: 04_arpeggio.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 5. ГАММА
    //=========================================================================
    std::cout << "5. ГАММА" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        std::vector<uint8_t> scale;
        for (int i = 0; i < 8; i++) {
            scale.push_back(60 + i);
        }
        
        for (size_t i = 0; i < scale.size(); i++) {
            Note note;
            note.pitch = scale[i];
            note.velocity = 100;
            note.startTick = i * 240;
            note.duration = 240;
            builder.addNote(note);
            printNoteInfo(note);
        }
        
        builder.save("05_scale.mid");
        std::cout << "  Создан: 05_scale.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 6. СМЕНА ТЕМПА
    //=========================================================================
    std::cout << "6. СМЕНА ТЕМПА" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        // Мелодия
        for (int i = 0; i < 8; i++) {
            Note note;
            note.pitch = 60 + i;
            note.velocity = 100;
            note.startTick = i * 240;
            note.duration = 240;
            builder.addNote(note);
        }
        
        // Ускоряемся после 2 секунд
        TempoChange faster;
        faster.tick = 960;
        faster.bpm = 180;
        builder.addTempoChange(faster);
        std::cout << "  Темп 120 BPM -> 180 BPM на tick=960" << std::endl;
        
        // Замедляемся
        TempoChange slower;
        slower.tick = 1920;
        slower.bpm = 100;
        builder.addTempoChange(slower);
        std::cout << "  Темп 180 BPM -> 100 BPM на tick=1920" << std::endl;
        
        builder.save("06_tempo_change.mid");
        std::cout << "  Создан: 06_tempo_change.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 7. РАЗМЕР ТАКТА И ТОНАЛЬНОСТЬ
    //=========================================================================
    std::cout << "7. РАЗМЕР ТАКТА И ТОНАЛЬНОСТЬ" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        // Вальс 3/4
        TimeSignature waltz;
        waltz.tick = 0;
        waltz.numerator = 3;
        waltz.denominator = 4;
        builder.addTimeSignature(waltz);
        std::cout << "  Размер: 3/4" << std::endl;
        
        // D major (2 диеза)
        KeySignature dMajor;
        dMajor.tick = 0;
        dMajor.sharpsFlats = 2;
        dMajor.isMinor = false;
        builder.addKeySignature(dMajor);
        std::cout << "  Тональность: D major (2 диеза)" << std::endl;
        
        builder.save("07_timesig_keysig.mid");
        std::cout << "  Создан: 07_timesig_keysig.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 8. ПРОГРАММА И КОНТРОЛЛЕРЫ
    //=========================================================================
    std::cout << "8. ПРОГРАММА И КОНТРОЛЛЕРЫ" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        // Выбор инструмента (Acoustic Grand Piano)
        ProgramChange piano;
        piano.tick = 0;
        piano.program = 0;
        builder.addProgramChange(piano);
        std::cout << "  Инструмент: Acoustic Grand Piano (program 0)" << std::endl;
        
        // Громкость
        ControlChange volume;
        volume.tick = 0;
        volume.controller = 7;
        volume.value = 100;
        builder.addControlChange(volume);
        std::cout << "  Громкость: 100" << std::endl;
        
        // Панорама
        ControlChange pan;
        pan.tick = 0;
        pan.controller = 10;
        pan.value = 64;
        builder.addControlChange(pan);
        std::cout << "  Панорама: центр (64)" << std::endl;
        
        // Реверберация
        ControlChange reverb;
        reverb.tick = 0;
        reverb.controller = 91;
        reverb.value = 64;
        builder.addControlChange(reverb);
        std::cout << "  Реверберация: 64" << std::endl;
        
        // Ноты
        for (int i = 0; i < 8; i++) {
            Note note;
            note.pitch = 60 + i;
            note.velocity = 100;
            note.startTick = i * 240;
            note.duration = 240;
            builder.addNote(note);
        }
        
        builder.save("08_program_control.mid");
        std::cout << "  Создан: 08_program_control.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 9. ПИТЧ БЕНД (ИЗМЕНЕНИЕ ВЫСОТЫ)
    //=========================================================================
    std::cout << "9. ПИТЧ БЕНД" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        // Нота
        Note note;
        note.pitch = 60;
        note.velocity = 100;
        note.startTick = 0;
        note.duration = 960;
        builder.addNote(note);
        
        // Подъем высоты
        PitchBend bendUp;
        bendUp.tick = 240;
        bendUp.value = 10000;
        builder.addPitchBend(bendUp);
        std::cout << "  Pitch bend up: 8192 -> 10000 на tick=240" << std::endl;
        
        // Возврат к центру
        PitchBend bendCenter;
        bendCenter.tick = 480;
        bendCenter.value = 8192;
        builder.addPitchBend(bendCenter);
        std::cout << "  Pitch bend center: 8192 на tick=480" << std::endl;
        
        builder.save("09_pitch_bend.mid");
        std::cout << "  Создан: 09_pitch_bend.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 10. МЕТАДАННЫЕ (ТЕКСТ, НАЗВАНИЕ)
    //=========================================================================
    std::cout << "10. МЕТАДАННЫЕ" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setTempo(120);
        
        // Название трека
        TextEvent title;
        title.tick = 0;
        title.text = "My Awesome Song";
        title.type = TextEvent::TRACK_NAME;
        builder.addText(title);
        std::cout << "  Название: My Awesome Song" << std::endl;
        
        // Авторские права
        TextEvent copyright;
        copyright.tick = 0;
        copyright.text = "Copyright 2024";
        copyright.type = TextEvent::COPYRIGHT;
        builder.addText(copyright);
        std::cout << "  Авторские права: Copyright 2024" << std::endl;
        
        // Текст песни
        TextEvent lyric;
        lyric.tick = 480;
        lyric.text = "Hello World!";
        lyric.type = TextEvent::LYRIC;
        builder.addText(lyric);
        std::cout << "  Текст: Hello World! на tick=480" << std::endl;
        
        builder.save("10_metadata.mid");
        std::cout << "  Создан: 10_metadata.mid" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 11. ПОЛНАЯ КОМПОЗИЦИЯ
    //=========================================================================
    std::cout << "11. ПОЛНАЯ КОМПОЗИЦИЯ" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setFormat(0);
        builder.setDivision(480);
        builder.setTempo(120);
        
        // Метаданные
        TextEvent title;
        title.tick = 0;
        title.text = "Complete Demo Song";
        title.type = TextEvent::TRACK_NAME;
        builder.addText(title);
        
        // Инструмент
        ProgramChange piano;
        piano.tick = 0;
        piano.program = 0;
        builder.addProgramChange(piano);
        
        // Громкость
        ControlChange volume;
        volume.tick = 0;
        volume.controller = 7;
        volume.value = 90;
        builder.addControlChange(volume);
        
        // Аккомпанемент (аккорды на канале 1)
        std::vector<uint8_t> chordC = {48, 52, 55};  // C3, E3, G3
        std::vector<uint8_t> chordF = {53, 57, 60};  // F3, A3, C4
        std::vector<uint8_t> chordG = {55, 59, 62};  // G3, B3, D4
        
        builder.addChord(0, chordC, 70, 1920, 1);
        builder.addChord(1920, chordF, 70, 1920, 1);
        builder.addChord(3840, chordG, 70, 1920, 1);
        builder.addChord(5760, chordC, 70, 1920, 1);
        
        // Мелодия (канал 0)
        std::vector<std::string> melodyNotes = {
            "C4", "E4", "G4", "A4", "G4", "E4", "C4",
            "D4", "F4", "A4", "C5", "A4", "F4", "D4",
            "E4", "G4", "B4", "C5", "B4", "G4", "E4",
            "C4"
        };
        
        uint32_t tick = 0;
        for (const auto& noteName : melodyNotes) {
            Note note;
            note.pitch = noteNameToNumber(noteName);
            note.velocity = 100;
            note.startTick = tick;
            note.duration = 480;
            note.channel = 0;
            builder.addNote(note);
            tick += 480;
        }
        
        builder.save("11_full_composition.mid");
        std::cout << "  Создан: 11_full_composition.mid" << std::endl;
        std::cout << "  Длительность: " << melodyNotes.size() << " нот" << std::endl;
        std::cout << "  Аккордов: 4" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // 12. МУЛЬТИТРЕК (ФОРМАТ 1)
    //=========================================================================
    std::cout << "12. МУЛЬТИТРЕК" << std::endl;
    printSeparator();
    
    {
        MidiBuilder builder;
        builder.setFormat(1);
        builder.setDivision(480);
        builder.setTempo(120);
        
        // Трек 0 - Мелодия
        std::vector<std::string> melody = {"C4", "E4", "G4", "C5"};
        uint32_t tick = 0;
        for (const auto& noteName : melody) {
            Note note;
            note.pitch = noteNameToNumber(noteName);
            note.velocity = 100;
            note.startTick = tick;
            note.duration = 480;
            note.channel = 0;
            builder.addNote(note);
            tick += 480;
        }
        
        // Трек 1 - Бас
        std::vector<uint8_t> bassNotes = {48, 53, 55, 48};
        tick = 0;
        for (auto pitch : bassNotes) {
            Note note;
            note.pitch = pitch;
            note.velocity = 90;
            note.startTick = tick;
            note.duration = 480;
            note.channel = 1;
            builder.addNote(note);
            tick += 480;
        }
        
        builder.save("12_multitrack.mid");
        std::cout << "  Создан: 12_multitrack.mid" << std::endl;
        std::cout << "  Формат: 1 (multi-track)" << std::endl;
        std::cout << "  Трек 0: мелодия (канал 0)" << std::endl;
        std::cout << "  Трек 1: бас (канал 1)" << std::endl;
    }
    std::cout << std::endl;

    //=========================================================================
    // ИТОГИ
    //=========================================================================
    printSeparator();
    std::cout << "ВСЕ ПРИМЕРЫ УСПЕШНО СОЗДАНЫ!" << std::endl;
    printSeparator();
    std::cout << std::endl;
    std::cout << "Созданные файлы:" << std::endl;
    std::cout << "  01_single_note.mid" << std::endl;
    std::cout << "  02_simple_melody.mid" << std::endl;
    std::cout << "  03_chords.mid" << std::endl;
    std::cout << "  04_arpeggio.mid" << std::endl;
    std::cout << "  05_scale.mid" << std::endl;
    std::cout << "  06_tempo_change.mid" << std::endl;
    std::cout << "  07_timesig_keysig.mid" << std::endl;
    std::cout << "  08_program_control.mid" << std::endl;
    std::cout << "  09_pitch_bend.mid" << std::endl;
    std::cout << "  10_metadata.mid" << std::endl;
    std::cout << "  11_full_composition.mid" << std::endl;
    std::cout << "  12_multitrack.mid" << std::endl;
    std::cout << std::endl;
    
    return 0;
}