#ifndef LOADEHEX_H
#define LOADEHEX_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cstdint>

using namespace std;

uint8_t hexToByte(const string& hex);

vector<uint8_t> loadHexToVector(const string& filename); // loads Intel HEX file into unsigned 8-bit vector

#endif