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
#include <termios.h>

#define MEMORY_SIZE 0xffff

#define FLAG_C 0x01
#define FLAG_N 0x02
#define FLAG_P 0x04
#define FLAG_H 0x20
#define FLAG_Z 0x40
#define FLAG_S 0x80


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

#define ALU_BIT0 0x15
#define ALU_BIT1 0x16
#define ALU_BIT2 0x17
#define ALU_BIT3 0x18
#define ALU_BIT4 0x19
#define ALU_BIT5 0x1A
#define ALU_BIT6 0x1B
#define ALU_BIT7 0x1C
#define ALU_RES0 0x1D
#define ALU_RES1 0x1E
#define ALU_RES2 0x1F
#define ALU_RES3 0x20
#define ALU_RES4 0x21
#define ALU_RES5 0x22
#define ALU_RES6 0x23
#define ALU_RES7 0x24
#define ALU_SET0 0x25
#define ALU_SET1 0x26
#define ALU_SET2 0x27
#define ALU_SET3 0x28
#define ALU_SET4 0x29
#define ALU_SET5 0x2A
#define ALU_SET6 0x2B
#define ALU_SET7 0x2C

#define ALU_INC16 0x2D
#define ALU_DEC16 0x2E

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

        void testAlu(uint8_t& reg, uint8_t reg2, uint8_t ins);
        int nop_watchdog = 0; // prevent infinite loops
        bool disableWatchdog = false;
    private:
        uint8_t ins;
        uint8_t memory[MEMORY_SIZE] = {0};
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
        void decode_execute(uint8_t instruction);

        void ed_instruction(uint8_t ins); // extended instructions
        void cb_instruction(uint8_t ins); // bit instructions
        void dd_instruction(uint8_t ins); // ix prefix instructions
        void fd_instruction(uint8_t ins); // iy prefix instructions
};
#endif