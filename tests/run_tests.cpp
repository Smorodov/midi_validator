#include "test_framework.h"
#include "midi_generator.h"
#include <iostream>
#include <cstdlib>
#include <filesystem>

using namespace midi_test;

std::string validatorPath;

bool runValidator(const std::string& midiFile, int& exitCode) {
    if (!std::filesystem::exists(midiFile)) {
        std::cerr << "ERROR: Test file does not exist: " << midiFile << std::endl;
        return false;
    }
    
    std::string fullValidatorPath = std::filesystem::absolute(validatorPath).string();
    std::string fullMidiPath = std::filesystem::absolute(midiFile).string();
    std::string cmd = "\"" + fullValidatorPath + "\" \"" + fullMidiPath + "\"";
    
#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return false;
    
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // consume output
    }
    exitCode = _pclose(pipe);
    return true;
#else
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;
    
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // consume output
    }
    exitCode = pclose(pipe);
    return true;
#endif
}

// Existing test functions
bool testValidFormat0(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/valid_format0.mid";
    
    if (!gen.generateValidFormat0(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode != 0) {
        errorMsg = "Validator returned non-zero exit code for valid file";
        return false;
    }
    
    return true;
}

bool testValidFormat1(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/valid_format1.mid";
    
    if (!gen.generateValidFormat1(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode != 0) {
        errorMsg = "Validator returned non-zero exit code for valid file";
        return false;
    }
    
    return true;
}

bool testValidWithTempo(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/valid_tempo.mid";
    
    if (!gen.generateValidWithTempo(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode != 0) {
        errorMsg = "Validator returned non-zero exit code for valid file";
        return false;
    }
    
    return true;
}

bool testValidWithTimeSignature(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/valid_timesig.mid";
    
    if (!gen.generateValidWithTimeSignature(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode != 0) {
        errorMsg = "Validator returned non-zero exit code for valid file";
        return false;
    }
    
    return true;
}

bool testValidMultiTrack(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/valid_multitrack.mid";
    
    if (!gen.generateValidWithMultipleTracks(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode != 0) {
        errorMsg = "Validator returned non-zero exit code for valid file";
        return false;
    }
    
    return true;
}

bool testValidSysEx(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/valid_sysex.mid";
    
    if (!gen.generateValidWithSysEx(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode != 0) {
        errorMsg = "Validator returned non-zero exit code for valid file";
        return false;
    }
    
    return true;
}

bool testInvalidHeader(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/invalid_header.mid";
    
    if (!gen.generateInvalidHeader(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode == 0) {
        errorMsg = "Validator returned zero exit code for invalid file";
        return false;
    }
    
    return true;
}

bool testInvalidFormat(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/invalid_format.mid";
    
    if (!gen.generateInvalidFormat(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode == 0) {
        errorMsg = "Validator returned zero exit code for invalid file";
        return false;
    }
    
    return true;
}

bool testMissingEndOfTrack(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/invalid_no_eot.mid";
    
    if (!gen.generateMissingEndOfTrack(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode == 0) {
        errorMsg = "Validator returned zero exit code for invalid file";
        return false;
    }
    
    return true;
}

bool testInvalidNoteRange(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/invalid_note_range.mid";
    
    if (!gen.generateInvalidNoteRange(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode == 0) {
        errorMsg = "Validator returned zero exit code for invalid file";
        return false;
    }
    
    return true;
}

bool testInvalidRunningStatus(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/invalid_running_status.mid";
    
    if (!gen.generateInvalidRunningStatus(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode == 0) {
        errorMsg = "Validator returned zero exit code for invalid file";
        return false;
    }
    
    return true;
}

bool testCorruptVLQ(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/invalid_corrupt_vlq.mid";
    
    if (!gen.generateCorruptVLQ(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode == 0) {
        errorMsg = "Validator returned zero exit code for invalid file";
        return false;
    }
    
    return true;
}

bool testEmptyFile(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/invalid_empty.mid";
    
    if (!gen.generateEmptyFile(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode == 0) {
        errorMsg = "Validator returned zero exit code for invalid file";
        return false;
    }
    
    return true;
}

bool testTruncatedFile(std::string& errorMsg) {
    MidiGenerator gen;
    std::string testFile = "tests/generated/invalid_truncated.mid";
    
    if (!gen.generateTruncatedFile(testFile)) {
        errorMsg = "Failed to generate test file";
        return false;
    }
    
    int exitCode;
    if (!runValidator(testFile, exitCode)) {
        errorMsg = "Failed to run validator";
        return false;
    }
    
    if (exitCode == 0) {
        errorMsg = "Validator returned zero exit code for invalid file";
        return false;
    }
    
    return true;
}

// ============================================================
// BOUNDARY VALUE TESTS
// ============================================================

bool testNoteRangeBoundary(std::string& errorMsg) {
    MidiGenerator gen;
    int exitCode;
    
    std::string testFile1 = "tests/generated/valid_note_0.mid";
    if (!gen.generateNoteOnWithParams(testFile1, 0, 64)) {
        errorMsg = "Failed to generate test file for note=0";
        return false;
    }
    if (!runValidator(testFile1, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on note=0 (should be valid)";
        return false;
    }
    
    std::string testFile2 = "tests/generated/valid_note_127.mid";
    if (!gen.generateNoteOnWithParams(testFile2, 127, 64)) {
        errorMsg = "Failed to generate test file for note=127";
        return false;
    }
    if (!runValidator(testFile2, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on note=127 (should be valid)";
        return false;
    }
    
    std::string testFile3 = "tests/generated/invalid_note_128.mid";
    if (!gen.generateNoteOutOfRange(testFile3, 128)) {
        errorMsg = "Failed to generate test file for note=128";
        return false;
    }
    if (!runValidator(testFile3, exitCode) || exitCode == 0) {
        errorMsg = "Validator accepted note=128 (should reject)";
        return false;
    }
    
    return true;
}

bool testVelocityRangeBoundary(std::string& errorMsg) {
    MidiGenerator gen;
    int exitCode;
    
    std::string testFile1 = "tests/generated/valid_vel_0.mid";
    if (!gen.generateNoteOnWithParams(testFile1, 60, 0)) {
        errorMsg = "Failed to generate test file for velocity=0";
        return false;
    }
    if (!runValidator(testFile1, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on velocity=0 (should be valid)";
        return false;
    }
    
    std::string testFile2 = "tests/generated/valid_vel_127.mid";
    if (!gen.generateNoteOnWithParams(testFile2, 60, 127)) {
        errorMsg = "Failed to generate test file for velocity=127";
        return false;
    }
    if (!runValidator(testFile2, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on velocity=127 (should be valid)";
        return false;
    }
    
    std::string testFile3 = "tests/generated/invalid_vel_128.mid";
    if (!gen.generateVelocityOutOfRange(testFile3, 128)) {
        errorMsg = "Failed to generate test file for velocity=128";
        return false;
    }
    if (!runValidator(testFile3, exitCode) || exitCode == 0) {
        errorMsg = "Validator accepted velocity=128 (should reject)";
        return false;
    }
    
    return true;
}

bool testControllerRangeBoundary(std::string& errorMsg) {
    MidiGenerator gen;
    int exitCode;
    
    std::string testFile1 = "tests/generated/valid_controller_0.mid";
    if (!gen.generateControlChangeWithParams(testFile1, 0, 64)) {
        errorMsg = "Failed to generate test file for controller=0";
        return false;
    }
    if (!runValidator(testFile1, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on controller=0 (should be valid)";
        return false;
    }
    
    std::string testFile2 = "tests/generated/valid_controller_127.mid";
    if (!gen.generateControlChangeWithParams(testFile2, 127, 64)) {
        errorMsg = "Failed to generate test file for controller=127";
        return false;
    }
    if (!runValidator(testFile2, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on controller=127 (should be valid)";
        return false;
    }
    
    std::string testFile3 = "tests/generated/invalid_controller_128.mid";
    if (!gen.generateControllerOutOfRange(testFile3, 128)) {
        errorMsg = "Failed to generate test file for controller=128";
        return false;
    }
    if (!runValidator(testFile3, exitCode) || exitCode == 0) {
        errorMsg = "Validator accepted controller=128 (should reject)";
        return false;
    }
    
    return true;
}

bool testControlValueRangeBoundary(std::string& errorMsg) {
    MidiGenerator gen;
    int exitCode;
    
    std::string testFile1 = "tests/generated/valid_control_value_0.mid";
    if (!gen.generateControlChangeWithParams(testFile1, 7, 0)) {
        errorMsg = "Failed to generate test file for control value=0";
        return false;
    }
    if (!runValidator(testFile1, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on control value=0 (should be valid)";
        return false;
    }
    
    std::string testFile2 = "tests/generated/valid_control_value_127.mid";
    if (!gen.generateControlChangeWithParams(testFile2, 7, 127)) {
        errorMsg = "Failed to generate test file for control value=127";
        return false;
    }
    if (!runValidator(testFile2, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on control value=127 (should be valid)";
        return false;
    }
    
    std::string testFile3 = "tests/generated/invalid_control_value_128.mid";
    if (!gen.generateControlValueOutOfRange(testFile3, 128)) {
        errorMsg = "Failed to generate test file for control value=128";
        return false;
    }
    if (!runValidator(testFile3, exitCode) || exitCode == 0) {
        errorMsg = "Validator accepted control value=128 (should reject)";
        return false;
    }
    
    return true;
}

bool testProgramRangeBoundary(std::string& errorMsg) {
    MidiGenerator gen;
    int exitCode;
    
    std::string testFile1 = "tests/generated/valid_program_0.mid";
    if (!gen.generateProgramChangeWithParams(testFile1, 0)) {
        errorMsg = "Failed to generate test file for program=0";
        return false;
    }
    if (!runValidator(testFile1, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on program=0 (should be valid)";
        return false;
    }
    
    std::string testFile2 = "tests/generated/valid_program_127.mid";
    if (!gen.generateProgramChangeWithParams(testFile2, 127)) {
        errorMsg = "Failed to generate test file for program=127";
        return false;
    }
    if (!runValidator(testFile2, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on program=127 (should be valid)";
        return false;
    }
    
    std::string testFile3 = "tests/generated/invalid_program_128.mid";
    if (!gen.generateProgramOutOfRange(testFile3, 128)) {
        errorMsg = "Failed to generate test file for program=128";
        return false;
    }
    if (!runValidator(testFile3, exitCode) || exitCode == 0) {
        errorMsg = "Validator accepted program=128 (should reject)";
        return false;
    }
    
    return true;
}

bool testPitchBendRangeBoundary(std::string& errorMsg) {
    MidiGenerator gen;
    int exitCode;
    
    std::string testFile1 = "tests/generated/valid_pitchbend_0.mid";
    if (!gen.generatePitchBendWithParams(testFile1, 0)) {
        errorMsg = "Failed to generate test file for pitch bend=0";
        return false;
    }
    if (!runValidator(testFile1, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on pitch bend=0 (should be valid)";
        return false;
    }
    
    std::string testFile2 = "tests/generated/valid_pitchbend_16383.mid";
    if (!gen.generatePitchBendWithParams(testFile2, 16383)) {
        errorMsg = "Failed to generate test file for pitch bend=16383";
        return false;
    }
    if (!runValidator(testFile2, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on pitch bend=16383 (should be valid)";
        return false;
    }
    
    std::string testFile3 = "tests/generated/invalid_pitchbend_16384.mid";
    if (!gen.generatePitchBendOutOfRange(testFile3, 16384)) {
        errorMsg = "Failed to generate test file for pitch bend=16384";
        return false;
    }
    if (!runValidator(testFile3, exitCode) || exitCode == 0) {
        errorMsg = "Validator accepted pitch bend=16384 (should reject)";
        return false;
    }
    
    return true;
}

bool testPolyAftertouchRange(std::string& errorMsg) {
    MidiGenerator gen;
    int exitCode;
    
    std::string testFile1 = "tests/generated/valid_pressure_0.mid";
    if (!gen.generatePolyAftertouchWithParams(testFile1, 60, 0)) {
        errorMsg = "Failed to generate test file for pressure=0";
        return false;
    }
    if (!runValidator(testFile1, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on pressure=0 (should be valid)";
        return false;
    }
    
    std::string testFile2 = "tests/generated/valid_pressure_127.mid";
    if (!gen.generatePolyAftertouchWithParams(testFile2, 60, 127)) {
        errorMsg = "Failed to generate test file for pressure=127";
        return false;
    }
    if (!runValidator(testFile2, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on pressure=127 (should be valid)";
        return false;
    }
    
    std::string testFile3 = "tests/generated/invalid_pressure_128.mid";
    if (!gen.generatePressureOutOfRange(testFile3, 128)) {
        errorMsg = "Failed to generate test file for pressure=128";
        return false;
    }
    if (!runValidator(testFile3, exitCode) || exitCode == 0) {
        errorMsg = "Validator accepted pressure=128 (should reject)";
        return false;
    }
    
    return true;
}

bool testChannelAftertouchRange(std::string& errorMsg) {
    MidiGenerator gen;
    int exitCode;
    
    std::string testFile1 = "tests/generated/valid_channel_pressure_0.mid";
    if (!gen.generateChannelAftertouchWithParams(testFile1, 0)) {
        errorMsg = "Failed to generate test file for channel pressure=0";
        return false;
    }
    if (!runValidator(testFile1, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on channel pressure=0 (should be valid)";
        return false;
    }
    
    std::string testFile2 = "tests/generated/valid_channel_pressure_127.mid";
    if (!gen.generateChannelAftertouchWithParams(testFile2, 127)) {
        errorMsg = "Failed to generate test file for channel pressure=127";
        return false;
    }
    if (!runValidator(testFile2, exitCode) || exitCode != 0) {
        errorMsg = "Validator failed on channel pressure=127 (should be valid)";
        return false;
    }
    
    std::string testFile3 = "tests/generated/invalid_channel_pressure_128.mid";
    if (!gen.generatePressureOutOfRange(testFile3, 128)) {
        errorMsg = "Failed to generate test file for channel pressure=128";
        return false;
    }
    if (!runValidator(testFile3, exitCode) || exitCode == 0) {
        errorMsg = "Validator accepted channel pressure=128 (should reject)";
        return false;
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "MIDI Validator Test Suite\n";
    std::cout << "==========================\n\n";
    
    // Generate all test files first
    MidiGenerator gen;
    std::cout << "Generating all test files...\n";
    if (!gen.generateAllTestFiles("tests/generated")) {
        std::cerr << "Warning: Some test files failed to generate\n";
    }
    std::cout << "\n";
    
    if (argc >= 2) {
        validatorPath = argv[1];
    } else {
        std::vector<std::string> possiblePaths = {
            "../midi_validator.exe",
            "../../midi_validator.exe",
            "midi_validator.exe",
            "../build/midi_validator.exe",
            "../../build/midi_validator.exe",
            "build/midi_validator.exe"
        };
        
        for (const auto& path : possiblePaths) {
            if (std::filesystem::exists(path)) {
                validatorPath = path;
                break;
            }
        }
        
        if (validatorPath.empty()) {
            std::cerr << "Error: Could not find midi_validator.exe\n";
            std::cerr << "Please provide path as argument: run_tests.exe <path_to_validator>\n";
            return 1;
        }
    }
    
    std::cout << "Using validator: " << validatorPath << "\n\n";
    
    createDirectory("tests/generated");
    createDirectory("tests/reports");
    
    TestFramework framework;
    
    std::cout << "--- Basic Validation Tests ---\n";
    framework.addTest("Valid Format 0", testValidFormat0);
    framework.addTest("Valid Format 1", testValidFormat1);
    framework.addTest("Valid Tempo Changes", testValidWithTempo);
    framework.addTest("Valid Time Signatures", testValidWithTimeSignature);
    framework.addTest("Valid Multi-Track", testValidMultiTrack);
    framework.addTest("Valid SysEx", testValidSysEx);
    
    std::cout << "\n--- Invalid File Tests ---\n";
    framework.addTest("Invalid Header", testInvalidHeader);
    framework.addTest("Invalid Format", testInvalidFormat);
    framework.addTest("Missing End of Track", testMissingEndOfTrack);
    framework.addTest("Invalid Note Range", testInvalidNoteRange);
    framework.addTest("Invalid Running Status", testInvalidRunningStatus);
    framework.addTest("Corrupt VLQ", testCorruptVLQ);
    framework.addTest("Empty File", testEmptyFile);
    framework.addTest("Truncated File", testTruncatedFile);
    
    std::cout << "\n--- Boundary Value Tests (New Validator Checks) ---\n";
    framework.addTest("Note Range (0-127)", testNoteRangeBoundary);
    framework.addTest("Velocity Range (0-127)", testVelocityRangeBoundary);
    framework.addTest("Controller Number Range (0-127)", testControllerRangeBoundary);
    framework.addTest("Control Value Range (0-127)", testControlValueRangeBoundary);
    framework.addTest("Program Number Range (0-127)", testProgramRangeBoundary);
    framework.addTest("Pitch Bend Range (0-16383)", testPitchBendRangeBoundary);
    framework.addTest("Poly Aftertouch Pressure Range", testPolyAftertouchRange);
    framework.addTest("Channel Aftertouch Pressure Range", testChannelAftertouchRange);
    
    framework.runAll();
    
    std::string reportFile = "tests/reports/test_report_" + 
                             std::to_string(std::time(nullptr)) + ".txt";
    framework.saveReport(reportFile);
    std::cout << "\nReport saved to: " << reportFile << "\n";
    
    return (framework.getTotalFailed() == 0) ? 0 : 1;
}