#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <filesystem>

namespace midi_test {

std::vector<uint8_t> writeVLQ(uint32_t value) {
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
    for (auto it = temp.rbegin(); it != temp.rend(); ++it) result.push_back(*it);
    return result;
}

bool writeFile(const std::string& filename, const std::vector<uint8_t>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    file.close();
    std::cout << "  Created: " << filename << " (" << data.size() << " bytes)" << std::endl;
    return true;
}

void generateValidFormat0(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    file.push_back(0x00); file.push_back(0x90); file.push_back(60); file.push_back(100);
    file.push_back(0x83); file.push_back(0x60); file.push_back(0x80); file.push_back(60); file.push_back(64);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/valid_format0.mid", file);
}

void generateValidFormat1(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x01,0x00,0x02,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t t1LenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t t1Start = file.size();
    uint8_t track1[] = {0x00,0xFF,0x51,0x03,0x07,0xA1,0x20,0x00,0xFF,0x2F,0x00};
    file.insert(file.end(), track1, track1 + sizeof(track1));
    uint32_t t1Len = static_cast<uint32_t>(file.size() - t1Start);
    file[t1LenPos] = (t1Len >> 24) & 0xFF;
    file[t1LenPos+1] = (t1Len >> 16) & 0xFF;
    file[t1LenPos+2] = (t1Len >> 8) & 0xFF;
    file[t1LenPos+3] = t1Len & 0xFF;
    
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t t2LenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t t2Start = file.size();
    uint8_t track2[] = {0x00,0x90,60,100,0x83,0x60,0x80,60,64,0x00,0xFF,0x2F,0x00};
    file.insert(file.end(), track2, track2 + sizeof(track2));
    uint32_t t2Len = static_cast<uint32_t>(file.size() - t2Start);
    file[t2LenPos] = (t2Len >> 24) & 0xFF;
    file[t2LenPos+1] = (t2Len >> 16) & 0xFF;
    file[t2LenPos+2] = (t2Len >> 8) & 0xFF;
    file[t2LenPos+3] = t2Len & 0xFF;
    writeFile(dir + "/valid_format1.mid", file);
}

void generateValidMultiTrack(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x01,0x00,0x03,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t t1LenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t t1Start = file.size();
    uint8_t track1[] = {0x00,0xFF,0x51,0x03,0x07,0xA1,0x20,0x00,0xFF,0x2F,0x00};
    file.insert(file.end(), track1, track1 + sizeof(track1));
    uint32_t t1Len = static_cast<uint32_t>(file.size() - t1Start);
    file[t1LenPos] = (t1Len >> 24) & 0xFF;
    file[t1LenPos+1] = (t1Len >> 16) & 0xFF;
    file[t1LenPos+2] = (t1Len >> 8) & 0xFF;
    file[t1LenPos+3] = t1Len & 0xFF;
    
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t t2LenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t t2Start = file.size();
    uint8_t track2[] = {0x00,0x90,60,100,0x83,0x60,0x80,60,64,0x00,0xFF,0x2F,0x00};
    file.insert(file.end(), track2, track2 + sizeof(track2));
    uint32_t t2Len = static_cast<uint32_t>(file.size() - t2Start);
    file[t2LenPos] = (t2Len >> 24) & 0xFF;
    file[t2LenPos+1] = (t2Len >> 16) & 0xFF;
    file[t2LenPos+2] = (t2Len >> 8) & 0xFF;
    file[t2LenPos+3] = t2Len & 0xFF;
    
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t t3LenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t t3Start = file.size();
    uint8_t track3[] = {0x00,0x91,40,80,0x83,0x60,0x81,40,64,0x00,0xFF,0x2F,0x00};
    file.insert(file.end(), track3, track3 + sizeof(track3));
    uint32_t t3Len = static_cast<uint32_t>(file.size() - t3Start);
    file[t3LenPos] = (t3Len >> 24) & 0xFF;
    file[t3LenPos+1] = (t3Len >> 16) & 0xFF;
    file[t3LenPos+2] = (t3Len >> 8) & 0xFF;
    file[t3LenPos+3] = t3Len & 0xFF;
    writeFile(dir + "/valid_multitrack.mid", file);
}

void generateValidTempo(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x51); file.push_back(0x03);
    file.push_back(0x07); file.push_back(0xA1); file.push_back(0x20);
    file.push_back(0x83); file.push_back(0x60);
    file.push_back(0xFF); file.push_back(0x51); file.push_back(0x03);
    file.push_back(0x0A); file.push_back(0x2C); file.push_back(0x2B);
    file.push_back(0x00); file.push_back(0x90); file.push_back(60); file.push_back(100);
    file.push_back(0x83); file.push_back(0x60); file.push_back(0x80); file.push_back(60); file.push_back(64);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/valid_tempo.mid", file);
}

void generateValidTimeSignature(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x58); file.push_back(0x04);
    file.push_back(0x04); file.push_back(0x02); file.push_back(0x18); file.push_back(0x08);
    file.push_back(0x83); file.push_back(0x60);
    file.push_back(0xFF); file.push_back(0x58); file.push_back(0x04);
    file.push_back(0x03); file.push_back(0x02); file.push_back(0x18); file.push_back(0x08);
    file.push_back(0x00); file.push_back(0x90); file.push_back(60); file.push_back(100);
    file.push_back(0x83); file.push_back(0x60); file.push_back(0x80); file.push_back(60); file.push_back(64);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/valid_timesig.mid", file);
}

void generateValidSysEx(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    file.push_back(0x00); file.push_back(0xF0); file.push_back(0x05);
    file.push_back(0x7E); file.push_back(0x7F); file.push_back(0x09); file.push_back(0x01); file.push_back(0xF7);
    file.push_back(0x00); file.push_back(0x90); file.push_back(60); file.push_back(100);
    file.push_back(0x83); file.push_back(0x60); file.push_back(0x80); file.push_back(60); file.push_back(64);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/valid_sysex.mid", file);
}

void generateValidMetaEvents(const std::string& dir) {
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
    file.push_back(0xFF); file.push_back(0x00); file.push_back(0x02);
    file.push_back(0x00); file.push_back(0x01);
    
    file.push_back(0x00);
    file.push_back(0xFF); file.push_back(0x03); file.push_back(0x04);
    file.push_back('T'); file.push_back('e'); file.push_back('s'); file.push_back('t');
    
    file.push_back(0x00);
    file.push_back(0xFF); file.push_back(0x04); file.push_back(0x04);
    file.push_back('P'); file.push_back('i'); file.push_back('a'); file.push_back('n');
    
    file.push_back(0x00);
    file.push_back(0xFF); file.push_back(0x51); file.push_back(0x03);
    file.push_back(0x07); file.push_back(0xA1); file.push_back(0x20);
    
    file.push_back(0x00);
    file.push_back(0x90); file.push_back(60); file.push_back(100);
    
    file.push_back(0x83);
    file.push_back(0x60);
    file.push_back(0x80); file.push_back(60); file.push_back(64);
    
    file.push_back(0x00);
    file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    writeFile(dir + "/valid_meta_events.mid", file);
}

void generateValidChannelMode(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    
    file.push_back(0x00); file.push_back(0xB0); file.push_back(120); file.push_back(0x00);
    file.push_back(0x00); file.push_back(0xB0); file.push_back(121); file.push_back(0x00);
    file.push_back(0x00); file.push_back(0xB0); file.push_back(123); file.push_back(0x00);
    file.push_back(0x00); file.push_back(0xB0); file.push_back(124); file.push_back(0x00);
    file.push_back(0x00); file.push_back(0xB0); file.push_back(125); file.push_back(0x00);
    file.push_back(0x00); file.push_back(0xB0); file.push_back(126); file.push_back(0x04);
    file.push_back(0x00); file.push_back(0xB0); file.push_back(127); file.push_back(0x00);
    
    file.push_back(0x00); file.push_back(0x90); file.push_back(60); file.push_back(100);
    auto delta480 = writeVLQ(480);
    file.insert(file.end(), delta480.begin(), delta480.end());
    file.push_back(0x80); file.push_back(60); file.push_back(64);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/valid_channel_mode.mid", file);
}

void generateValidSystemCommon(const std::string& dir) {
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
    
    // Note On (delta 0)
    file.push_back(0x00);
    file.push_back(0x90); file.push_back(60); file.push_back(100);
    
    // Delta 480
    file.push_back(0x83);
    file.push_back(0x60);
    
    // Note Off
    file.push_back(0x80); file.push_back(60); file.push_back(64);
    
    // End of Track
    file.push_back(0x00);
    file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    writeFile(dir + "/valid_system_common.mid", file);
}

void generateValidSystemRealTime(const std::string& dir) {
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
    
    // Note On (delta 0)
    file.push_back(0x00);
    file.push_back(0x90); file.push_back(60); file.push_back(100);
    
    // Delta 480
    file.push_back(0x83);
    file.push_back(0x60);
    
    // Note Off
    file.push_back(0x80); file.push_back(60); file.push_back(64);
    
    // End of Track
    file.push_back(0x00);
    file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos + 1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos + 2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos + 3] = trackLen & 0xFF;
    
    writeFile(dir + "/valid_system_realtime.mid", file);
}

void generateValidFormat2(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x02,0x00,0x02,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t t1LenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t t1Start = file.size();
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x00); file.push_back(0x02);
    file.push_back(0x00); file.push_back(0x01);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t t1Len = static_cast<uint32_t>(file.size() - t1Start);
    file[t1LenPos] = (t1Len >> 24) & 0xFF;
    file[t1LenPos+1] = (t1Len >> 16) & 0xFF;
    file[t1LenPos+2] = (t1Len >> 8) & 0xFF;
    file[t1LenPos+3] = t1Len & 0xFF;
    
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t t2LenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t t2Start = file.size();
    file.push_back(0x00); file.push_back(0x90); file.push_back(60); file.push_back(100);
    auto delta480 = writeVLQ(480);
    file.insert(file.end(), delta480.begin(), delta480.end());
    file.push_back(0x80); file.push_back(60); file.push_back(64);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t t2Len = static_cast<uint32_t>(file.size() - t2Start);
    file[t2LenPos] = (t2Len >> 24) & 0xFF;
    file[t2LenPos+1] = (t2Len >> 16) & 0xFF;
    file[t2LenPos+2] = (t2Len >> 8) & 0xFF;
    file[t2LenPos+3] = t2Len & 0xFF;
    writeFile(dir + "/valid_format2.mid", file);
}

void generateInvalidNoEOT(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    file.push_back(0x00); file.push_back(0x90); file.push_back(60); file.push_back(100);
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/invalid_no_eot.mid", file);
}

void generateInvalidNoteRange(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    file.push_back(0x00); file.push_back(0x90); file.push_back(200); file.push_back(100);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/invalid_note_range.mid", file);
}

void generateInvalidRunningStatus(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    file.push_back(0x00); file.push_back(0x60); file.push_back(100);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/invalid_running_status.mid", file);
}

void generateInvalidEmpty(const std::string& dir) {
    std::vector<uint8_t> file;
    writeFile(dir + "/invalid_empty.mid", file);
}

void generateInvalidTruncated(const std::string& dir) {
    std::vector<uint8_t> file;
    file.push_back('M'); file.push_back('T'); file.push_back('h'); file.push_back('d');
    writeFile(dir + "/invalid_truncated.mid", file);
}

void generateInvalidHeader(const std::string& dir) {
    std::vector<uint8_t> file;
    file.push_back('X'); file.push_back('T'); file.push_back('h'); file.push_back('d');
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x06);
    writeFile(dir + "/invalid_header.mid", file);
}

void generateInvalidFormat(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t data[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x63,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), data, data + sizeof(data));
    writeFile(dir + "/invalid_format.mid", file);
}

void generateInvalidMetaEvent(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('M'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    size_t trackLenPos = file.size();
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x00);
    size_t trackStart = file.size();
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x99); file.push_back(0x00);
    file.push_back(0x00); file.push_back(0xFF); file.push_back(0x2F); file.push_back(0x00);
    uint32_t trackLen = static_cast<uint32_t>(file.size() - trackStart);
    file[trackLenPos] = (trackLen >> 24) & 0xFF;
    file[trackLenPos+1] = (trackLen >> 16) & 0xFF;
    file[trackLenPos+2] = (trackLen >> 8) & 0xFF;
    file[trackLenPos+3] = trackLen & 0xFF;
    writeFile(dir + "/invalid_meta_event.mid", file);
}

void generateInvalidTrackChunk(const std::string& dir) {
    std::vector<uint8_t> file;
    const uint8_t header[] = {'M','T','h','d',0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0x01,0xE0};
    file.insert(file.end(), header, header + sizeof(header));
    file.push_back('X'); file.push_back('T'); file.push_back('r'); file.push_back('k');
    file.push_back(0x00); file.push_back(0x00); file.push_back(0x00); file.push_back(0x04);
    writeFile(dir + "/invalid_track_chunk.mid", file);
}

} // namespace midi_test

int main() {
    std::string outDir = "tests/generated";
    
    std::filesystem::create_directories(outDir);
    
    std::cout << "Generating MIDI test files in: " << outDir << "\n\n";
    
    std::cout << "Valid files:\n";
    midi_test::generateValidFormat0(outDir);
    midi_test::generateValidFormat1(outDir);
    midi_test::generateValidMultiTrack(outDir);
    midi_test::generateValidTempo(outDir);
    midi_test::generateValidTimeSignature(outDir);
    midi_test::generateValidSysEx(outDir);
    midi_test::generateValidMetaEvents(outDir);
    midi_test::generateValidChannelMode(outDir);
    midi_test::generateValidSystemCommon(outDir);
    midi_test::generateValidSystemRealTime(outDir);
    midi_test::generateValidFormat2(outDir);
    
    std::cout << "\nInvalid files:\n";
    midi_test::generateInvalidNoEOT(outDir);
    midi_test::generateInvalidNoteRange(outDir);
    midi_test::generateInvalidRunningStatus(outDir);
    midi_test::generateInvalidEmpty(outDir);
    midi_test::generateInvalidTruncated(outDir);
    midi_test::generateInvalidHeader(outDir);
    midi_test::generateInvalidFormat(outDir);
    midi_test::generateInvalidMetaEvent(outDir);
    midi_test::generateInvalidTrackChunk(outDir);
    
    std::cout << "\nDone! Total files: 20\n";
    return 0;
}