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

#define MEMORY_SIZE 0xffff


#define ALU_ADD8 0x00
#define ALU_ADD16 0x01
#define ALU_ADC8 0x02
#define ALU_ADC16 0x03
#define ALU_SUB8 0x04
#define ALU_SUB16 0x05
#define ALU_SBC8 0x06
#define ALU_SBC16 0x07
#define ALU_AND8 0x08
#define ALU_OR8 0x09
#define ALU_XOR8 0x0A
#define ALU_CP8 0x0B
#define ALU_INC8 0x0C
#define ALU_DEC8 0x0D
#define ALU_RLC8 0x0E
#define ALU_RL8 0x0F
#define ALU_RRC8 0x10
#define ALU_RR8 0x11
#define ALU_SLA8 0x12
#define ALU_SRA8 0x13
#define ALU_SRL8 0x14

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
        uint8_t memory[MEMORY_SIZE];
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
        uint8_t i, r; // interrupt and refresh register
        uint8_t im; // interrupt mode
        bool iff1, iff2;

        bool isInput; 

        void alu(uint16_t& op1, uint16_t op2, uint8_t ins);

        uint8_t fetchOperand();
        void fetchInstruction();
        uint8_t inputHandler(uint8_t port);
        uint8_t outputHandler(uint8_t &reg, uint8_t port);
        void interruptHandler();
        void swapRegs(uint8_t& temp1, uint8_t& temp2);
        uint16_t convToRegPair(uint8_t l, uint8_t h); //used for 16-bit operations
        void decode_execute();

        void ed_instruction(uint8_t ins); // extended instructions
        void cb_instruction(uint8_t ins); // bit instructions
};
#endif