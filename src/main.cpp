#include "midi_analyzer.h"
#include "report_formatter.h"
#include <iostream>
#include <string>
#include <memory>

void printUsage(const char* programName) {
    std::cout << "\nMIDI Validator v7.0.0\n";
    std::cout << "Usage: " << programName << " <midi_file> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --format, -f <format>   Output format: console, json, silent\n";
    std::cout << "  --help, -h              Show this help\n";
    std::cout << "  --version, -v           Show version\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " song.mid\n";
    std::cout << "  " << programName << " song.mid --format json\n";
    std::cout << "  " << programName << " song.mid --format silent\n\n";
}

void printVersion() {
    std::cout << "MIDI Validator v7.0.0\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 2;
    }
    
    std::string filename;
    std::string formatName = "console";
    bool showHelp = false;
    bool showVersion = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") showHelp = true;
        else if (arg == "--version" || arg == "-v") showVersion = true;
        else if ((arg == "--format" || arg == "-f") && i + 1 < argc) formatName = argv[++i];
        else if (arg[0] != '-') filename = arg;
    }
    
    if (showHelp) { printUsage(argv[0]); return 0; }
    if (showVersion) { printVersion(); return 0; }
    if (filename.empty()) { printUsage(argv[0]); return 2; }
    
    auto formatter = midi::FormatterFactory::create(formatName);
    if (!formatter) {
        std::cerr << "Unknown format: " << formatName << "\n";
        return 2;
    }
    
    midi::MidiAnalyzer analyzer;
    midi::AnalysisResult result = analyzer.analyzeFile(filename);
    
    if (formatName != "silent") std::cout << formatter->format(result, filename);
    return result.isValid ? 0 : 1;
}