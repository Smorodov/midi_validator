#include "midi_builder.hpp"

int main() {
    midi::MidiBuilder builder;
    
    midi::Note note;
    note.pitch = 60;  // C4
    note.velocity = 100;
    note.startTick = 0;
    note.duration = 480;
    builder.addNote(note);
    
    builder.save("output.mid");
    return 0;
}