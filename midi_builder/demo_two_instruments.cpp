#include "midi_builder.hpp"

int main() {
    midi::MidiBuilder builder;
    builder.setTempo(120);
    
    // ===========================================
    // ИНСТРУМЕНТ 1 (канал 0) - Фортепиано (правая рука)
    // ===========================================
    midi::ProgramChange piano;
    piano.tick = 0;
    piano.program = 0;      // Acoustic Grand Piano
    piano.channel = 0;
    builder.addProgramChange(piano);
    
    // Мелодия на канале 0
    std::vector<std::string> melody = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};
    uint32_t tick = 0;
    for (const auto& noteName : melody) {
        midi::Note note;
        note.pitch = midi::noteNameToNumber(noteName);
        note.velocity = 100;
        note.startTick = tick;
        note.duration = 480;
        note.channel = 0;
        builder.addNote(note);
        tick += 480;
    }
    
    // ===========================================
    // ИНСТРУМЕНТ 2 (канал 1) - Бас (левая рука)
    // ===========================================
    midi::ProgramChange bass;
    bass.tick = 0;
    bass.program = 33;      // Electric Bass (finger)
    bass.channel = 1;
    builder.addProgramChange(bass);
    
    // Басовая партия на канале 1 (играет одновременно с мелодией)
    std::vector<uint8_t> bassNotes = {48, 50, 52, 53, 55, 57, 59, 60};  // C3, D3, E3...
    tick = 0;
    for (auto pitch : bassNotes) {
        midi::Note note;
        note.pitch = pitch;
        note.velocity = 90;
        note.startTick = tick;
        note.duration = 480;
        note.channel = 1;
        builder.addNote(note);
        tick += 480;
    }
    
    // ===========================================
    // ИНСТРУМЕНТ 3 (канал 9) - Ударные
    // ===========================================
    // Канал 9 по умолчанию используется для ударных
    // Не нужно менять программу, ударные всегда на канале 9
    
    // Бас-барабан (kick) на каждой сильной доле
    for (int i = 0; i < 8; i++) {
        midi::Note kick;
        kick.pitch = 36;          // Kick drum
        kick.velocity = 100;
        kick.startTick = i * 960; // каждая 2-я доля
        kick.duration = 240;
        kick.channel = 9;
        builder.addNote(kick);
    }
    
    // Малый барабан (snare) на 2-й и 4-й долях
    midi::Note snare;
    snare.pitch = 38;             // Snare drum
    snare.velocity = 90;
    snare.startTick = 960;
    snare.duration = 240;
    snare.channel = 9;
    builder.addNote(snare);
    
    snare.startTick = 2880;
    builder.addNote(snare);
    
    builder.save("two_instruments.mid");
    
    return 0;
}