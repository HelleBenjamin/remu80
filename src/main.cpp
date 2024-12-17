#include "../include/z80e.h"
#include "../include/loadHex.h"

int main(int argc, char *argv[]) {
    cout << "Z80 emulator v1.0 (C) Benjamin Helle 2024" << endl;
    Z80_Core z80;
    string filename;
    for (int i = 0; i < argc; i++) {
        if ((string(argv[i])).find("-p") == 0) { // source program
            filename = argv[i + 1];
        }
        if ((string(argv[i])).find("-d") == 0) { // debug mode
            z80.DEBUG = true;
        }
        if ((string(argv[i])).find("-t") == 0) { // test, you can edit this for testing purposes
            return 0;
        }
    }
    vector<uint8_t> executable_program = loadHexToVector(filename);
    z80.loadProgram(executable_program);
    z80.run();
    return 0;
}