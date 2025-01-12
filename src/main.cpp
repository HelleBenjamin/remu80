#include "../include/z80e.h"
#include "../include/loadHex.h"
#include <csignal>
#include <cstdlib>
#include <execinfo.h>
Z80_Core z80;
void handleSignal(int signal) {
    cerr << "Error: Caught signal " << signal << " (Segmentation Fault)" << endl;

    cout << "Current CPU state:" << endl;
    z80.printCurrentState();

    z80.view_program();

    // Exit gracefully (or restart, or other custom behavior)
    exit(signal);
}


int main(int argc, char *argv[]) {
    //cout << "Z80 emulator v1.0 (C) Benjamin Helle 2024" << endl;
    signal(SIGSEGV, handleSignal);
    string filename;
    bool printMemory = false;
    for (int i = 0; i < argc; i++) {
        if ((string(argv[i])).find("-s") == 0) { // source program, load and run
            filename = argv[i + 1];
            vector<uint8_t> executable_program = loadHexToVector(filename);
            z80.loadProgram(executable_program);
        }
        if ((string(argv[i])).find("-d") == 0) { // debug mode
            z80.DEBUG = true;
        }
        if ((string(argv[i])).find("-t") == 0) { // test, you can edit this for testing purposes
            return 0;
        }
        if ((string(argv[i])).find("-p") == 0) { // print memory after execution
            printMemory = true;
        }
        if ((string(argv[i])).find("-r") == 0) { // print state after execution
            z80.printInfo();
        }
        if ((string(argv[i])).find("-w") == 0) { // disable watchdog
            z80.disableWatchdog = true;
        }
    }
    z80.run();
    if (printMemory == true) z80.view_program();

    return 0;
}