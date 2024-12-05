#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "../include/loadHex.h"

uint8_t hexToByte(const string& hex) { 
    return static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
}
vector<uint8_t> loadHexToVector(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Failed to open the file: " + filename);
    }

    vector<uint8_t> data;
    string line;

    while (getline(file, line)) {
        if (line.empty() || line[0] != ':') { // throw error if the line is empty or doesn't start with ':'
            throw runtime_error("Invalid Intel HEX record: " + line);
        }

        line = line.substr(1); // remove the ':'

        // Extract fields
        string byteCountStr = line.substr(0, 2);
        string addrStr = line.substr(2, 4);
        string recordTypeStr = line.substr(6, 2);

        uint8_t byteCount = hexToByte(byteCountStr);
        uint8_t recordType = hexToByte(recordTypeStr);

        if (recordType != 0) { // skip if the record type is not 0
            continue;
        }
        
        for (size_t i = 0; i < byteCount; ++i) {
            string byteStr = line.substr(8 + i * 2, 2);
            uint8_t value = hexToByte(byteStr);
            data.push_back(value);
        }

        string checksumStr = line.substr(8 + byteCount * 2, 2); // extract checksum
        uint8_t checksum = hexToByte(checksumStr);
        uint8_t calculatedChecksum = byteCount + hexToByte(addrStr.substr(0, 2)) + hexToByte(addrStr.substr(2, 2)) + recordType;
        for (size_t i = 0; i < byteCount; ++i) {
            calculatedChecksum += hexToByte(line.substr(8 + i * 2, 2));
        }
        calculatedChecksum = (~calculatedChecksum + 1) & 0xFF;

        if (calculatedChecksum != checksum) {
            throw runtime_error("Checksum mismatch in record: " + line); // throw error if checksum doesn't match
        }
    }

    return data;
}
