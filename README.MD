# Remu80 - Retro Z80 Emulator

## THIS PROJECT IS A WORK IN PROGRESS. SOME FEATURES MAY BE BROKEN OR MISSING.

Remu80 is a simple Z80 emulator written in C++. Its goal is to be a fully functional emulator of the Z80 instruction set.
It also comes with a compiler for the ZC80 language. ZC80 is a C-like language, which is almost fully compatible with C(with minor exceptions) thus making it easy to port programs from C to ZC80.

## Features
- Z80 instruction set (undocumented instructions will be added later)
- Same registers as the original Z80
- Port ```0x00``` is used for standard input and output
- Watchdog for detecting infinite loops, can be disabled using the ```-w``` flag
- ACIA 6850 support (Work in progress)

## How to use
1. Compile the project using the command ```make```.
2. Run the emulator using the command ```./main -s <program.name>```. For debugging purposes, run with the ```-d``` flag.
3. The program will be loaded into memory and will be executed.

## Options
- ```-s``` - Source program, load and run
- ```-d``` - Enable debugging mode
- ```-t``` - Run tests
- ```-p``` - Print memory after execution
- ```-r``` - Print state after execution
- ```-w``` - Disable watchdog

## License
This project is released under the [GPL V3](https://www.gnu.org/licenses/gpl-3.0.en.html) license