#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "../include/loadHex.h"

namespace {
    constexpr uint8_t DATA_RECORD = 0; // Intel HEX record type for data
}

// Convert a 2-character hex string to a byte
uint8_t hexToByte(const std::string& hex) {
    if (hex.size() != 2 || !std::isxdigit(hex[0]) || !std::isxdigit(hex[1])) {
        throw std::invalid_argument("Invalid hex string: " + hex);
    }
    return static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
}

// Load an Intel HEX file into a vector of bytes
std::vector<uint8_t> loadHexToVector(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file: " + filename);
    }

    std::vector<uint8_t> data;
    std::string line;
    size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Check for the starting colon
        if (line[0] != ':') {
            throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Missing ':' at the start of record.");
        }

        // Remove the colon and check line length
        line = line.substr(1);
        if (line.size() < 10) { // Minimum length for a valid record
            throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Record too short to be valid.");
        }

        try {
            // Parse fields
            std::string byteCountStr = line.substr(0, 2);
            std::string addrStr = line.substr(2, 4);
            std::string recordTypeStr = line.substr(6, 2);

            uint8_t byteCount = hexToByte(byteCountStr);
            uint8_t recordType = hexToByte(recordTypeStr);

            // Verify that the line is long enough for the stated byte count
            size_t expectedLength = 8 + byteCount * 2 + 2; // Fields + data bytes + checksum
            if (line.size() < expectedLength) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Record length mismatch.");
            }

            // Skip non-data records
            if (recordType != DATA_RECORD) {
                continue;
            }

            // Extract data bytes
            uint16_t address = hexToByte(addrStr.substr(0, 2)) << 8 | hexToByte(addrStr.substr(2, 2));

            for (size_t i = 0; i < byteCount; ++i) {
                std::string byteStr = line.substr(8 + i * 2, 2);
                uint8_t value = hexToByte(byteStr);

                if (address + i >= data.size()) {
                    data.resize(address + i + 1, 0);
                }

                data[address + i] = value;
            }

            // Verify checksum
            std::string checksumStr = line.substr(8 + byteCount * 2, 2);
            uint8_t checksum = hexToByte(checksumStr);

            uint8_t calculatedChecksum = byteCount;
            calculatedChecksum += hexToByte(addrStr.substr(0, 2));
            calculatedChecksum += hexToByte(addrStr.substr(2, 2));
            calculatedChecksum += recordType;

            for (size_t i = 0; i < byteCount; ++i) {
                calculatedChecksum += hexToByte(line.substr(8 + i * 2, 2));
            }

            calculatedChecksum = (~calculatedChecksum + 1) & 0xFF;

            if (calculatedChecksum != checksum) {
                throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Checksum mismatch.");
            }

        } catch (const std::exception& e) {
            throw std::runtime_error("Line " + std::to_string(lineNumber) + ": " + e.what());
        }
    }

    return data;
}

