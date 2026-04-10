#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include "midi_analyzer.h"

#define VERSION "6.0.0"

int main(int argc, char* argv[]) {
    std::cout << "\n+--------------------------------------------------+\n";
    std::cout << "|     MIDI VALIDATOR v" << VERSION << " (C++)                  |\n";
    std::cout << "+--------------------------------------------------+\n";
    
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <midi_file>\n";
        return 1;
    }
    
    MidiAnalyzer analyzer;
    MidiInfo info = analyzer.analyze(argv[1]);
    analyzer.printReport(argv[1], info);
    
    std::cout << "\nExit code: " << (info.is_valid ? 0 : 1) << "\n";
    return info.is_valid ? 0 : 1;
}