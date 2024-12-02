#ifndef Z80E_H
#define Z80E_H

#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <map>
#include <unordered_map>
//#include <windows.h> //uncomment for windows
#include <unistd.h> //comment for windows
#include <fstream>
#include <cstdint>
#define RAM_SIZE 65536

using namespace std;
class Z80_Core {
    public:
        bool DEBUG;
        Z80_Core();
        void reset();
        void loadProgram(vector<uint8_t>& inputProgram);
        void run();
        void view_program();
        void view_ram();
        void printInfo();
        void printCurrentState();
    private:
        uint8_t ins;
        uint8_t memory[RAM_SIZE];
        bool halt, interrupts;
        vector<string> rom;
        unsigned pc; // program counter
        unsigned sp; // stack pointer
        uint8_t f; // flags
        uint8_t w,z; // temp regs to hold operands
        int acc; // accumulator
        uint8_t a, b, c, d, e, h, l; // main registers, can be made to 16-bit register pairs
        uint16_t afa, bca, dea, hla; // alternate register pairs
        uint16_t ix, iy; // index registers
        bool isInput;

        void alu(size_t& op1, size_t& op2, uint8_t ins);

        uint8_t fetchOperand();
        void fetchInstruction();
        uint8_t inputHandler();
        uint8_t outputHandler(uint8_t &reg);
        void swapRegs(uint8_t& temp1, uint8_t& temp2);
        void flagsAfterCalcA();
        void decode_execute();
        void ed_instruction(uint8_t ins);
};
#endif