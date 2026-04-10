#include "midi_generator.h"
#include "test_framework.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>

namespace midi_test {

MidiGenerator::MidiGenerator() {
}

MidiGenerator::~MidiGenerator() {
}

std::vector<uint8_t> MidiGenerator::writeVLQ(uint32_t value) {
    std::vector<uint8_t> result;
    
    if (value == 0) {
        result.push_back(0);
        return result;
    }
    
    std::vector<uint8_t> temp;
    while (value > 0) {
        uint8_t byte = value & 0x7F;
        value >>= 7;
        if (!temp.empty()) {
            byte |= 0x80;
        }
        temp.push_back(byte);
    }
    
    for (auto it = temp.rbegin(); it != temp.rend(); ++it) {
        result.push_back(*it);
    }
    
    return result;
}

std::vector<uint8_t> MidiGenerator::writeUint16(uint16_t value) {
    return {
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
}

std::vector<uint8_t> MidiGenerator::writeUint32(uint32_t value) {
    return {
        static_cast<uint8_t>((value >> 24) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
}

std::vector<uint8_t> MidiGenerator::makeNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    return {static_cast<uint8_t>(0x90 | (channel & 0x0F)), note, velocity};
}

std::vector<uint8_t> MidiGenerator::makeNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    return {static_cast<uint8_t>(0x80 | (channel & 0x0F)), note, velocity};
}

std::vector<uint8_t> MidiGenerator::makeMetaEvent(uint8_t metaType, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> result;
    result.push_back(0xFF);
    result.push_back(metaType);
    auto vlq = writeVLQ(static_cast<uint32_t>(data.size()));
    result.insert(result.end(), vlq.begin(), vlq.end());
    result.insert(result.end(), data.begin(), data.end());
    return result;
}

std::vector<uint8_t> MidiGenerator::makeEndOfTrack() {
    return makeMetaEvent(0x2F, {});
}

std::vector<uint8_t> MidiGenerator::makeTempoEvent(uint32_t microsecondsPerQuarter) {
    std::vector<uint8_t> data;
    data.push_back(static_cast<uint8_t>((microsecondsPerQuarter >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((microsecondsPerQuarter >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>(microsecondsPerQuarter & 0xFF));
    return makeMetaEvent(0x51, data);
}

std::vector<uint8_t> MidiGenerator::makeTimeSignature(uint8_t numerator, uint8_t denominatorPower,
                                                        uint8_t clocksPerTick, uint8_t thirtySecondNotes) {
    std::vector<uint8_t> data = {numerator, denominatorPower, clocksPerTick, thirtySecondNotes};
    return makeMetaEvent(0x58, data);
}

bool MidiGenerator::generateValidFormat0(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0x90);
    file.push_back(60);
    file.push_back(100);
    file.push_back(0x83);
    file.push_back(0x60);
    file.push_back(0x80);
    file.push_back(60);
    file.push_back(64);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateValidFormat1(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x01,
        0x00,0x02,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t track1LenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t track1Start = file.size();
    
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x51);
    file.push_back(0x03);
    file.push_back(0x07);
    file.push_back(0xA1);
    file.push_back(0x20);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t track1Len = static_cast<uint32_t>(file.size() - track1Start);
    file[track1LenPos] = (track1Len >> 24) & 0xFF;
    file[track1LenPos + 1] = (track1Len >> 16) & 0xFF;
    file[track1LenPos + 2] = (track1Len >> 8) & 0xFF;
    file[track1LenPos + 3] = track1Len & 0xFF;
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t track2LenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t track2Start = file.size();
    
    file.push_back(0x00);
    file.push_back(0x90);
    file.push_back(60);
    file.push_back(100);
    file.push_back(0x83);
    file.push_back(0x60);
    file.push_back(0x80);
    file.push_back(60);
    file.push_back(64);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t track2Len = static_cast<uint32_t>(file.size() - track2Start);
    file[track2LenPos] = (track2Len >> 24) & 0xFF;
    file[track2LenPos + 1] = (track2Len >> 16) & 0xFF;
    file[track2LenPos + 2] = (track2Len >> 8) & 0xFF;
    file[track2LenPos + 3] = track2Len & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateValidWithTempo(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x51);
    file.push_back(0x03);
    file.push_back(0x07);
    file.push_back(0xA1);
    file.push_back(0x20);
    file.push_back(0x83);
    file.push_back(0x60);
    file.push_back(0xFF);
    file.push_back(0x51);
    file.push_back(0x03);
    file.push_back(0x0A);
    file.push_back(0x2C);
    file.push_back(0x2B);
    file.push_back(0x00);
    file.push_back(0x90);
    file.push_back(60);
    file.push_back(100);
    file.push_back(0x83);
    file.push_back(0x60);
    file.push_back(0x80);
    file.push_back(60);
    file.push_back(64);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateValidWithTimeSignature(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x58);
    file.push_back(0x04);
    file.push_back(0x04);
    file.push_back(0x02);
    file.push_back(0x18);
    file.push_back(0x08);
    file.push_back(0x83);
    file.push_back(0x60);
    file.push_back(0xFF);
    file.push_back(0x58);
    file.push_back(0x04);
    file.push_back(0x03);
    file.push_back(0x02);
    file.push_back(0x18);
    file.push_back(0x08);
    file.push_back(0x00);
    file.push_back(0x90);
    file.push_back(60);
    file.push_back(100);
    file.push_back(0x83);
    file.push_back(0x60);
    file.push_back(0x80);
    file.push_back(60);
    file.push_back(64);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateValidWithMultipleTracks(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x01,
        0x00,0x03,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t t1LenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t t1Start = file.size();
    uint8_t track1[] = {0x00,0xFF,0x51,0x03,0x07,0xA1,0x20,0x00,0xFF,0x2F,0x00};
    file.insert(file.end(), track1, track1 + sizeof(track1));
    
    uint32_t t1Len = static_cast<uint32_t>(file.size() - t1Start);
    file[t1LenPos] = (t1Len >> 24) & 0xFF;
    file[t1LenPos + 1] = (t1Len >> 16) & 0xFF;
    file[t1LenPos + 2] = (t1Len >> 8) & 0xFF;
    file[t1LenPos + 3] = t1Len & 0xFF;
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t t2LenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t t2Start = file.size();
    uint8_t track2[] = {0x00,0x90,60,100,0x83,0x60,0x80,60,64,0x00,0xFF,0x2F,0x00};
    file.insert(file.end(), track2, track2 + sizeof(track2));
    
    uint32_t t2Len = static_cast<uint32_t>(file.size() - t2Start);
    file[t2LenPos] = (t2Len >> 24) & 0xFF;
    file[t2LenPos + 1] = (t2Len >> 16) & 0xFF;
    file[t2LenPos + 2] = (t2Len >> 8) & 0xFF;
    file[t2LenPos + 3] = t2Len & 0xFF;
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t t3LenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t t3Start = file.size();
    uint8_t track3[] = {0x00,0x91,40,80,0x83,0x60,0x81,40,64,0x00,0xFF,0x2F,0x00};
    file.insert(file.end(), track3, track3 + sizeof(track3));
    
    uint32_t t3Len = static_cast<uint32_t>(file.size() - t3Start);
    file[t3LenPos] = (t3Len >> 24) & 0xFF;
    file[t3LenPos + 1] = (t3Len >> 16) & 0xFF;
    file[t3LenPos + 2] = (t3Len >> 8) & 0xFF;
    file[t3LenPos + 3] = t3Len & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateValidWithSysEx(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0xF0);
    file.push_back(0x05);
    file.push_back(0x7E);
    file.push_back(0x7F);
    file.push_back(0x09);
    file.push_back(0x01);
    file.push_back(0xF7);
    file.push_back(0x00);
    file.push_back(0x90);
    file.push_back(60);
    file.push_back(100);
    file.push_back(0x83);
    file.push_back(0x60);
    file.push_back(0x80);
    file.push_back(60);
    file.push_back(64);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateInvalidHeader(const std::string& filename) {
    std::vector<uint8_t> file;
    file.push_back('X');
    file.push_back('T');
    file.push_back('h');
    file.push_back('d');
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x06);
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateInvalidFormat(const std::string& filename) {
    std::vector<uint8_t> file;
    const uint8_t data[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x63,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), data, data + sizeof(data));
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateMissingEndOfTrack(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0x90);
    file.push_back(60);
    file.push_back(100);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateInvalidNoteRange(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0x90);
    file.push_back(200);
    file.push_back(100);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

// FIXED: Generate truly invalid running status
// First event: valid Note On
// Second event: data byte without previous status (invalid)
bool MidiGenerator::generateInvalidRunningStatus(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    // First event: valid Note On
    file.push_back(0x00);  // delta time
    file.push_back(0x90);  // Note On status
    file.push_back(60);    // note
    file.push_back(100);   // velocity
    
    // Second event: INVALID - data byte without previous status
    // running_status is 0 (reset after first event? or we never set it?)
    // Actually after first event, running_status = 0x90
    // But we need to create a situation where running_status is NOT set
    // The easiest way: start with a data byte without any status before
    
    // Better approach: first event is data byte (invalid)
    // But to make it more realistic, let's reset and create truly invalid sequence
    // Let's rewind and create a file with NO initial status, just data
    
    // Clear and recreate with truly invalid running status
    file.clear();
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    trackStart = file.size();
    
    // Start with a data byte (0x60) without any previous status byte
    // This is INVALID because the first byte after delta time must be status (>= 0x80)
    file.push_back(0x00);  // delta time
    file.push_back(0x60);  // data byte (not a status!) - INVALID
    file.push_back(100);   // second data byte
    
    // End of Track
    file.push_back(0x00);  // delta
    file.push_back(0xFF);  // meta
    file.push_back(0x2F);  // end of track
    file.push_back(0x00);  // length
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateCorruptVLQ(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0xFF);
    file.push_back(0xFF);
    file.push_back(0xFF);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateEmptyFile(const std::string& filename) {
    std::vector<uint8_t> empty;
    return writeBinaryFile(filename, empty);
}

bool MidiGenerator::generateTruncatedFile(const std::string& filename) {
    std::vector<uint8_t> file;
    file.push_back('M');
    file.push_back('T');
    file.push_back('h');
    file.push_back('d');
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateInvalidMetaEvent(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x99);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateInvalidTrackChunk(const std::string& filename) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('X');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x04);
    
    return writeBinaryFile(filename, file);
}

// ============================================================
// METHODS FOR BOUNDARY VALUE TESTS
// ============================================================

bool MidiGenerator::generateNoteOnWithParams(const std::string& filename, uint8_t note, uint8_t velocity) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0x90);
    file.push_back(note);
    file.push_back(velocity);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateNoteOffWithParams(const std::string& filename, uint8_t note, uint8_t velocity) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0x80);
    file.push_back(note);
    file.push_back(velocity);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateControlChangeWithParams(const std::string& filename, uint8_t controller, uint8_t value) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0xB0);
    file.push_back(controller);
    file.push_back(value);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateProgramChangeWithParams(const std::string& filename, uint8_t program) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0xC0);
    file.push_back(program);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generatePitchBendWithParams(const std::string& filename, uint16_t pitchBend) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    uint8_t lsb = pitchBend & 0x7F;
    uint8_t msb = (pitchBend >> 7) & 0x7F;
    
    file.push_back(0x00);
    file.push_back(0xE0);
    file.push_back(lsb);
    file.push_back(msb);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generatePolyAftertouchWithParams(const std::string& filename, uint8_t note, uint8_t pressure) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0xA0);
    file.push_back(note);
    file.push_back(pressure);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

bool MidiGenerator::generateChannelAftertouchWithParams(const std::string& filename, uint8_t pressure) {
    std::vector<uint8_t> file;
    
    const uint8_t header[] = {
        'M','T','h','d',
        0x00,0x00,0x00,0x06,
        0x00,0x00,
        0x00,0x01,
        0x01,0xE0
    };
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M');
    file.push_back('T');
    file.push_back('r');
    file.push_back('k');
    
    size_t trackLenPos = file.size();
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    file.push_back(0x00);
    
    size_t trackStart = file.size();
    
    file.push_back(0x00);
    file.push_back(0xD0);
    file.push_back(pressure);
    file.push_back(0x00);
    file.push_back(0xFF);
    file.push_back(0x2F);
    file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    return writeBinaryFile(filename, file);
}

// Out-of-range wrappers
bool MidiGenerator::generateNoteOutOfRange(const std::string& filename, uint8_t note) {
    return generateNoteOnWithParams(filename, note, 64);
}

bool MidiGenerator::generateVelocityOutOfRange(const std::string& filename, uint8_t velocity) {
    return generateNoteOnWithParams(filename, 60, velocity);
}

bool MidiGenerator::generatePressureOutOfRange(const std::string& filename, uint8_t pressure) {
    return generatePolyAftertouchWithParams(filename, 60, pressure);
}

bool MidiGenerator::generateControllerOutOfRange(const std::string& filename, uint8_t controller) {
    return generateControlChangeWithParams(filename, controller, 64);
}

bool MidiGenerator::generateControlValueOutOfRange(const std::string& filename, uint8_t value) {
    return generateControlChangeWithParams(filename, 7, value);
}

bool MidiGenerator::generateProgramOutOfRange(const std::string& filename, uint8_t program) {
    return generateProgramChangeWithParams(filename, program);
}

bool MidiGenerator::generatePitchBendOutOfRange(const std::string& filename, uint16_t pitchBend) {
    return generatePitchBendWithParams(filename, pitchBend);
}

// Generate all boundary test files (valid and invalid)
bool MidiGenerator::generateBoundaryTestFiles(const std::string& outputDir) {
    if (!createDirectory(outputDir)) {
        std::cerr << "Failed to create directory: " << outputDir << std::endl;
        return false;
    }
    
    bool allSuccess = true;
    
    std::cout << "Generating boundary valid test files...\n";
    
    // Note range tests
    allSuccess &= generateNoteOnWithParams(outputDir + "/valid_note_0.mid", 0, 64);
    allSuccess &= generateNoteOnWithParams(outputDir + "/valid_note_127.mid", 127, 64);
    
    // Velocity range tests
    allSuccess &= generateNoteOnWithParams(outputDir + "/valid_vel_0.mid", 60, 0);
    allSuccess &= generateNoteOnWithParams(outputDir + "/valid_vel_127.mid", 60, 127);
    
    // Controller range tests
    allSuccess &= generateControlChangeWithParams(outputDir + "/valid_controller_0.mid", 0, 64);
    allSuccess &= generateControlChangeWithParams(outputDir + "/valid_controller_127.mid", 127, 64);
    
    // Control value range tests
    allSuccess &= generateControlChangeWithParams(outputDir + "/valid_control_value_0.mid", 7, 0);
    allSuccess &= generateControlChangeWithParams(outputDir + "/valid_control_value_127.mid", 7, 127);
    
    // Program range tests
    allSuccess &= generateProgramChangeWithParams(outputDir + "/valid_program_0.mid", 0);
    allSuccess &= generateProgramChangeWithParams(outputDir + "/valid_program_127.mid", 127);
    
    // Pitch bend range tests
    allSuccess &= generatePitchBendWithParams(outputDir + "/valid_pitchbend_0.mid", 0);
    allSuccess &= generatePitchBendWithParams(outputDir + "/valid_pitchbend_16383.mid", 16383);
    
    // Pressure range tests (Poly Aftertouch)
    allSuccess &= generatePolyAftertouchWithParams(outputDir + "/valid_pressure_0.mid", 60, 0);
    allSuccess &= generatePolyAftertouchWithParams(outputDir + "/valid_pressure_127.mid", 60, 127);
    
    // Channel Aftertouch range tests
    allSuccess &= generateChannelAftertouchWithParams(outputDir + "/valid_channel_pressure_0.mid", 0);
    allSuccess &= generateChannelAftertouchWithParams(outputDir + "/valid_channel_pressure_127.mid", 127);
    
    std::cout << "Generating boundary invalid test files...\n";
    
    // Invalid note range
    allSuccess &= generateNoteOutOfRange(outputDir + "/invalid_note_128.mid", 128);
    
    // Invalid velocity range
    allSuccess &= generateVelocityOutOfRange(outputDir + "/invalid_vel_128.mid", 128);
    
    // Invalid controller range
    allSuccess &= generateControllerOutOfRange(outputDir + "/invalid_controller_128.mid", 128);
    
    // Invalid control value range
    allSuccess &= generateControlValueOutOfRange(outputDir + "/invalid_control_value_128.mid", 128);
    
    // Invalid program range
    allSuccess &= generateProgramOutOfRange(outputDir + "/invalid_program_128.mid", 128);
    
    // Invalid pitch bend range
    allSuccess &= generatePitchBendOutOfRange(outputDir + "/invalid_pitchbend_16384.mid", 16384);
    
    // Invalid pressure range
    allSuccess &= generatePressureOutOfRange(outputDir + "/invalid_pressure_128.mid", 128);
    allSuccess &= generatePressureOutOfRange(outputDir + "/invalid_channel_pressure_128.mid", 128);
    
    if (allSuccess) {
        std::cout << "All boundary test files generated successfully!\n";
    } else {
        std::cout << "Some boundary test files failed to generate!\n";
    }
    
    return allSuccess;
}

bool MidiGenerator::generateAllTestFiles(const std::string& outputDir) {
    if (!createDirectory(outputDir)) {
        std::cerr << "Failed to create directory: " << outputDir << std::endl;
        return false;
    }
    
    bool allSuccess = true;
    
    std::cout << "Generating valid test files...\n";
    allSuccess &= generateValidFormat0(outputDir + "/valid_format0.mid");
    allSuccess &= generateValidFormat1(outputDir + "/valid_format1.mid");
    allSuccess &= generateValidWithTempo(outputDir + "/valid_tempo.mid");
    allSuccess &= generateValidWithTimeSignature(outputDir + "/valid_timesig.mid");
    allSuccess &= generateValidWithMultipleTracks(outputDir + "/valid_multitrack.mid");
    allSuccess &= generateValidWithSysEx(outputDir + "/valid_sysex.mid");
    
    std::cout << "Generating invalid test files...\n";
    allSuccess &= generateInvalidHeader(outputDir + "/invalid_header.mid");
    allSuccess &= generateInvalidFormat(outputDir + "/invalid_format.mid");
    allSuccess &= generateMissingEndOfTrack(outputDir + "/invalid_no_eot.mid");
    allSuccess &= generateInvalidNoteRange(outputDir + "/invalid_note_range.mid");
    allSuccess &= generateInvalidRunningStatus(outputDir + "/invalid_running_status.mid");
    allSuccess &= generateCorruptVLQ(outputDir + "/invalid_corrupt_vlq.mid");
    allSuccess &= generateEmptyFile(outputDir + "/invalid_empty.mid");
    allSuccess &= generateTruncatedFile(outputDir + "/invalid_truncated.mid");
    allSuccess &= generateInvalidMetaEvent(outputDir + "/invalid_meta_event.mid");
    allSuccess &= generateInvalidTrackChunk(outputDir + "/invalid_track_chunk.mid");
    
    // Generate all boundary test files
    allSuccess &= generateBoundaryTestFiles(outputDir);
    
    if (allSuccess) {
        std::cout << "All test files generated successfully!\n";
    } else {
        std::cout << "Some test files failed to generate!\n";
    }
    
    return allSuccess;
}

}