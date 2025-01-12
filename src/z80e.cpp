
#include "../include/z80e.h"
#include <fcntl.h>

#define clear() printf("\033[H\033[J") // macro to clear the screen

using namespace std;

vector<uint8_t> program;

/* Z80 EMULATOR by Benjamin Helle (C) 2024-2025 */

/*
    FLAGS
    7 - Sign S
    6 - Zero Z
    5 - Not used
    4 - Half carry H
    3 - Not used
    2 - Parity/overflow P/V
    1 - Add/Subtract N
    0 - Carry C

*/

map<uint8_t, string> Opcodes = {
    {0x00, "NOP"},
    {0x01, "LD BC, nn"},
    {0x02, "LD (BC), A"},
    {0x03, "INC BC"},
    {0x04, "INC B"},
    {0x05, "DEC B"},
    {0x06, "LD B, n"},
    {0x07, "RLCA"},
    {0x08, "EX AF, AF'"},
    {0x09, "ADD HL, BC"},
    {0x0A, "LD A, (BC)"},
    {0x0B, "DEC BC"},
    {0x0C, "INC C"},
    {0x0D, "DEC C"},
    {0x0E, "LD C, n"},
    {0x0F, "RRCA"},
    {0x10, "DJNZ e"},
    {0x11, "LD DE, nn"},
    {0x12, "LD (DE), A"},
    {0x13, "INC DE"},
    {0x14, "INC D"},
    {0x15, "DEC D"},
    {0x16, "LD D, n"},
    {0x17, "RLA"},
    {0x18, "JR e"},
    {0x19, "ADD HL, DE"},
    {0x1A, "LD A, (DE)"},
    {0x1B, "DEC DE"},
    {0x1C, "INC E"},
    {0x1D, "DEC E"},
    {0x1E, "LD E, n"},
    {0x1F, "RRA"},
    {0x20, "JR NZ, e"},
    {0x21, "LD HL, nn"},
    {0x22, "LD (nn), HL"},
    {0x23, "INC HL"},
    {0x24, "INC H"},
    {0x25, "DEC H"},
    {0x26, "LD H, n"},
    {0x27, "DAA"},
    {0x28, "JR Z, e"},
    {0x29, "ADD HL, HL"},
    {0x2A, "LD HL, (nn)"},
    {0x2B, "DEC HL"},
    {0x2C, "INC L"},
    {0x2D, "DEC L"},
    {0x2E, "LD L, n"},
    {0x2F, "CPL"},
    {0x30, "JR NC, e"},
    {0x31, "LD SP, nn"},
    {0x32, "LD (nn), A"},
    {0x33, "INC SP"},
    {0x34, "INC (HL)"},
    {0x35, "DEC (HL)"},
    {0x36, "LD (HL), n"},
    {0x37, "SCF"},
    {0x38, "JR C, e"},
    {0x39, "ADD HL, SP"},
    {0x3A, "LD A, (nn)"},
    {0x3B, "DEC SP"},
    {0x3C, "INC A"},
    {0x3D, "DEC A"},
    {0x3E, "LD A, n"},
    {0x3F, "CCF"},
    {0x40, "LD B, B"},
    {0x41, "LD B, C"},
    {0x42, "LD B, D"},
    {0x43, "LD B, E"},
    {0x44, "LD B, H"},
    {0x45, "LD B, L"},
    {0x46, "LD B, (HL)"},
    {0x47, "LD B, A"},
    {0x48, "LD C, B"},
    {0x49, "LD C, C"},
    {0x4A, "LD C, D"},
    {0x4B, "LD C, E"},
    {0x4C, "LD C, H"},
    {0x4D, "LD C, L"},
    {0x4E, "LD C, (HL)"},
    {0x4F, "LD C, A"},
    {0x50, "LD D, B"},
    {0x51, "LD D, C"},
    {0x52, "LD D, D"},
    {0x53, "LD D, E"},
    {0x54, "LD D, H"},
    {0x55, "LD D, L"},
    {0x56, "LD D, (HL)"},
    {0x57, "LD D, A"},
    {0x58, "LD E, B"},
    {0x59, "LD E, C"},
    {0x5A, "LD E, D"},
    {0x5B, "LD E, E"},
    {0x5C, "LD E, H"},
    {0x5D, "LD E, L"},
    {0x5E, "LD E, (HL)"},
    {0x5F, "LD E, A"},
    {0x60, "LD H, B"},
    {0x61, "LD H, C"},
    {0x62, "LD H, D"},
    {0x63, "LD H, E"},
    {0x64, "LD H, H"},
    {0x65, "LD H, L"},
    {0x66, "LD H, (HL)"},
    {0x67, "LD H, A"},
    {0x68, "LD L, B"},
    {0x69, "LD L, C"},
    {0x6A, "LD L, D"},
    {0x6B, "LD L, E"},
    {0x6C, "LD L, H"},
    {0x6D, "LD L, L"},
    {0x6E, "LD L, (HL)"},
    {0x6F, "LD L, A"},
    {0x70, "LD (HL), B"},
    {0x71, "LD (HL), C"},
    {0x72, "LD (HL), D"},
    {0x73, "LD (HL), E"},
    {0x74, "LD (HL), H"},
    {0x75, "LD (HL), L"},
    {0x76, "HALT"},
    {0x77, "LD (HL), A"},
    {0x78, "LD A, B"},
    {0x79, "LD A, C"},
    {0x7A, "LD A, D"},
    {0x7B, "LD A, E"},
    {0x7C, "LD A, H"},
    {0x7D, "LD A, L"},
    {0x7E, "LD A, (HL)"},
    {0x7F, "LD A, A"},
    {0x80, "ADD A, B"},
    {0x81, "ADD A, C"},
    {0x82, "ADD A, D"},
    {0x83, "ADD A, E"},
    {0x84, "ADD A, H"},
    {0x85, "ADD A, L"},
    {0x86, "ADD A, (HL)"},
    {0x87, "ADD A, A"},
    {0x88, "ADC A, B"},
    {0x89, "ADC A, C"},
    {0x8A, "ADC A, D"},
    {0x8B, "ADC A, E"},
    {0x8C, "ADC A, H"},
    {0x8D, "ADC A, L"},
    {0x8E, "ADC A, (HL)"},
    {0x8F, "ADC A, A"},
    {0x90, "SUB B"},
    {0x91, "SUB C"},
    {0x92, "SUB D"},
    {0x93, "SUB E"},
    {0x94, "SUB H"},
    {0x95, "SUB L"},
    {0x96, "SUB (HL)"},
    {0x97, "SUB A"},
    {0x98, "SBC A, B"},
    {0x99, "SBC A, C"},
    {0x9A, "SBC A, D"},
    {0x9B, "SBC A, E"},
    {0x9C, "SBC A, H"},
    {0x9D, "SBC A, L"},
    {0x9E, "SBC A, (HL)"},
    {0x9F, "SBC A, A"},
    {0xA0, "AND B"},
    {0xA1, "AND C"},
    {0xA2, "AND D"},
    {0xA3, "AND E"},
    {0xA4, "AND H"},
    {0xA5, "AND L"},
    {0xA6, "AND (HL)"},
    {0xA7, "AND A"},
    {0xA8, "XOR B"},
    {0xA9, "XOR C"},
    {0xAA, "XOR D"},
    {0xAB, "XOR E"},
    {0xAC, "XOR H"},
    {0xAD, "XOR L"},
    {0xAE, "XOR (HL)"},
    {0xAF, "XOR A"},
    {0xB0, "OR B"},
    {0xB1, "OR C"},
    {0xB2, "OR D"},
    {0xB3, "OR E"},
    {0xB4, "OR H"},
    {0xB5, "OR L"},
    {0xB6, "OR (HL)"},
    {0xB7, "OR A"},
    {0xB8, "CP B"},
    {0xB9, "CP C"},
    {0xBA, "CP D"},
    {0xBB, "CP E"},
    {0xBC, "CP H"},
    {0xBD, "CP L"},
    {0xBE, "CP (HL)"},
    {0xBF, "CP A"},
    {0xC0, "RET NZ"},
    {0xC1, "POP BC"},
    {0xC2, "JP NZ, nn"},
    {0xC3, "JP nn"},
    {0xC4, "CALL NZ, nn"},
    {0xC5, "PUSH BC"},
    {0xC6, "ADD A, n"},
    {0xC7, "RST 00H"},
    {0xC8, "RET Z"},
    {0xC9, "RET"},
    {0xCA, "JP Z, nn"},
    {0xCB, "PREFIX CB"},
    {0xCC, "CALL Z, nn"},
    {0xCD, "CALL nn"},
    {0xCE, "ADC A, n"},
    {0xCF, "RST 08H"},
    {0xD0, "RET NC"},
    {0xD1, "POP DE"},
    {0xD2, "JP NC, nn"},
    {0xD3, "OUT (n), A"},
    {0xD4, "CALL NC, nn"},
    {0xD5, "PUSH DE"},
    {0xD6, "SUB n"},
    {0xD7, "RST 10H"},
    {0xD8, "RET C"},
    {0xD9, "EXX"},
    {0xDA, "JP C, nn"},
    {0xDB, "IN A, (n)"},
    {0xDC, "CALL C, nn"},
    {0xDD, "PREFIX DD"},
    {0xDE, "SBC A, n"},
    {0xDF, "RST 18H"},
    {0xE0, "RET PO"},
    {0xE1, "POP HL"},
    {0xE2, "JP PO, nn"},
    {0xE3, "EX (SP), HL"},
    {0xE4, "CALL PO, nn"},
    {0xE5, "PUSH HL"},
    {0xE6, "AND n"},
    {0xE7, "RST 20H"},
    {0xE8, "RET PE"},
    {0xE9, "JP (HL)"},
    {0xEA, "JP PE, nn"},
    {0xEB, "EX DE, HL"},
    {0xEC, "CALL PE, nn"},
    {0xED, "PREFIX ED"},
    {0xEE, "XOR n"},
    {0xEF, "RST 28H"},
    {0xF0, "RET P"},
    {0xF1, "POP AF"},
    {0xF2, "JP P, nn"},
    {0xF3, "DI"},
    {0xF4, "CALL P, nn"},
    {0xF5, "PUSH AF"},
    {0xF6, "OR n"},
    {0xF7, "RST 30H"},
    {0xF8, "RET M"},
    {0xF9, "LD SP, HL"},
    {0xFA, "JP M, nn"},
    {0xFB, "EI"},
    {0xFC, "CALL M, nn"},
    {0xFD, "PREFIX FD"},
    {0xFE, "CP n"},
    {0xFF, "RST 38H"}
};


Z80_Core::Z80_Core() {
    reset(); // initialize the cpu
}

void Z80_Core::loadProgram(vector<uint8_t>& inputProgram) {
    for (unsigned i = 0; i < inputProgram.size(); i++) {
        memory[i] = inputProgram[i];
    }
    cout << "Program loaded, " << inputProgram.size() << " bytes" << endl;

}
void Z80_Core::view_ram() {
    int addr1, addr2;
    cout << "Z80 RAM viewer v1.0" << endl;
    cin >> addr1 >> addr2;
    for (int i = addr1; i <= addr2; i++) {
        cout << i << ": " << unsigned(memory[i]) << endl;
    }
    system("pause");
}

void Z80_Core::view_program() {
    int i = 0;
    cout << "Z80 program viewer v1.0" << endl;
    while (true) {
        if (i > MEMORY_SIZE) break;
        cout << hex << (unsigned)i << ": " << unsigned(memory[i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i]) << " " << unsigned(memory[++i])  << endl;
        i++;
    }

}
vector<uint8_t> ExecutedInstructions;

void Z80_Core::reset() {
    a = b = c = d = e = h = l = 0;
    afa = bca = dea = hla = 0;
    ix = iy = 0;
    pc = 0;
    sp = MEMORY_SIZE; // set sp to top of memory
    acc = 0;
    f = 0;
    halt = false;
    isInput = false;
    iff1 = iff2 = false;
}

void Z80_Core::run() {
    reset();
    pc = 0;
    uint8_t opcode;
    while (!halt) {
        opcode = fetchOperand();
        decode_execute(opcode);
        ACIA_6850_Handler();
        if(isPending) interruptHandler();
        usleep(500); // adjust delay
    }
    if (DEBUG) {
        printInfo();
        cout << "Executed " << ExecutedInstructions.size() << " instructions:" << endl;
        for (int i = 0; i < ExecutedInstructions.size(); i++) { // print executed instructions
            cout << Opcodes[ExecutedInstructions[i]] << endl;
        }
    }
}

uint8_t Z80_Core::ACIA_6850(uint8_t op, uint8_t operand) {
    /* OPERATIONS:
        0 - Receive data
        1 - Transmit data
        2 - Read status register
        3 - Write control register
        4 - Check if input is available
    */

    /* STATUS REGISTER:
        Bit 0 - RDRF (Read data register full)
        Bit 1 - TDRE (Transmit data register empty)
        Bit 2 - DCD (Data carrier detect)
        Bit 3 - CTS (Clear to send)
        Bit 4 - FE (Framing error)
        Bit 5 - RO (Receiver overrun)
        Bit 6 - PE (Parity error)
        Bit 7 - IRQ
    */

    /* CONTROL REGISTER:
        Bit 0-1 - Counter divider select (CDS1, CDS2)
        Bit 2-4 - Word select (WS1, WS2, WS3)
        Bit 5-6 - Transmit control (TC1, TC2)
        Bit 7   - RIE (Receive interrupt enable)
    */

    char ch;
    struct termios oldt, newt;
    int flags;
    ssize_t bytesRead;

    switch (op) {
        case 0: // Receive data
            if (ACIA_status & 0x01) {
                ACIA_status &= ~0x01;
                return ACIA_RDR;
            }
            return 0;

        case 1: // Transmit data
            ACIA_status &= ~0x02;
            printf("%c", operand);
            fflush(stdout);
            return 0;

        case 2: // Read status register
            return ACIA_status;

        case 3: // Write control register
            ACIA_control = operand;
            return 0;

        case 4: // Check if input is available
            tcgetattr(STDIN_FILENO, &oldt); // Save terminal settings
            newt = oldt;
            newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);

            flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK); // Set non-blocking mode

            bytesRead = read(STDIN_FILENO, &ch, 1); // Try to read input
            if (bytesRead > 0) {
                ACIA_RDR = ch;
                ACIA_status |= 0x01;
                if (ACIA_control & 0x80) {
                    interruptHandler();
                }
            } else {
                ACIA_status &= ~0x01;
            }

            fcntl(STDIN_FILENO, F_SETFL, flags);
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            return (bytesRead > 0) ? 1 : 0;

        default:
            break;
    }

    return 0;
}

uint8_t Z80_Core::ACIA_6850_Handler() {
    /* OPERATIONS:
        0 - Receive data
        1 - Transmit data
        2 - Read status register
        3 - Write control register
        4 - Check if input is available
    */

    /* STATUS REGISTER:
        Bit 0 - RDRF (Read data register full)
        Bit 1 - TDRE (Transmit data register empty)
        Bit 2 - DCD (Data carrier detect)
        Bit 3 - CTS (Clear to send)
        Bit 4 - FE (Framing error)
        Bit 5 - RO (Receiver overrun)
        Bit 6 - PE (Parity error)
        Bit 7 - IRQ
    */

    /* CONTROL REGISTER:
        Bit 0-1 - Counter divider select (CDS1, CDS2)
        Bit 2-4 - Word select (WS1, WS2, WS3)
        Bit 5-6 - Transmit control (TC1, TC2)
        Bit 7   - RIE (Receive interrupt enable)
    */

    char ch;
    struct termios oldt, newt;
    int flags;
    ssize_t bytesRead;

    tcgetattr(STDIN_FILENO, &oldt); // Save terminal settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK); // Set non-blocking mode

    bytesRead = read(STDIN_FILENO, &ch, 1); // Try to read input
    if (bytesRead > 0) {
        ACIA_RDR = ch;
        ACIA_status |= 0x01;
        if (ACIA_control & 0x80) {
            //cout << "Interrupt pending, iff1: " << iff1 << " iff2: " << iff2  << endl;
            isPending = true;
        } else {
            // Do nothing
        }
    } else {
        isPending = false;
    }

    fcntl(STDIN_FILENO, F_SETFL, flags);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return (bytesRead > 0) ? 1 : 0;
}


uint8_t Z80_Core::inputHandler(uint8_t port) {
    uint8_t input = 0;

    if (port == 0x00){ // Stdin
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);  // Save the terminal settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Apply new settings

        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);  // Set non-blocking

        char ch;
        ssize_t bytesRead = read(STDIN_FILENO, &ch, 1);  // Attempt to read input
        if (bytesRead > 0) {
            input = static_cast<uint8_t>(ch);
        }

        fcntl(STDIN_FILENO, F_SETFL, flags);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
    if (port == 0x80){ // ACIA 6850 Status Register
        input = ACIA_6850(ACIA_READ_STATUS, 0);
    }
    if (port == 0x81){ // ACIA 6850 Data Register
        input = ACIA_6850(ACIA_RECIEVE, 0);
    }
    return input;
}

uint8_t Z80_Core::outputHandler(uint8_t &reg, uint8_t port) {
    switch (port) {
        case 0x00: // stdout
            printf("%c", reg);
            break;
        case 0x01: // debug
            if (DEBUG) {
                printInfo();
            }
            break;
        case 0x80: // ACIA 6850 Control Register
            ACIA_6850(ACIA_WRITE_CONTROL, reg);
            break;
        case 0x81: // ACIA 6850 Data Register
            ACIA_6850(ACIA_TRANSMIT, reg);
            break;
        default:
            break;
    }
    return reg;
}

void Z80_Core::interruptHandler() {
    if (iff1 == 0) return;
    iff1, iff2 = 0;
    switch (im){
        case 0: // TODO
            break;
        case 1: // RST 38H
            push(pc);
            pc = 0x38;
            isPending = false;
            break;
        case 2: // TODO
            break;
    }
}

void Z80_Core::printCurrentState() {
    cout << "PC: " << pc << " SP: " << sp << " F: " << bitset<8>(f) << endl;
    cout << "IX: " << ix << " IY: " << iy << endl;
    cout << "ACC: " << acc << endl;
    cout << "AF: " << unsigned(a) << unsigned(f) << " BC: " << unsigned(b) << unsigned(c) << " DE: " << unsigned(d) << unsigned(e) << " HL: " << unsigned(h) << unsigned(l) << endl;
}

void Z80_Core::printInfo() {
    cout << "PC: 0x" << hex << pc << " SP: 0x" << sp << " F: 0x" << bitset<8>(f) << endl;
    cout << "IX: 0x" << ix << " IY: 0x" << iy << endl;
    cout << "A: 0x" << hex << unsigned(a) << " BC: 0x" << unsigned(b) << unsigned(c) << " DE: 0x" << unsigned(d) << unsigned(e) << " HL: 0x" << unsigned(h) << unsigned(l) << endl;
}

void Z80_Core::testAlu(uint8_t& reg, uint8_t reg2, uint8_t ins) {
    alu((uint16_t&)reg, reg2, ins);
}

void Z80_Core::alu(uint16_t& op1, uint16_t op2, uint8_t ins) {

    /* 
    OP1 is the destination register
    OP2 is the source register/immediate
    INS is the ALU operation
    
    ALU instructions
        0x00, 8-bit ADD reg, reg/n
        0x01, 16-bit ADD reg, reg/nn
        0x02, 8-bit ADC reg, reg/n
        0x03, 16-bit ADC reg, reg/nn
        0x04, 8-bit SUB reg, reg/n
        0x05, 16-bit SUB reg, reg/nn
        0x06, 8-bit SBC reg, reg/n
        0x07, 16-bit SBC reg, reg/nn
        0x08, 8-bit AND reg, reg/n
        0x09, 8-bit OR reg, reg/n
        0x0A, 8-bit XOR reg, reg/n
        0x0B, 8-bit CP reg, reg/n
        0x0C, 8-bit INC reg
        0x0D, 8-bit DEC reg
        0x0E, RLC reg
        0x0F, RL reg
        0x10, RRC reg
        0x11, RR reg
        0x12, SLA reg
        0x13, SRA reg
        0x14, SRL reg
        0x15, BIT 0, reg
        0x16, BIT 1, reg
        0x17, BIT 2, reg
        0x18, BIT 3, reg
        0x19, BIT 4, reg
        0x1A, BIT 5, reg
        0x1B, BIT 6, reg
        0x1C, BIT 7, reg
        0x1D, RES 0, reg
        0x1E, RES 1, reg
        0x1F, RES 2, reg
        0x20, RES 3, reg
        0x21, RES 4, reg
        0x22, RES 5, reg
        0x23, RES 6, reg
        0x24, RES 7, reg
        0x25, SET 0, reg
        0x26, SET 1, reg
        0x27, SET 2, reg
        0x28, SET 3, reg
        0x29, SET 4, reg
        0x2A, SET 5, reg
        0x2B, SET 6, reg
        0x2C, SET 7, reg
        0x2D, 16-bit INC reg
        0x2E, 16-bit DEC reg
    */
   
    bool isCompare = false;
    bool bitShort = false;
    int result = 0;
    bool carry_in = false;
    
    auto updateFlags8bit = [&](int val) {
        // Zero flag
        f = (val == 0) ? (f | FLAG_Z) : (f & ~FLAG_Z);
        // Carry flag
        f = (val & 0x100) ? (f | FLAG_C) : (f & ~FLAG_C);
        // Half-carry flag
        f = ((op1 & 0xF) + (op2 & 0xF)) > 0xF ? (f | FLAG_H) : (f & ~FLAG_H);
        // Negative flag
        f = (val < 0) ? (f | FLAG_N) : (f & ~FLAG_N);
    };

    switch (ins) {
        case 0x00: // ADD8 reg, reg/n
            result = (op1 & 0xFF) + (op2 & 0xFF);
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            break;
        case 0x01: // ADD16 reg, reg/nn
            result = op1 + op2;
            bitShort = true;
            op1 = result;
            updateFlags8bit(result);
            break;
        case 0x02: // ADC8 reg, reg/n
            result = (op1 & 0xFF) + (op2 & 0xFF) + (f & FLAG_C);
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            break;
        case 0x03: // ADC16 reg, reg/nn
            result = op1 + op2 + (f & FLAG_C);
            bitShort = true;
            op1 = result;
            updateFlags8bit(result);
            break;
        case 0x04: // SUB8 reg, reg/n
            result = (op1 & 0xFF) - (op2 & 0xFF);
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            break;
        case 0x05: // SUB16 reg, reg/nn
            result = op1 - op2;
            bitShort = true;
            op1 = result;
            updateFlags8bit(result);
            break;
        case 0x06: // SBC8 reg, reg/n
            result = (op1 & 0xFF) - (op2 & 0xFF) - (f & FLAG_C);
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            break;
        case 0x07: // SBC16 reg, reg/nn
            result = op1 - op2 - (f & FLAG_C);
            bitShort = true;
            op1 = result;
            updateFlags8bit(result);
            break;
        case 0x08: // AND reg, reg/n
            result = (op1 & 0xFF) & (op2 & 0xFF);
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            f |= FLAG_H; // Always set the half-carry flag for AND operations
            break;
        case 0x09: // OR reg, reg/n
            result = (op1 & 0xFF) | (op2 & 0xFF);
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            break;
        case 0x0A: // XOR reg, reg/n
            result = (op1 & 0xFF) ^ (op2 & 0xFF);
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            break;
        case 0x0B: // CP reg, reg/n
            result = (op1 & 0xFF) - (op2 & 0xFF);
            isCompare = true;
            updateFlags8bit(result);
            break;
        case 0x0C: // INC reg
            result = (op1 & 0xFF) + 1;
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            break;
        case 0x0D: // DEC reg
            result = (op1 & 0xFF) - 1;
            op1 = (op1 & 0xFF00) | (result & 0xFF);
            updateFlags8bit(result);
            break;
        // Rotation and shifting
        case 0x0E: // RLC reg
        case 0x0F: // RL reg
        case 0x10: // RRC reg
        case 0x11: // RR reg
        case 0x12: // SLA reg
        case 0x13: // SRA reg
        case 0x14: // SRL reg
            op1 &= 0xFF; // clear upper 8 bits
            carry_in = (f & FLAG_C);
            switch (ins) {
                case 0x0E: // RLC
                    f = (op1 & 0x80) ? (f | FLAG_C) : (f & ~FLAG_C);
                    op1 = (op1 << 1) | (op1 >> 7);
                    break;
                case 0x0F: // RL
                    f = (op1 & 0x80) ? (f | FLAG_C) : (f & ~FLAG_C);
                    op1 = (op1 << 1) | carry_in;
                    break;
                case 0x10: // RRC
                    f = (op1 & 0x01) ? (f | FLAG_C) : (f & ~FLAG_C);
                    op1 = (op1 >> 1) | (op1 << 7);
                    break;
                case 0x11: // RR
                    f = (op1 & 0x01) ? (f | FLAG_C) : (f & ~FLAG_C);
                    op1 = (op1 >> 1) | (carry_in << 7);
                    break;
                case 0x12: // SLA
                    f = (op1 & 0x80) ? (f | FLAG_C) : (f & ~FLAG_C);
                    op1 = (op1 << 1);
                    break;
                case 0x13: // SRA
                    f = (op1 & 0x01) ? (f | FLAG_C) : (f & ~FLAG_C);
                    op1 = (op1 >> 1) | (op1 & 0x80);
                    break;
                case 0x14: // SRL
                    f = (op1 & 0x01) ? (f | FLAG_C) : (f & ~FLAG_C);
                    op1 = (op1 >> 1);
                    break;
            }
            f = (op1 == 0) ? (f | FLAG_Z) : (f & ~FLAG_Z); // Set zero flag
            break;

        case 0x15: // BIT 0, reg
        case 0x16: // BIT 1, reg
        case 0x17: // BIT 2, reg
        case 0x18: // BIT 3, reg
        case 0x19: // BIT 4, reg
        case 0x1A: // BIT 5, reg
        case 0x1B: // BIT 6, reg
        case 0x1C: // BIT 7, reg
            f = (op1 & (1 << (ins - 0x15))) ? (f | FLAG_Z) : (f & ~FLAG_Z);
            f |= FLAG_H;
            break;

        case 0x1D: // RES 0, reg
        case 0x1E: // RES 1, reg
        case 0x1F: // RES 2, reg
        case 0x20: // RES 3, reg
        case 0x21: // RES 4, reg
        case 0x22: // RES 5, reg
        case 0x23: // RES 6, reg
        case 0x24: // RES 7, reg
            op1 &= ~(1 << (ins - 0x20));
            break;

        case 0x25: // SET 0, reg
        case 0x26: // SET 1, reg
        case 0x27: // SET 2, reg
        case 0x28: // SET 3, reg
        case 0x29: // SET 4, reg
        case 0x2A: // SET 5, reg
        case 0x2B: // SET 6, reg
        case 0x2C: // SET 7, reg
            op1 |= (1 << (ins - 0x28));
            break;
        
        case 0x2D: // 16-bit INC reg
            op1++;
            break;
        case 0x2E: // 16-bit DEC reg
            op1--;
            break;
    }
}


uint8_t Z80_Core::fetchOperand() { // fetch operand
    uint8_t operand = memory[pc];
    pc++;
    return operand;
}

void Z80_Core::fetchInstruction() {
    ins = memory[pc];
    pc++;

    //cout << "INS: " << ins << endl; // for debugging
}

void Z80_Core::swapRegs(uint8_t& temp1, uint8_t& temp2) {
    uint8_t temp3 = temp1;
    temp1 = temp2;
    temp2 = temp3;
}

uint16_t Z80_Core::convToRegPair(uint8_t l, uint8_t h) {
    return (uint16_t)l | (h << 8);
}

void Z80_Core::incRegPair(uint8_t& l, uint8_t& h) {
    if (l == 0xFF) {
        l = 0;
        h++;
    } else {
        l++;
    }
}

void Z80_Core::decRegPair(uint8_t& l, uint8_t& h) {
    if (h == 0) {
        l--;
    } else {
        h--;
    }
}

void Z80_Core::push(uint16_t reg){
    //cout << "SP: " << hex << (unsigned)sp << endl;
    sp--;
    memory[sp] = (reg >> 8); // high byte
    sp--;
    memory[sp] = reg & 0xFF; // low byte
    //cout << "Pushed: " << hex << (unsigned)reg << endl;
}

void Z80_Core::push_reg_pair(uint8_t l, uint8_t h) {
    push(convToRegPair(l, h));
}

uint16_t Z80_Core::pop() {
    //cout << "SP: " << hex << (unsigned)sp << endl;
    uint16_t reg = memory[sp];
    sp++;
    reg |= memory[sp] << 8;
    sp++;
    return reg;
}

void Z80_Core::pop_reg_pair(uint8_t& l, uint8_t& h) {
    uint16_t reg = pop();
    h = reg >> 8;
    l = reg & 0xFF;
}

void Z80_Core::decode_execute(uint8_t instruction) {
    int8_t raddr = 0; // relative address, used for relative jumps
    uint16_t temp, temp2 = 0;
    if (nop_watchdog > 10 && !disableWatchdog) { // prevent infinite loops, can be adjusted or disabled
        cout << "Infinite loop detected at address: " << hex << pc << endl;
        exit(0);
    }
    ExecutedInstructions.push_back(instruction);
    if(DEBUG) cout << "PC: " << hex << (unsigned)pc << "  INS: " << (unsigned)instruction << " " << Opcodes[instruction] << endl;
    switch (instruction) {
        case 0x00: // NOP
            if(ExecutedInstructions[pc-1] == 0x00) nop_watchdog++;
            break;
        case 0x01: // LD BC, nn
            c = fetchOperand();
            b = fetchOperand();
            break;
        case 0x02: // LD (BC), A
            memory[c | (b << 8)] = a;
            break;
        case 0x03: // INC BC
            if (c == 0xFF) {
                c = 0;
                b++;
            } else {
                c++;
            }
            break;
        case 0x04: // INC B
            alu((uint16_t&)b, (uint16_t&)w, ALU_INC8);
            break;
        case 0x05: // DEC B
            alu((uint16_t&)b, 0, ALU_DEC8);
            break;
        case 0x06: // LD B, n
            b = fetchOperand();
            break;
        case 0x07: // RLCA
            alu((uint16_t&)a, (uint16_t&)f, ALU_RLC8);
            break;
        case 0x08: // EX AF, AF'
            w = (uint8_t)(afa << 8);
            swapRegs(a, w);
            z = (uint8_t)(afa & 0xff);
            swapRegs(f, z);
            break;
        case 0x09: // ADD HL, BC
            temp = (uint16_t&)l | (h << 8);
            temp2 = (uint16_t&)c | (b << 8);
            alu(temp,temp2, ALU_ADC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x0A: // LD A, (BC)
            a = memory[c | (b << 8)];
            break;
        case 0x0B: // DEC BC
            if (b == 0) {
                c--;
            } else {
                b--;
            }
            break;
        case 0x0C: // INC C
            alu((uint16_t&)c, (uint16_t&)w, ALU_INC8);
            break;
        case 0x0D: // DEC C
            alu((uint16_t&)c, (uint16_t&)w, ALU_DEC8);
            break;
        case 0x0E: // LD C, n
            c = fetchOperand();
            break;
        case 0x0F: // RRCA
            alu((uint16_t&)a, (uint16_t&)f, ALU_RRC8);
            break;
        case 0x10: // DJNZ d
            b--;
            if (b != 0) {
                pc = pc + fetchOperand();
            }
            break;
        case 0x11: // LD DE, nn
            e = fetchOperand();
            d = fetchOperand();
            break;
        case 0x12: // LD (DE), A
            memory[e | (d << 8)] = a;
            break;
        case 0x13: // INC DE
            if (e == 0xFF) {
                e = 0;
                d++;
            } else {
                e++;
            }
            break;
        case 0x14: // INC D
            alu((uint16_t&)d, (uint16_t&)w, ALU_INC8);
            break;
        case 0x15: // DEC D
            alu((uint16_t&)d, (uint16_t&)w, ALU_DEC8);
            break;
        case 0x16: // LD D, n
            d = fetchOperand();
            break;
        case 0x17: // RLA
            alu((uint16_t&)a, (uint16_t&)f, ALU_RL8);
            break;
        case 0x18: // JR n
            raddr = (int8_t)fetchOperand();
            pc = pc + raddr;
            break;
        case 0x19: // ADD HL, DE
            temp = (uint16_t&)l | (h << 8);
            temp2 = (uint16_t&)e | (d << 8);
            alu(temp,temp2, ALU_ADC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x1A: // LD A, (DE)
            a = memory[e | (d << 8)];
            break;
        case 0x1B: // DEC DE
            if (d == 0) {
                e--;
            } else {
                d--;
            }
            break;
        case 0x1C: // INC E
            alu((uint16_t&)e, (uint16_t&)w, ALU_INC8);
            break;
        case 0x1D: // DEC E
            alu((uint16_t&)e, (uint16_t&)w, ALU_DEC8);
            break;
        case 0x1E: // LD E, n
            e = fetchOperand();
            break;
        case 0x1F: // RRA
            alu((uint16_t&)a, (uint16_t&)f, ALU_RR8);
            break;
        case 0x20: // JR NZ, n
            raddr = (int8_t)fetchOperand();
            if (!(f & FLAG_Z)){
                pc = pc + raddr;
            }
            break;
        case 0x21: // LD HL, nn
            l = fetchOperand();
            h = fetchOperand();
            break;
        case 0x22: // LD (nn), HL
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            memory[w | (z << 8)] = l;
            memory[w | (z << 8) + 1] = h;
            break;
        case 0x23: // INC HL
            if (l == 0xFF) {
                l = 0;
                h++;
            } else {
                l++;
            }
            break;
        case 0x24: // INC H
            alu((uint16_t&)h, (uint16_t&)w, ALU_INC8);
            break;
        case 0x25: // DEC H
            alu((uint16_t&)h, (uint16_t&)w, ALU_DEC8);
            break;
        case 0x26: // LD H, n
            h = fetchOperand();
            break;
        case 0x27: // DAA
            //TODO
            break;
        case 0x28: // JR Z, n
            raddr = (int8_t)fetchOperand();
            if (f & FLAG_Z) {
                pc = pc + raddr;
            }
            break;
        case 0x29: // ADD HL, HL
            temp = (uint16_t&)l | (h << 8);
            temp2 = (uint16_t&)l | (h << 8);
            alu(temp,temp2, ALU_ADC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x2A: // LD HL, (nn)
            w = fetchOperand();
            z = fetchOperand();
            l = memory[w | (z << 8)];
            h = memory[w | (z << 8) + 1];
            break;
        case 0x2B: // DEC HL
            if (h == 0) {
                l--;
            } else {
                h--;
            }
            break;
        case 0x2C: // INC L
            alu((uint16_t&)l, (uint16_t&)w, ALU_INC8);
            break;
        case 0x2D: // DEC L
            alu((uint16_t&)l, (uint16_t&)w, ALU_DEC8);
            break;
        case 0x2E: // LD L, n
            l = fetchOperand();
            break;
        case 0x2F: // CPL
            a = ~a + 1;
            f ^= (1 << 4) | (1 << 1);
            break;
        case 0x30: // JR NC, n
            raddr = (int8_t)fetchOperand();
            if (!(f & FLAG_C)) {
                pc = pc + raddr;
            }
            break;
        case 0x31: // LD SP, nn
            sp = fetchOperand() | (fetchOperand() << 8);
            break;
        case 0x32: // LD (nn), A
            memory[fetchOperand() | (fetchOperand() << 8)] = a;
            break;
        case 0x33: // INC SP
            sp++;
            break;
        case 0x34: // INC (HL)
            alu((uint16_t&)memory[l | (h << 8)], (uint16_t&)w, ALU_INC8);
            //memory[l | (h << 8)]++;
            break;
        case 0x35: // DEC (HL)
            alu((uint16_t&)memory[l | (h << 8)], (uint16_t&)w, ALU_DEC8);
            //memory[l | (h << 8)]--;
            break;
        case 0x36: // LD (HL), n
            memory[l | (h << 8)] = fetchOperand();
            break;
        case 0x37: // SCF
            f |= 1;
            break;
        case 0x38: // JR C, n
            raddr = (int8_t)fetchOperand();
            if (f & FLAG_C) {
                pc = pc + raddr;
            }
            break;
        case 0x39: // ADD HL, SP
            acc = (h << 8 | l) + sp;
            if (acc > 65535) {
                f |= 0x01;
            }
            h = acc >> 8;
            l = acc & 0xff;
            break;
        case 0x3A: // LD A, (nn)
            a = memory[fetchOperand() | (fetchOperand() << 8)];
            break;
        case 0x3B: // DEC SP
            sp--;
            break;
        case 0x3C: // INC A
            alu((uint16_t&)a, (uint16_t&)a, ALU_INC8);
            break;
        case 0x3D: // DEC A
            alu((uint16_t&)a, (uint16_t&)a, ALU_DEC8);
            break;
        case 0x3E: // LD A, n
            a = fetchOperand();
            break;
        case 0x3F: // CCF, invert carry flag
            f |= !(f & FLAG_C);
            break;
        case 0x40: // LD B, B
            b = b;
            break;
        case 0x41: // LD B, C
            b = c;
            break;
        case 0x42: // LD B, D
            b = d;
            break;
        case 0x43: // LD B, E
            b = e;
            break;
        case 0x44: // LD B, H
            b = h;
            break;
        case 0x45: // LD B, L
            b = l;
            break;
        case 0x46: // LD B, (HL)
            b = memory[l | (h << 8)];
            break;
        case 0x47: // LD B, A
            b = a;
            break;
        case 0x48: // LD C, B
            c = b;
            break;
        case 0x49: // LD C, C
            c = c;
            break;
        case 0x4A: // LD C, D
            c = d;
            break;
        case 0x4B: // LD C, E
            c = e;
            break;
        case 0x4C: // LD C, H
            c = h;
            break;
        case 0x4D: // LD C, L
            c = l;
            break;
        case 0x4E: // LD C, (HL)
            c = memory[l | (h << 8)];
            break;
        case 0x4F: // LD C, A
            c = a;
            break;
        case 0x50: // LD D, B
            d = b;
            break;
        case 0x51: // LD D, C
            d = c;
            break;
        case 0x52: // LD D, D
            d = d;
            break;
        case 0x53: // LD D, E
            d = e;
            break;
        case 0x54: // LD D, H
            d = h;
            break;
        case 0x55: // LD D, L
            d = l;
            break;
        case 0x56: // LD D, (HL)
            d = memory[l | (h << 8)];
            break;
        case 0x57: // LD D, A
            d = a;
            break;
        case 0x58: // LD E, B
            e = b;
            break;
        case 0x59: // LD E, C
            e = c;
            break;
        case 0x5A: // LD E, D
            e = d;
            break;
        case 0x5B: // LD E, E
            e = e;
            break;
        case 0x5C: // LD E, H
            e = h;
            break;
        case 0x5D: // LD E, L
            e = l;
            break;
        case 0x5E: // LD E, (HL)
            e = memory[l | (h << 8)];
            break;
        case 0x5F: // LD E, A
            e = a;
            break;
        case 0x60: // LD H, B
            h = b;
            break;
        case 0x61: // LD H, C
            h = c;
            break;
        case 0x62: // LD H, D
            h = d;
            break;
        case 0x63: // LD H, E
            h = e;
            break;
        case 0x64: // LD H, H
            h = h;
            break;
        case 0x65: // LD H, L
            h = l;
            break;
        case 0x66: // LD H, (HL)
            h = memory[l | (h << 8)];
            break;
        case 0x67: // LD H, A
            h = a;
            break;
        case 0x68: // LD L, B
            l = b;
            break;
        case 0x69: // LD L, C
            l = c;
            break;
        case 0x6A: // LD L, D
            l = d;
            break;
        case 0x6B: // LD L, E
            l = e;
            break;
        case 0x6C: // LD L, H
            l = h;
            break;
        case 0x6D: // LD L, L
            l = l;
            break;
        case 0x6E: // LD L, (HL)
            l = memory[l | (h << 8)];
            break;
        case 0x6F: // LD L, A
            l = a;
            break;
        case 0x70: // LD (HL), B
            memory[l | (h << 8)] = b;
            break;
        case 0x71: // LD (HL), C
            memory[l | (h << 8)] = c;
            break;
        case 0x72: // LD (HL), D
            memory[l | (h << 8)] = d;
            break;
        case 0x73: // LD (HL), E
            memory[l | (h << 8)] = e;
            break;
        case 0x74: // LD (HL), H
            memory[l | (h << 8)] = h;
            break;
        case 0x75: // LD (HL), L
            memory[l | (h << 8)] = l;
            break;
        case 0x76: // HALT
            halt = true;
            break;
        case 0x77: // LD (HL), A
            memory[l | (h << 8)] = a;
            break;
        case 0x78: // LD A, B
            a = b;
            break;
        case 0x79: // LD A, C
            a = c;
            break;
        case 0x7A: // LD A, D
            a = d;
            break;
        case 0x7B: // LD A, E
            a = e;
            break;
        case 0x7C: // LD A, H
            a = h;
            break;
        case 0x7D: // LD A, L
            a = l;
            break;
        case 0x7E: // LD A, (HL)
            a = memory[l | (h << 8)];
            break;
        case 0x7F: // LD A, A
            a = a;
            break;
        case 0x80: // ADD A, B
            alu((uint16_t&)a, (uint16_t&)b, ALU_ADD8);
            break;
        case 0x81: // ADD A, C
            alu((uint16_t&)a, (uint16_t&)c, ALU_ADD8);
            break;
        case 0x82: // ADD A, D
            alu((uint16_t&)a, (uint16_t&)d, ALU_ADD8);
            break;
        case 0x83: // ADD A, E
            alu((uint16_t&)a, (uint16_t&)e, ALU_ADD8);
            break;
        case 0x84: // ADD A, H
            alu((uint16_t&)a, (uint16_t&)h, ALU_ADD8);
            break;
        case 0x85: // ADD A, L
            alu((uint16_t&)a, (uint16_t&)l, ALU_ADD8);
            break;
        case 0x86: // ADD A, (HL)
            alu((uint16_t&)a, (uint16_t&)memory[l | (h << 8)], ALU_ADD8);
            break;
        case 0x87: // ADD A, A
            alu((uint16_t&)a, (uint16_t&)a, ALU_ADD8);
            break;
        case 0x88: // ADC A, B
            alu((uint16_t&)a, (uint16_t&)b, ALU_ADC8);
            break;
        case 0x89: // ADC A, C
            alu((uint16_t&)a, (uint16_t&)c, ALU_ADC8);
            break;
        case 0x8A: // ADC A, D
            alu((uint16_t&)a, (uint16_t&)d, ALU_ADC8);
            break;
        case 0x8B: // ADC A, E
            alu((uint16_t&)a, (uint16_t&)e, ALU_ADC8);
            break;
        case 0x8C: // ADC A, H
            alu((uint16_t&)a, (uint16_t&)h, ALU_ADC8);
            break;
        case 0x8D: // ADC A, L
            alu((uint16_t&)a, (uint16_t&)l, ALU_ADC8);
            break;
        case 0x8E: // ADC A, (HL)
            alu((uint16_t&)a, (uint16_t&)memory[l | (h << 8)], ALU_ADC8);
            break;
        case 0x8F: // ADC A, A
            alu((uint16_t&)a, (uint16_t&)a, ALU_ADC8);
            break;
        case 0x90: // SUB B
            alu((uint16_t&)a, (uint16_t&)b, ALU_SUB8);
            break;
        case 0x91: // SUB C
            alu((uint16_t&)a, (uint16_t&)c, ALU_SUB8);
            break;
        case 0x92: // SUB D
            alu((uint16_t&)a, (uint16_t&)d, ALU_SUB8);
            break;
        case 0x93: // SUB E
            alu((uint16_t&)a, (uint16_t&)e, ALU_SUB8);
            break;
        case 0x94: // SUB H
            alu((uint16_t&)a, (uint16_t&)h, ALU_SUB8);
            break;
        case 0x95: // SUB L
            alu((uint16_t&)a, (uint16_t&)l, ALU_SUB8);
            break;
        case 0x96: // SUB (HL)
            alu((uint16_t&)a, (uint16_t&)memory[l | (h << 8)], ALU_SUB8);
            break;
        case 0x97: // SUB A
            alu((uint16_t&)a, (uint16_t&)a, ALU_SUB8);
            break;
        case 0x98: // SBC A, B
            alu((uint16_t&)a, (uint16_t&)b, ALU_SBC8);
            break;
        case 0x99: // SBC A, C
            alu((uint16_t&)a, (uint16_t&)c, ALU_SBC8);
            break;
        case 0x9A: // SBC A, D
            alu((uint16_t&)a, (uint16_t&)d, ALU_SBC8);
            break;
        case 0x9B: // SBC A, E
            alu((uint16_t&)a, (uint16_t&)e, ALU_SBC8);
            break;
        case 0x9C: // SBC A, H
            alu((uint16_t&)a, (uint16_t&)h, ALU_SBC8);
            break;
        case 0x9D: // SBC A, L
            alu((uint16_t&)a, (uint16_t&)l, ALU_SBC8);
            break;
        case 0x9E: // SBC A, (HL)
            alu((uint16_t&)a, (uint16_t&)memory[l | (h << 8)], ALU_SBC8);
            break;
        case 0x9F: // SBC A, A
            alu((uint16_t&)a, (uint16_t&)a, ALU_SBC8);
            break;
        case 0xA0: // AND B
            alu((uint16_t&)a, (uint16_t&)b, ALU_AND8);
            break;
        case 0xA1: // AND C
            alu((uint16_t&)a, (uint16_t&)c, ALU_AND8);
            break;
        case 0xA2: // AND D
            alu((uint16_t&)a, (uint16_t&)d, ALU_AND8);
            break;
        case 0xA3: // AND E
            alu((uint16_t&)a, (uint16_t&)e, ALU_AND8);
            break;
        case 0xA4: // AND H
            alu((uint16_t&)a, (uint16_t&)h, ALU_AND8);
            break;
        case 0xA5: // AND L
            alu((uint16_t&)a, (uint16_t&)l, ALU_AND8);
            break;
        case 0xA6: // AND (HL)
            alu((uint16_t&)a, (uint16_t&)memory[l | (h << 8)], ALU_AND8);
            break;
        case 0xA7: // AND A
            alu((uint16_t&)a, (uint16_t&)a, ALU_AND8);
            break;
        case 0xA8: // XOR B
            alu((uint16_t&)a, (uint16_t&)b, ALU_XOR8);
            break;
        case 0xA9: // XOR C
            alu((uint16_t&)a, (uint16_t&)c, ALU_XOR8);
            break;
        case 0xAA: // XOR D
            alu((uint16_t&)a, (uint16_t&)d, ALU_XOR8);
            break;
        case 0xAB: // XOR E
            alu((uint16_t&)a, (uint16_t&)e, ALU_XOR8);
            break;
        case 0xAC: // XOR H
            alu((uint16_t&)a, (uint16_t&)h, ALU_XOR8);
            break;
        case 0xAD: // XOR L
            alu((uint16_t&)a, (uint16_t&)l, ALU_XOR8);
            break;
        case 0xAE: // XOR (HL)
            alu((uint16_t&)a, (uint16_t&)memory[l | (h << 8)], ALU_XOR8);
            break;
        case 0xAF: // XOR A
            alu((uint16_t&)a, (uint16_t&)a, ALU_XOR8);
            break;
        case 0xB0: // OR B
            alu((uint16_t&)a, (uint16_t&)b, ALU_OR8);
            break;
        case 0xB1: // OR C
            alu((uint16_t&)a, (uint16_t&)c, ALU_OR8);
            break;
        case 0xB2: // OR D
            alu((uint16_t&)a, (uint16_t&)d, ALU_OR8);
            break;
        case 0xB3: // OR E
            alu((uint16_t&)a, (uint16_t&)e, ALU_OR8);
            break;
        case 0xB4: // OR H
            alu((uint16_t&)a, (uint16_t&)h, ALU_OR8);
            break;
        case 0xB5: // OR L
            alu((uint16_t&)a, (uint16_t&)l, ALU_OR8);
            break;
        case 0xB6: // OR (HL)
            alu((uint16_t&)a, (uint16_t&)memory[l | (h << 8)], ALU_OR8);
            break;
        case 0xB7: // OR A
            alu((uint16_t&)a, (uint16_t&)a, ALU_OR8);
            break;
        case 0xB8: // CP B
            alu((uint16_t&)a, (uint16_t&)b, ALU_CP8);
            break;
        case 0xB9: // CP C
            alu((uint16_t&)a, (uint16_t&)c, ALU_CP8);
            break;
        case 0xBA: // CP D
            alu((uint16_t&)a, (uint16_t&)d, ALU_CP8);
            break;
        case 0xBB: // CP E
            alu((uint16_t&)a, (uint16_t&)e, ALU_CP8);
            break;
        case 0xBC: // CP H
            alu((uint16_t&)a, (uint16_t&)h, ALU_CP8);
            break;
        case 0xBD: // CP L
            alu((uint16_t&)a, (uint16_t&)l, ALU_CP8);
            break;
        case 0xBE: // CP (HL)
            alu((uint16_t&)a, (uint16_t&)memory[l | (h << 8)], ALU_CP8);
            break;
        case 0xBF: // CP A
            alu((uint16_t&)a, (uint16_t&)a, ALU_CP8);
            break;
        case 0xC0: // RET NZ
            if (!(f & FLAG_Z)) {
                pc = pop();
            }
            //cout << "Returned PC by RET NZ: " << hex << (unsigned)pc << endl;
            break;
        case 0xC1: // POP BC
            pop_reg_pair(c, b);
            break;
        case 0xC2: // JP NZ, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if (!(f & FLAG_Z)) {
                pc = (w | (z << 8));
            }
            break;
        case 0xC3: // JP nn
            pc = (fetchOperand() | (fetchOperand() << 8));
            break;
        case 0xC4: // CALL NZ, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if (!(f & FLAG_Z)) {
                push(pc);
                pc = (w | (z << 8));
            }
            break;
        case 0xC5: // PUSH BC
            push_reg_pair(c, b);
            break;
        case 0xC6: // ADD A, n
            w = fetchOperand();
            alu((uint16_t&)a, (uint16_t&)w, ALU_ADD8);
            break;
        case 0xC7: // RST 00h
            push(pc);
            pc = 0x00;
            break;
        case 0xC8: // RET Z
            if(f & FLAG_Z){
                pc = pop();
            }
            break;
        case 0xC9: // RET
            //cout << "Stack:" << hex << (unsigned)memory[sp+1] << (unsigned)memory[sp] << endl;
            pc = pop();
            //cout << "Popped PC: " << hex << (unsigned)pc << endl;
            break;
        case 0xCA: // JP Z, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(f & FLAG_Z){
                pc = (w | (z << 8));
            }
            break;
        case 0xCB: //BIT INSTRUCTION
            w = fetchOperand();
            ed_instruction(w);
            break;
        case 0xCC: // CALL Z, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(f & FLAG_Z){
                push(pc);
                pc = (w | (z << 8));
            }
            break;
        case 0xCD: // CALL nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            push(pc);
            //cout << "Saving PC: " << hex << (unsigned)pc << endl;
            //cout << "Saved PC: " << hex << (unsigned)memory[sp+1] << (unsigned)memory[sp] << endl;
            pc = (w | (z << 8));
            break;
        case 0xCE: // ADC A, n
            w = fetchOperand();
            alu((uint16_t&)a, (uint16_t&)w, ALU_ADC8);
            break;
        case 0xCF: // RST 08h
            push(pc);
            pc = 0x08;
            break;
        case 0xD0: // RET NC
            if(!(f & FLAG_C)){
                pc = pop();
            }
            break;
        case 0xD1: // POP DE
            pop_reg_pair(e, d);
            break;
        case 0xD2: // JP NC, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(!(f & FLAG_C)){
                pc = (w | (z << 8));
            }
            break;
        case 0xD3: // OUT (n), A
            w = fetchOperand();
            outputHandler(a,w);
            break;
        case 0xD4: // CALL NC, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(!(f & FLAG_C)){
                push(pc);
                pc = (w | (z << 8));
            }
            break;
        case 0xD5: // PUSH DE
            push_reg_pair(e, d);
            break;
        case 0xD6: // SUB n
            w = fetchOperand();
            alu((uint16_t&)a, (uint16_t&)w, ALU_SUB8);
            break;
        case 0xD7: // RST 10h
            push(pc);
            pc = 0x10;
            break;
        case 0xD8: // RET C
            if(f & FLAG_C){
                pc = pop();
            }
            break;
        case 0xD9: // EXX
            w = (uint8_t)(bca >> 8);
            swapRegs(b, w);
            w = (uint8_t)(bca & 0xFF);
            swapRegs(c, w);
            w = (uint8_t)(dea >> 8);
            swapRegs(d, w);
            w = (uint8_t)(dea & 0xFF);
            swapRegs(e, w);
            w = (uint8_t)(hla >> 8);
            swapRegs(h, w);
            w = (uint8_t)(hla & 0xFF);
            swapRegs(l, w);
            break;
        case 0xDA: // JP C, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(f & FLAG_C){
                pc = (w | (z << 8));
            }
            break;
        case 0xDB: // IN A, (n)
            w = fetchOperand();
            a = inputHandler(w);
            break;
        case 0xDC: // CALL C, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(f & FLAG_C){
                push(pc);
                pc = (w | (z << 8));
            }
            break;
        case 0xDD: // IX PREFIX
            w = fetchOperand();
            dd_instruction(w);
            break;
        case 0xDE: // SBC A, n
            w = fetchOperand();
            alu((uint16_t&)a, (uint16_t&)w, ALU_SBC8);
            break;
        case 0xDF: // RST 18h
            push(pc);
            pc = 0x18;
            break;
        case 0xE0: // RET PO
            if(!(f & 0x04)){
                pc = pop();
            }
            break;
        case 0xE1: // POP HL
            pop_reg_pair(l, h);
            break;
        case 0xE2: // JP PO, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(!(f & 0x04)){
                pc = (w | (z << 8));
            }
            break;
        case 0xE3: // EX (SP), HL
            w = memory[sp];
            memory[sp] = l;
            l = w;
            sp++;
            z = memory[sp];
            sp++;
            memory[sp] = h;
            h = z;
            break;
        case 0xE4: // CALL PO, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(!(f & 0x04)){
                push(pc);
                pc = (w | (z << 8));
            }
            break;
        case 0xE5: // PUSH HL
            push_reg_pair(l,h);
            break;
        case 0xE6: // AND n
            w = fetchOperand();
            alu((uint16_t&)a, (uint16_t&)w, ALU_AND8);
            break;
        case 0xE7: // RST 20h
            push(pc);
            pc = 0x20;
            break;
        case 0xE8: // RET PE
            if(!(f & 0x04)){
                pc = pop();
            }
            break;
        case 0xE9: // JP (HL)
            pc = l | (h << 8);
            break;
        case 0xEA: // JP PE, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(!(f & 0x04)){
                pc = (w | (z << 8));
            }
            break;
        case 0xEB: // EX DE, HL
            w = (uint8_t)(dea >> 8);
            swapRegs(d, w);
            w = (uint8_t)(dea & 0xFF);
            swapRegs (e, w);
            w = (uint8_t)(hla >> 8);
            swapRegs (h, w);
            w = (uint8_t)(hla & 0xFF);
            swapRegs (l, w);
            break;
        case 0xEC: // CALL PE, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if((f & 0x04)){
                push(pc);
                pc = (w | (z << 8));
            }
            break;
        case 0xED: // ED PREFIX
            w = fetchOperand();
            ed_instruction(w); // call ed instruction
            break;
        case 0xEE: // XOR n
            w = fetchOperand();
            alu((uint16_t&)a, (uint16_t&)w, ALU_XOR8);
            break;
        case 0xEF: // RST 28h
            push(pc);
            pc = 0x28;
            break;
        case 0xF0: // RET P
            if(!(f & 0x80)){
                pc = pop();
            }
            break;
        case 0xF1: // POP AF
            pop_reg_pair(f, a);
            break;
        case 0xF2: // JP P, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(!(f & 0x80)){
                pc = (w | (z << 8));
            }
            break;
        case 0xF3: // DI
            iff1 = iff2 = false;
            break;
        case 0xF4: // CALL P, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if(!(f & 0x80)){
                push(pc);
                pc = (w | (z << 8));
            }
            break;
        case 0xF5: // PUSH AF
            push_reg_pair(f, a);
            break;
        case 0xF6: // OR n
            w = fetchOperand();
            alu((uint16_t&)a, (uint16_t&)w, ALU_OR8);
            break;
        case 0xF7: // RST 30h
            push(pc);
            pc = 0x30;
            break;
        case 0xF8: // RET M
            if(!(f & 0x80)){
                pc = pop();
            }
            break;
        case 0xF9: // LD SP, HL
            sp = l | (h << 8);
            break;
        case 0xFA: // JP M, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if((f & 0x80)){
                pc = (w | (z << 8));
            }
            break;
        case 0xFB: // EI
            iff1 = iff2 = 1;
            break;
        case 0xFC: // CALL M, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            if((f & 0x80)){
                push(pc);
                pc = (w | (z << 8));
            }
            break;
        case 0xFD: // IY PREFIX
            w = fetchOperand();
            fd_instruction(w);
            break;
        case 0xFE: // CP n
            w = fetchOperand();
            alu((uint16_t&)a, (uint16_t&)w, ALU_CP8);
            break;
        case 0xFF: // RST 38h
            push(pc);
            pc = 0x38;
            break;
        default:
            cout << "Invalid MAIN instruction: " << hex << (int)ins << endl;
            break;

    }
}

void Z80_Core::ed_instruction(uint8_t ins) {
    uint16_t temp, temp2 = 0;
    switch (ins) {
        case 0x40: // IN B, (C)
            b = inputHandler(c);
            break;
        case 0x41: // OUT (C), B
            outputHandler(b, c);
            break;
        case 0x42: // SBC HL, BC
            temp = (uint16_t&)l | (h << 8);
            temp2 = (uint16_t&)c | (b << 8);
            alu(temp,temp2, ALU_SBC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x43: // LD (nn), BC
            memory[fetchOperand() | (fetchOperand() << 8)] = c;
            memory[fetchOperand() | (fetchOperand() << 8) + 1] = b;
            break;
        case 0x44: // NEG
            a = -a;
            break;
        case 0x45: // RETN
            // Add more functionality later when interrupts are better implemented
            pc = pop();
            iff1 = iff2;
            break;
        case 0x46: // IM 0
            im = 0;
            break;
        case 0x47: // LD I, A
            i = a;
            break;
        case 0x48: // IN C, (C)
            c = inputHandler(c);
            break;
        case 0x49: // OUT (C), C
            outputHandler(c, c);
            break;
        case 0x4A: // ADC HL, BC
            temp = (uint16_t&)l | (h << 8);
            temp2 = (uint16_t&)c | (b << 8);
            alu(temp,temp2, ALU_ADC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x4B: // LD BC, (nn)
            c = memory[fetchOperand() | (fetchOperand() << 8)];
            b = memory[fetchOperand() | (fetchOperand() << 8) + 1];
            break;
        case 0x4D: // RETI
            // Add more functionality later when interrupts are better implemented
            pc = pop();
            iff1 = iff2;
            break;
        case 0x4F: // LD R, A
            r = a;
            break;
        case 0x50: // IN D, (C)
            d = inputHandler(c);
            break;
        case 0x51: // OUT (C), D
            outputHandler(d, c);
            break;
        case 0x52: // SBC HL, DE
            temp = (uint16_t&)l | (h << 8);
            temp2 = (uint16_t&)d | (e << 8);
            alu(temp,temp2, ALU_SBC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x53: // LD (nn), DE
            memory[fetchOperand() | (fetchOperand() << 8)] = e;
            memory[fetchOperand() | (fetchOperand() << 8) + 1] = d;
            break;
        case 0x56: // IM 1
            im = 1;
            break;
        case 0x57: // LD A, I
            a = i;
            break;
        case 0x58: // IN E, (C)
            e = inputHandler(c);
            break;
        case 0x59: // OUT (C), E
            outputHandler(e, c);
            break;
        case 0x5A: // ADC HL, DE
            temp = (uint16_t&)l | (h << 8);
            temp2 = (uint16_t&)d | (e << 8);
            alu(temp,temp2, ALU_ADC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x5B: // LD DE, (nn)
            e = memory[fetchOperand() | (fetchOperand() << 8)];
            d = memory[fetchOperand() | (fetchOperand() << 8) + 1];
            break;
        case 0x5E: // IM 2
            im = 2;
            break;
        case 0x5F: // LD A, R
            a = r;
            break;
        case 0x60: // IN H, (C)
            h = inputHandler(c);
            break;
        case 0x61: // OUT (C), H
            outputHandler(h, c);
            break;
        case 0x62: // SBC HL, HL
            temp = (uint16_t&)l | (h << 8);
            alu(temp,temp, ALU_SBC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x67: // RRD
            w = memory[fetchOperand() | (fetchOperand() << 8)];
            memory[fetchOperand() | (fetchOperand() << 8)] = (w >> 4) | (w << 4);
            break;
        case 0x68: // IN L, (C)
            l = inputHandler(c);
            break;
        case 0x69: // OUT (C), L
            outputHandler(l, c);
            break;
        case 0x6A: // ADC HL, HL
            temp = (uint16_t&)l | (h << 8);
            alu(temp,temp, ALU_ADC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x6F: // RLD
            w = memory[fetchOperand() | (fetchOperand() << 8)];
            memory[fetchOperand() | (fetchOperand() << 8)] = (w << 4) | (w >> 4);
            break;
        case 0x72: // SBC HL, SP
            temp = (uint16_t&)l | (h << 8);
            alu(temp, sp, ALU_SBC16);
            l = temp & 0xFF;
            h = temp >> 8;
            break;
        case 0x73: // LD (nn), SP
            memory[fetchOperand() | (fetchOperand() << 8)] = (sp & 0xff);
            memory[fetchOperand() | (fetchOperand() << 8) + 1] = (sp << 8);
            break;
        case 0x78: // IN A, (C)
            a = inputHandler(c);
            break;
        case 0x79: // OUT (C), A
            outputHandler(a,c);
            break;
        case 0x7A: // ADC HL, SP
            temp = (uint16_t&)l | (h << 8);
            alu(temp, sp, ALU_ADC16);
            l = temp & 0xFF;
            h = temp >> 8;
        case 0x7B: // LD (nn), SP
            temp = fetchOperand() | (fetchOperand() << 8);
            sp = (memory[temp] | memory[temp+1] << 8);
        case 0xA0: // LDI
            memory[e | (d << 8)] = memory[h | (l << 8)];
            incRegPair(l, h);
            incRegPair(e, d);
            decRegPair(c, b);
            if (b == 0 && c == 0) {
                f |= FLAG_C;
            } else f &= ~FLAG_C;
            break;
        case 0xA1: // CPI
            alu((uint16_t&)a, memory[h | (l << 8)], ALU_CP8);
            incRegPair(l, h);
            incRegPair(e, d);
            decRegPair(c, b);
            if (b == 0 && c == 0) {
                f |= FLAG_C;
            } else f &= ~FLAG_C;
            break;
        case 0xA2: // INI
            memory[e | (d << 8)] = inputHandler(c);
            incRegPair(l, h);
            alu((uint16_t&)b, 0, ALU_DEC8);
            break;
        case 0xA3: // OUTI
            outputHandler(memory[h | (l << 8)], c);
            incRegPair(l, h);
            alu((uint16_t&)b, 0, ALU_DEC8);
            break;
        case 0xA8: // LDD
            memory[e | (d << 8)] = memory[h | (l << 8)];
            decRegPair(l, h);
            decRegPair(e, d);
            decRegPair(c, b);
            if (b == 0 && c == 0) {
                f |= FLAG_C;
            } else f&= ~FLAG_C;
            break;
        case 0xA9: // CPD
            alu((uint16_t&)a, memory[h | (l << 8)], ALU_CP8);
            decRegPair(l, h);
            decRegPair(e, d);
            decRegPair(c, b);
            if (b == 0 && c == 0) {
                f |= FLAG_C;
            } else f &= ~FLAG_C;
            break;
        case 0xAA: // IND
            memory[e | (d << 8)] = inputHandler(c);
            decRegPair(l, h);
            alu((uint16_t&)b, 0, ALU_DEC8);
            break;
        case 0xAB: // OUTD
            outputHandler(memory[h | (l << 8)], c);
            decRegPair(l, h);
            alu((uint16_t&)b, 0, ALU_DEC8);
            break;
        case 0xB0: // LDIR
            while (b != 0 || c != 0) {
                memory[e | (d << 8)] = memory[h | (l << 8)];
                incRegPair(l, h);
                incRegPair(e, d);
                decRegPair(c, b);
                if (b == 0 && c == 0) {
                    f |= FLAG_C;
                } else f &= ~FLAG_C;
            }
            break;
        case 0xB1: // CPIR
            while (b != 0 || c != 0 || (f & FLAG_Z) == 0) {
                alu((uint16_t&)a, memory[h | (l << 8)], ALU_CP8);
                incRegPair(l, h);
                incRegPair(e, d);
                decRegPair(c, b);
                if (b == 0 && c == 0) {
                    f |= FLAG_C;
                } else f &= ~FLAG_C;
            }
            break;
        case 0xB2: // INIR
            while (b != 0 || c != 0) {
                memory[e | (d << 8)] = inputHandler(c);
                incRegPair(l, h);
                alu((uint16_t&)b, 0, ALU_DEC8);
            }
            break;
        case 0xB3: // OUTIR
            while (b != 0 || c != 0 || (f & FLAG_Z) == 0) {
                outputHandler(memory[h | (l << 8)], c);
                incRegPair(l, h);
                alu((uint16_t&)b, 0, ALU_DEC8);
            }
            break;
        case 0xB4: // LDDR
            while (b != 0 || c != 0) {
                memory[e | (d << 8)] = memory[h | (l << 8)];
                decRegPair(l, h);
                decRegPair(e, d);
                decRegPair(c, b);
                if (b == 0 && c == 0) {
                    f |= FLAG_C;
                } else f &= ~FLAG_C;
            }
            break;
        case 0xB5: // CPDR
            while (b != 0 || c != 0 || (f & FLAG_Z) == 0) {
                alu((uint16_t&)a, memory[h | (l << 8)], ALU_CP8);
                decRegPair(l, h);
                decRegPair(e, d);
                decRegPair(c, b);
                if (b == 0 && c == 0) {
                    f |= FLAG_C;
                } else f &= ~FLAG_C;
            }
            break;
        case 0xB6: // INDR
            while (b != 0 || c != 0) {
                memory[e | (d << 8)] = inputHandler(c);
                decRegPair(l, h);
                alu((uint16_t&)b, 0, ALU_DEC8);
            }
            break;
        case 0xB7: // OUTDR
            while (b != 0 || c != 0 || (f & FLAG_Z) == 0) {
                outputHandler(memory[h | (l << 8)], c);
                decRegPair(l, h);
                alu((uint16_t&)b, 0, ALU_DEC8);
            }
            break;
        default:
            cout << "Invalid MISC instruction: " << hex << (int)ins << " at PC: " << (int)pc << endl;
            break;
    }
}

void Z80_Core::cb_instruction(uint8_t ins) {
    switch (ins) {
        case 0x00: // RLC B
            alu((uint16_t&)b, 0, ALU_RLC8);
            break;
        case 0x01: // RLC C
            alu((uint16_t&)c, 0, ALU_RLC8);
            break;
        case 0x02: // RLC D
            alu((uint16_t&)d, 0, ALU_RLC8);
            break;
        case 0x03: // RLC E
            alu((uint16_t&)e, 0, ALU_RLC8);
            break;
        case 0x04: // RLC H
            alu((uint16_t&)h, 0, ALU_RLC8);
            break;
        case 0x05: // RLC L
            alu((uint16_t&)l, 0, ALU_RLC8);
            break;
        case 0x06: // RLC (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RLC8);
            break;
        case 0x07: // RLC A
            alu((uint16_t&)a, 0, ALU_RLC8);
            break;
        case 0x08: // RRC B
            alu((uint16_t&)b, 0, ALU_RRC8);
            break;
        case 0x09: // RRC C
            alu((uint16_t&)c, 0, ALU_RRC8);
            break;
        case 0x0A: // RRC D
            alu((uint16_t&)d, 0, ALU_RRC8);
            break;
        case 0x0B: // RRC E
            alu((uint16_t&)e, 0, ALU_RRC8);
            break;
        case 0x0C: // RRC H
            alu((uint16_t&)h, 0, ALU_RRC8);
            break;
        case 0x0D: // RRC L
            alu((uint16_t&)l, 0, ALU_RRC8);
            break;
        case 0x0E: // RRC (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RRC8);
            break;
        case 0x0F: // RRC A
            alu((uint16_t&)a, 0, ALU_RRC8);
            break;
        case 0x10: // RL B
            alu((uint16_t&)b, 0, ALU_RL8);
            break;
        case 0x11: // RL C
            alu((uint16_t&)c, 0, ALU_RL8);
            break;
        case 0x12: // RL D
            alu((uint16_t&)d, 0, ALU_RL8);
            break;
        case 0x13: // RL E
            alu((uint16_t&)e, 0, ALU_RL8);
            break;
        case 0x14: // RL H
            alu((uint16_t&)h, 0, ALU_RL8);
            break;
        case 0x15: // RL L
            alu((uint16_t&)l, 0, ALU_RL8);
            break;
        case 0x16: // RL (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RL8);
            break;
        case 0x17: // RL A
            alu((uint16_t&)a, 0, ALU_RL8);
            break;
        case 0x18: // RR B
            alu((uint16_t&)b, 0, ALU_RR8);
            break;
        case 0x19: // RR C
            alu((uint16_t&)c, 0, ALU_RR8);
            break;
        case 0x1A: // RR D
            alu((uint16_t&)d, 0, ALU_RR8);
            break;
        case 0x1B: // RR E
            alu((uint16_t&)e, 0, ALU_RR8);
            break;
        case 0x1C: // RR H
            alu((uint16_t&)h, 0, ALU_RR8);
            break;
        case 0x1D: // RR L
            alu((uint16_t&)l, 0, ALU_RR8);
            break;
        case 0x1E: // RR (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RR8);
            break;
        case 0x1F: // RR A
            alu((uint16_t&)a, 0, ALU_RR8);
            break;
        case 0x20: // SLA B
            alu((uint16_t&)b, 0, ALU_SLA8);
            break;
        case 0x21: // SLA C
            alu((uint16_t&)c, 0, ALU_SLA8);
            break;
        case 0x22: // SLA D
            alu((uint16_t&)d, 0, ALU_SLA8);
            break;
        case 0x23: // SLA E
            alu((uint16_t&)e, 0, ALU_SLA8);
            break;
        case 0x24: // SLA H
            alu((uint16_t&)h, 0, ALU_SLA8);
            break;
        case 0x25: // SLA L
            alu((uint16_t&)l, 0, ALU_SLA8);
            break;
        case 0x26: // SLA (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SLA8);
            break;
        case 0x27: // SLA A
            alu((uint16_t&)a, 0, ALU_SLA8);
            break;
        case 0x28: // SRA B
            alu((uint16_t&)b, 0, ALU_SRA8);
            break;
        case 0x29: // SRA C
            alu((uint16_t&)c, 0, ALU_SRA8);
            break;
        case 0x2A: // SRA D
            alu((uint16_t&)d, 0, ALU_SRA8);
            break;
        case 0x2B: // SRA E
            alu((uint16_t&)e, 0, ALU_SRA8);
            break;
        case 0x2C: // SRA H
            alu((uint16_t&)h, 0, ALU_SRA8);
            break;
        case 0x2D: // SRA L
            alu((uint16_t&)l, 0, ALU_SRA8);
            break;
        case 0x2E: // SRA (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SRA8);
            break;
        case 0x2F: // SRA A
            alu((uint16_t&)a, 0, ALU_SRA8);
            break;
        case 0x38: // SRL B
            alu((uint16_t&)b, 0, ALU_SRL8);
            break;
        case 0x39: // SRL C
            alu((uint16_t&)c, 0, ALU_SRL8);
            break;
        case 0x3A: // SRL D
            alu((uint16_t&)d, 0, ALU_SRL8);
            break;
        case 0x3B: // SRL E
            alu((uint16_t&)e, 0, ALU_SRL8);
            break;
        case 0x3C: // SRL H
            alu((uint16_t&)h, 0, ALU_SRL8);
            break;
        case 0x3D: // SRL L
            alu((uint16_t&)l, 0, ALU_SRL8);
            break;
        case 0x3E: // SRL (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SRL8);
            break;
        case 0x3F: // SRL A
            alu((uint16_t&)a, 0, ALU_SRL8);
            break;
        case 0x40: // BIT 0, B
            alu((uint16_t&)b, 0, ALU_BIT0);
            break;
        case 0x41: // BIT 0, C
            alu((uint16_t&)c, 0, ALU_BIT0);
            break;
        case 0x42: // BIT 0, D
            alu((uint16_t&)d, 0, ALU_BIT0);
            break;
        case 0x43: // BIT 0, E
            alu((uint16_t&)e, 0, ALU_BIT0);
            break;
        case 0x44: // BIT 0, H
            alu((uint16_t&)h, 0, ALU_BIT0);
            break;
        case 0x45: // BIT 0, L
            alu((uint16_t&)l, 0, ALU_BIT0);
            break;
        case 0x46: // BIT 0, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_BIT0);
            break;
        case 0x47: // BIT 0, A
            alu((uint16_t&)a, 0, ALU_BIT0);
            break;
        case 0x48: // BIT 1, B
            alu((uint16_t&)b, 0, ALU_BIT1);
            break;
        case 0x49: // BIT 1, C
            alu((uint16_t&)c, 0, ALU_BIT1);
            break;
        case 0x4A: // BIT 1, D
            alu((uint16_t&)d, 0, ALU_BIT1);
            break;
        case 0x4B: // BIT 1, E
            alu((uint16_t&)e, 0, ALU_BIT1);
            break;
        case 0x4C: // BIT 1, H
            alu((uint16_t&)h, 0, ALU_BIT1);
            break;
        case 0x4D: // BIT 1, L
            alu((uint16_t&)l, 0, ALU_BIT1);
            break;
        case 0x4E: // BIT 1, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_BIT1);
            break;
        case 0x4F: // BIT 1, A
            alu((uint16_t&)a, 0, ALU_BIT1);
            break;
        case 0x50: // BIT 2, B
            alu((uint16_t&)b, 0, ALU_BIT2);
            break;
        case 0x51: // BIT 2, C
            alu((uint16_t&)c, 0, ALU_BIT2);
            break;
        case 0x52: // BIT 2, D
            alu((uint16_t&)d, 0, ALU_BIT2);
            break;
        case 0x53: // BIT 2, E
            alu((uint16_t&)e, 0, ALU_BIT2);
            break;
        case 0x54: // BIT 2, H
            alu((uint16_t&)h, 0, ALU_BIT2);
            break;
        case 0x55: // BIT 2, L
            alu((uint16_t&)l, 0, ALU_BIT2);
            break;
        case 0x56: // BIT 2, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_BIT2);
            break;
        case 0x57: // BIT 2, A
            alu((uint16_t&)a, 0, ALU_BIT2);
            break;
        case 0x58: // BIT 3, B
            alu((uint16_t&)b, 0, ALU_BIT3);
            break;
        case 0x59: // BIT 3, C
            alu((uint16_t&)c, 0, ALU_BIT3);
            break;
        case 0x5A: // BIT 3, D
            alu((uint16_t&)d, 0, ALU_BIT3);
            break;
        case 0x5B: // BIT 3, E
            alu((uint16_t&)e, 0, ALU_BIT3);
            break;
        case 0x5C: // BIT 3, H
            alu((uint16_t&)h, 0, ALU_BIT3);
            break;
        case 0x5D: // BIT 3, L
            alu((uint16_t&)l, 0, ALU_BIT3);
            break;
        case 0x5E: // BIT 3, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_BIT3);
            break;
        case 0x5F: // BIT 3, A
            alu((uint16_t&)a, 0, ALU_BIT3);
            break;
        case 0x60: // BIT 4, B
            alu((uint16_t&)b, 0, ALU_BIT4);
            break;
        case 0x61: // BIT 4, C
            alu((uint16_t&)c, 0, ALU_BIT4);
            break;
        case 0x62: // BIT 4, D
            alu((uint16_t&)d, 0, ALU_BIT4);
            break;
        case 0x63: // BIT 4, E
            alu((uint16_t&)e, 0, ALU_BIT4);
            break;
        case 0x64: // BIT 4, H
            alu((uint16_t&)h, 0, ALU_BIT4);
            break;
        case 0x65: // BIT 4, L
            alu((uint16_t&)l, 0, ALU_BIT4);
            break;
        case 0x66: // BIT 4, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_BIT4);
            break;
        case 0x67: // BIT 4, A
            alu((uint16_t&)a, 0, ALU_BIT4);
            break;
        case 0x68: // BIT 5, B
            alu((uint16_t&)b, 0, ALU_BIT5);
            break;
        case 0x69: // BIT 5, C
            alu((uint16_t&)c, 0, ALU_BIT5);
            break;
        case 0x6A: // BIT 5, D
            alu((uint16_t&)d, 0, ALU_BIT5);
            break;
        case 0x6B: // BIT 5, E
            alu((uint16_t&)e, 0, ALU_BIT5);
            break;
        case 0x6C: // BIT 5, H
            alu((uint16_t&)h, 0, ALU_BIT5);
            break;
        case 0x6D: // BIT 5, L
            alu((uint16_t&)l, 0, ALU_BIT5);
            break;
        case 0x6E: // BIT 5, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_BIT5);
            break;
        case 0x6F: // BIT 5, A
            alu((uint16_t&)a, 0, ALU_BIT5);
            break;
        case 0x70: // BIT 6, B
            alu((uint16_t&)b, 0, ALU_BIT6);
            break;
        case 0x71: // BIT 6, C
            alu((uint16_t&)c, 0, ALU_BIT6);
            break;
        case 0x72: // BIT 6, D
            alu((uint16_t&)d, 0, ALU_BIT6);
            break;
        case 0x73: // BIT 6, E
            alu((uint16_t&)e, 0, ALU_BIT6);
            break;
        case 0x74: // BIT 6, H
            alu((uint16_t&)h, 0, ALU_BIT6);
            break;
        case 0x75: // BIT 6, L
            alu((uint16_t&)l, 0, ALU_BIT6);
            break;
        case 0x76: // BIT 6, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_BIT6);
            break;
        case 0x77: // BIT 6, A
            alu((uint16_t&)a, 0, ALU_BIT6);
            break;
        case 0x78: // BIT 7, B
            alu((uint16_t&)b, 0, ALU_BIT7);
            break;
        case 0x79: // BIT 7, C
            alu((uint16_t&)c, 0, ALU_BIT7);
            break;
        case 0x7A: // BIT 7, D
            alu((uint16_t&)d, 0, ALU_BIT7);
            break;
        case 0x7B: // BIT 7, E
            alu((uint16_t&)e, 0, ALU_BIT7);
            break;
        case 0x7C: // BIT 7, H
            alu((uint16_t&)h, 0, ALU_BIT7);
            break;
        case 0x7D: // BIT 7, L
            alu((uint16_t&)l, 0, ALU_BIT7);
            break;
        case 0x7E: // BIT 7, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_BIT7);
            break;
        case 0x7F: // BIT 7, A
            alu((uint16_t&)a, 0, ALU_BIT7);
            break;
        case 0x80: // RES 0, B
            alu((uint16_t&)b, 0, ALU_RES0);
            break;
        case 0x81: // RES 0, C
            alu((uint16_t&)c, 0, ALU_RES0);
            break;
        case 0x82: // RES 0, D
            alu((uint16_t&)d, 0, ALU_RES0);
            break;
        case 0x83: // RES 0, E
            alu((uint16_t&)e, 0, ALU_RES0);
            break;
        case 0x84: // RES 0, H
            alu((uint16_t&)h, 0, ALU_RES0);
            break;
        case 0x85: // RES 0, L
            alu((uint16_t&)l, 0, ALU_RES0);
            break;
        case 0x86: // RES 0, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RES0);
            break;
        case 0x87: // RES 0, A
            alu((uint16_t&)a, 0, ALU_RES0);
            break;
        case 0x88: // RES 1, B
            alu((uint16_t&)b, 0, ALU_RES1);
            break;
        case 0x89: // RES 1, C
            alu((uint16_t&)c, 0, ALU_RES1);
            break;
        case 0x8A: // RES 1, D
            alu((uint16_t&)d, 0, ALU_RES1);
            break;
        case 0x8B: // RES 1, E
            alu((uint16_t&)e, 0, ALU_RES1);
            break;
        case 0x8C: // RES 1, H
            alu((uint16_t&)h, 0, ALU_RES1);
            break;
        case 0x8D: // RES 1, L
            alu((uint16_t&)l, 0, ALU_RES1);
            break;
        case 0x8E: // RES 1, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RES1);
            break;
        case 0x8F: // RES 1, A
            alu((uint16_t&)a, 0, ALU_RES1);
            break;
        case 0x90: // RES 2, B
            alu((uint16_t&)b, 0, ALU_RES2);
            break;
        case 0x91: // RES 2, C
            alu((uint16_t&)c, 0, ALU_RES2);
            break;
        case 0x92: // RES 2, D
            alu((uint16_t&)d, 0, ALU_RES2);
            break;
        case 0x93: // RES 2, E
            alu((uint16_t&)e, 0, ALU_RES2);
            break;
        case 0x94: // RES 2, H
            alu((uint16_t&)h, 0, ALU_RES2);
            break;
        case 0x95: // RES 2, L
            alu((uint16_t&)l, 0, ALU_RES2);
            break;
        case 0x96: // RES 2, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RES2);
            break;
        case 0x97: // RES 2, A
            alu((uint16_t&)a, 0, ALU_RES2);
            break;
        case 0x98: // RES 3, B
            alu((uint16_t&)b, 0, ALU_RES3);
            break;
        case 0x99: // RES 3, C
            alu((uint16_t&)c, 0, ALU_RES3);
            break;
        case 0x9A: // RES 3, D
            alu((uint16_t&)d, 0, ALU_RES3);
            break;
        case 0x9B: // RES 3, E
            alu((uint16_t&)e, 0, ALU_RES3);
            break;
        case 0x9C: // RES 3, H
            alu((uint16_t&)h, 0, ALU_RES3);
            break;
        case 0x9D: // RES 3, L
            alu((uint16_t&)l, 0, ALU_RES3);
            break;
        case 0x9E: // RES 3, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RES3);
            break;
        case 0x9F: // RES 3, A
            alu((uint16_t&)a, 0, ALU_RES3);
            break;
        case 0xA0: // RES 4, B
            alu((uint16_t&)b, 0, ALU_RES4);
            break;
        case 0xA1: // RES 4, C
            alu((uint16_t&)c, 0, ALU_RES4);
            break;
        case 0xA2: // RES 4, D
            alu((uint16_t&)d, 0, ALU_RES4);
            break;
        case 0xA3: // RES 4, E
            alu((uint16_t&)e, 0, ALU_RES4);
            break;
        case 0xA4: // RES 4, H
            alu((uint16_t&)h, 0, ALU_RES4);
            break;
        case 0xA5: // RES 4, L
            alu((uint16_t&)l, 0, ALU_RES4);
            break;
        case 0xA6: // RES 4, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RES4);
            break;
        case 0xA7: // RES 4, A
            alu((uint16_t&)a, 0, ALU_RES4);
            break;
        case 0xA8: // RES 5, B
            alu((uint16_t&)b, 0, ALU_RES5);
            break;
        case 0xA9: // RES 5, C
            alu((uint16_t&)c, 0, ALU_RES5);
            break;
        case 0xAA: // RES 5, D
            alu((uint16_t&)d, 0, ALU_RES5);
            break;
        case 0xAB: // RES 5, E
            alu((uint16_t&)e, 0, ALU_RES5);
            break;
        case 0xAC: // RES 5, H
            alu((uint16_t&)h, 0, ALU_RES5);
            break;
        case 0xAD: // RES 5, L
            alu((uint16_t&)l, 0, ALU_RES5);
            break;
        case 0xAE: // RES 5, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RES5);
            break;
        case 0xAF: // RES 5, A
            alu((uint16_t&)a, 0, ALU_RES5);
            break;
        case 0xB0: // RES 6, B
            alu((uint16_t&)b, 0, ALU_RES6);
            break;
        case 0xB1: // RES 6, C
            alu((uint16_t&)c, 0, ALU_RES6);
            break;
        case 0xB2: // RES 6, D
            alu((uint16_t&)d, 0, ALU_RES6);
            break;
        case 0xB3: // RES 6, E
            alu((uint16_t&)e, 0, ALU_RES6);
            break;
        case 0xB4: // RES 6, H
            alu((uint16_t&)h, 0, ALU_RES6);
            break;
        case 0xB5: // RES 6, L
            alu((uint16_t&)l, 0, ALU_RES6);
            break;
        case 0xB6: // RES 6, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RES6);
            break;
        case 0xB7: // RES 6, A
            alu((uint16_t&)a, 0, ALU_RES6);
            break;
        case 0xB8: // RES 7, B
            alu((uint16_t&)b, 0, ALU_RES7);
            break;
        case 0xB9: // RES 7, C
            alu((uint16_t&)c, 0, ALU_RES7);
            break;
        case 0xBA: // RES 7, D
            alu((uint16_t&)d, 0, ALU_RES7);
            break;
        case 0xBB: // RES 7, E
            alu((uint16_t&)e, 0, ALU_RES7);
            break;
        case 0xBC: // RES 7, H
            alu((uint16_t&)h, 0, ALU_RES7);
            break;
        case 0xBD: // RES 7, L
            alu((uint16_t&)l, 0, ALU_RES7);
            break;
        case 0xBE: // RES 7, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_RES7);
            break;
        case 0xBF: // RES 7, A
            alu((uint16_t&)a, 0, ALU_RES7);
            break;
        case 0xC0: // SET 0, B
            alu((uint16_t&)b, 0, ALU_SET0);
            break;
        case 0xC1: // SET 0, C
            alu((uint16_t&)c, 0, ALU_SET0);
            break;
        case 0xC2: // SET 0, D
            alu((uint16_t&)d, 0, ALU_SET0);
            break;
        case 0xC3: // SET 0, E
            alu((uint16_t&)e, 0, ALU_SET0);
            break;
        case 0xC4: // SET 0, H
            alu((uint16_t&)h, 0, ALU_SET0);
            break;
        case 0xC5: // SET 0, L
            alu((uint16_t&)l, 0, ALU_SET0);
            break;
        case 0xC6: // SET 0, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SET0);
            break;
        case 0xC7: // SET 0, A
            alu((uint16_t&)a, 0, ALU_SET0);
            break;
        case 0xC8: // SET 1, B
            alu((uint16_t&)b, 0, ALU_SET1);
            break;
        case 0xC9: // SET 1, C
            alu((uint16_t&)c, 0, ALU_SET1);
            break;
        case 0xCA: // SET 1, D
            alu((uint16_t&)d, 0, ALU_SET1);
            break;
        case 0xCB: // SET 1, E
            alu((uint16_t&)e, 0, ALU_SET1);
            break;
        case 0xCC: // SET 1, H
            alu((uint16_t&)h, 0, ALU_SET1);
            break;
        case 0xCD: // SET 1, L
            alu((uint16_t&)l, 0, ALU_SET1);
            break;
        case 0xCE: // SET 1, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SET1);
            break;
        case 0xCF: // SET 1, A
            alu((uint16_t&)a, 0, ALU_SET1);
            break;
        case 0xD0: // SET 2, B
            alu((uint16_t&)b, 0, ALU_SET2);
            break;
        case 0xD1: // SET 2, C
            alu((uint16_t&)c, 0, ALU_SET2);
            break;
        case 0xD2: // SET 2, D
            alu((uint16_t&)d, 0, ALU_SET2);
            break;
        case 0xD3: // SET 2, E
            alu((uint16_t&)e, 0, ALU_SET2);
            break;
        case 0xD4: // SET 2, H
            alu((uint16_t&)h, 0, ALU_SET2);
            break;
        case 0xD5: // SET 2, L
            alu((uint16_t&)l, 0, ALU_SET2);
            break;
        case 0xD6: // SET 2, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SET2);
            break;
        case 0xD7: // SET 2, A
            alu((uint16_t&)a, 0, ALU_SET2);
            break;
        case 0xD8: // SET 3, B
            alu((uint16_t&)b, 0, ALU_SET3);
            break;
        case 0xD9: // SET 3, C
            alu((uint16_t&)c, 0, ALU_SET3);
            break;
        case 0xDA: // SET 3, D
            alu((uint16_t&)d, 0, ALU_SET3);
            break;
        case 0xDB: // SET 3, E
            alu((uint16_t&)e, 0, ALU_SET3);
            break;
        case 0xDC: // SET 3, H
            alu((uint16_t&)h, 0, ALU_SET3);
            break;
        case 0xDD: // SET 3, L
            alu((uint16_t&)l, 0, ALU_SET3);
            break;
        case 0xDE: // SET 3, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SET3);
            break;
        case 0xDF: // SET 3, A
            alu((uint16_t&)a, 0, ALU_SET3);
            break;
        case 0xE0: // SET 4, B
            alu((uint16_t&)b, 0, ALU_SET4);
            break;
        case 0xE1: // SET 4, C
            alu((uint16_t&)c, 0, ALU_SET4);
            break;
        case 0xE2: // SET 4, D
            alu((uint16_t&)d, 0, ALU_SET4);
            break;
        case 0xE3: // SET 4, E
            alu((uint16_t&)e, 0, ALU_SET4);
            break;
        case 0xE4: // SET 4, H
            alu((uint16_t&)h, 0, ALU_SET4);
            break;
        case 0xE5: // SET 4, L
            alu((uint16_t&)l, 0, ALU_SET4);
            break;
        case 0xE6: // SET 4, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SET4);
            break;
        case 0xE7: // SET 4, A
            alu((uint16_t&)a, 0, ALU_SET4);
            break;
        case 0xE8: // SET 5, B
            alu((uint16_t&)b, 0, ALU_SET5);
            break;
        case 0xE9: // SET 5, C
            alu((uint16_t&)c, 0, ALU_SET5);
            break;
        case 0xEA: // SET 5, D
            alu((uint16_t&)d, 0, ALU_SET5);
            break;
        case 0xEB: // SET 5, E
            alu((uint16_t&)e, 0, ALU_SET5);
            break;
        case 0xEC: // SET 5, H
            alu((uint16_t&)h, 0, ALU_SET5);
            break;
        case 0xED: // SET 5, L
            alu((uint16_t&)l, 0, ALU_SET5);
            break;
        case 0xEE: // SET 5, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SET5);
            break;
        case 0xEF: // SET 5, A
            alu((uint16_t&)a, 0, ALU_SET5);
            break;
        case 0xF0: // SET 6, B
            alu((uint16_t&)b, 0, ALU_SET6);
            break;
        case 0xF1: // SET 6, C
            alu((uint16_t&)c, 0, ALU_SET6);
            break;
        case 0xF2: // SET 6, D
            alu((uint16_t&)d, 0, ALU_SET6);
            break;
        case 0xF3: // SET 6, E
            alu((uint16_t&)e, 0, ALU_SET6);
            break;
        case 0xF4: // SET 6, H
            alu((uint16_t&)h, 0, ALU_SET6);
            break;
        case 0xF5: // SET 6, L
            alu((uint16_t&)l, 0, ALU_SET6);
            break;
        case 0xF6: // SET 6, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SET6);
            break;
        case 0xF7: // SET 6, A
            alu((uint16_t&)a, 0, ALU_SET6);
            break;
        case 0xF8: // SET 7, B
            alu((uint16_t&)b, 0, ALU_SET7);
            break;
        case 0xF9: // SET 7, C
            alu((uint16_t&)c, 0, ALU_SET7);
            break;
        case 0xFA: // SET 7, D
            alu((uint16_t&)d, 0, ALU_SET7);
            break;
        case 0xFB: // SET 7, E
            alu((uint16_t&)e, 0, ALU_SET7);
            break;
        case 0xFC: // SET 7, H
            alu((uint16_t&)h, 0, ALU_SET7);
            break;
        case 0xFD: // SET 7, L
            alu((uint16_t&)l, 0, ALU_SET7);
            break;
        case 0xFE: // SET 7, (HL)
            alu((uint16_t&)memory[l | (h << 8)], 0, ALU_SET7);
            break;
        case 0xFF: // SET 7, A
            alu((uint16_t&)a, 0, ALU_SET7);
            break;
        default:
            cout << "Invalid BIT instruction: " << hex << (int)ins << " at PC: " << (int)pc << endl;
    }
}

void Z80_Core::dd_instruction(uint8_t ins) { // TODO: Implement undocumented instructions
    switch (ins) {
        case 0x09: // ADD IX, BC
            alu(ix, convToRegPair(c, b), ALU_ADD16);
            break;
        case 0x19: // ADD IX, DE
            alu(ix, convToRegPair(e, d), ALU_ADD16);
            break;
        case 0x21: // LD IX, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            ix = w | (z << 8);
            break;
        case 0x22: // LD (nn), IX
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            memory[w | (z << 8)] = l;
            memory[(w | (z << 8)) + 1] = h;
            break;
        case 0x23: // INC IX
            alu(ix, 0, ALU_INC16);
            break;
        case 0x29: // ADD IX, IX
            alu(ix, ix, ALU_ADD16);
            break;
        case 0x2A: // LD IX, (nn)
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            ix = (w | (z << 8));
            break;
        case 0x2B: // DEC IX
            alu(ix, 0, ALU_DEC16);
            break;
        case 0x34: // INC (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)memory[ix+w], 0 , ALU_INC8);
            break;
        case 0x35: // DEC (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)memory[ix+w], 0 , ALU_DEC8);
            break;
        case 0x36: // LD (IX+d), n
            w = (int8_t)fetchOperand();
            memory[ix+w] = fetchOperand();
            break;
        case 0x39: // ADD IX, SP
            alu(ix, sp, ALU_ADD16);
            break;
        case 0x46: // LD B, (IX+d)
            w = (int8_t)fetchOperand();
            b = memory[ix+w];
            break;
        case 0x4E: // LD C, (IX+d)
            w = (int8_t)fetchOperand();
            c = memory[ix+w];
            break;
        case 0x56: // LD D, (IX+d)
            w = (int8_t)fetchOperand();
            d = memory[ix+w];
            break;
        case 0x5E: // LD E, (IX+d)
            w = (int8_t)fetchOperand();
            e = memory[ix+w];
            break;
        case 0x66: // LD H, (IX+d)
            w = (int8_t)fetchOperand();
            h = memory[ix+w];
            break;
        case 0x70: // LD (IX+d), B
            w = (int8_t)fetchOperand();
            memory[ix+w] = b;
            break;
        case 0x71: // LD (IX+d), C
            w = (int8_t)fetchOperand();
            memory[ix+w] = c;
            break;
        case 0x72: // LD (IX+d), D
            w = (int8_t)fetchOperand();
            memory[ix+w] = d;
            break;
        case 0x73: // LD (IX+d), E
            w = (int8_t)fetchOperand();
            memory[ix+w] = e;
            break;
        case 0x74: // LD (IX+d), H
            w = (int8_t)fetchOperand();
            memory[ix+w] = h;
            break;
        case 0x75: // LD (IX+d), L
            w = (int8_t)fetchOperand();
            memory[ix+w] = l;
            break;
        case 0x77: // LD (IX+d), A
            w = (int8_t)fetchOperand();
            memory[ix+w] = a;
            break;
        case 0x7e: // LD A, (IX+d)
            w = (int8_t)fetchOperand();
            a = memory[ix+w];
            break;
        case 0x86: // ADD A, (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[ix+w], ALU_ADD8);
            break;
        case 0x8e: // ADC A, (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[ix+w], ALU_ADC8);
            break;
        case 0x96: // SUB (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[ix+w], ALU_SUB8);
            break;
        case 0x9e: // SBC (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[ix+w], ALU_SBC8);
            break;
        case 0xA6: // AND (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[ix+w], ALU_AND8);
            break;
        case 0xAE: // XOR (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[ix+w], ALU_XOR8);
            break;
        case 0xB6: // OR (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[ix+w], ALU_OR8);
            break;
        case 0xBE: // CP (IX+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[ix+w], ALU_CP8);
            break;
        case 0xCB: // IX Bit
            cout << "IX BIT Instructions not implemented";
            break;
        case 0xE1: // POP IX
            ix = pop();
            break;
        case 0xE3: // EX (SP), IX
            w = ix & 0xff;
            z = ix >> 8;
            ix = memory[sp] | (memory[sp+1] << 8);
            memory[sp] = w;
            memory[sp+1] = z;
            break;
        case 0xE5: // PUSH IX
            push(ix);
            break;
        case 0xE9: // JP (IX)
            pc = ix;
            break;
        case 0xF9: // LD SP, IX
            sp = ix;
            break;
        default:
            cout << "Invalid DD instruction: " << hex << (int)ins << " at PC: " << (int)pc << endl;
    }
}

void Z80_Core::fd_instruction(uint8_t ins) { // TODO: Implement undocumented instructions
    switch (ins) {
        case 0x09: // ADD iy, BC
            alu(iy, convToRegPair(c, b), ALU_ADD16);
            break;
        case 0x19: // ADD iy, DE
            alu(iy, convToRegPair(e, d), ALU_ADD16);
            break;
        case 0x21: // LD iy, nn
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            iy = w | (z << 8);
            break;
        case 0x22: // LD (nn), iy
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            memory[w | (z << 8)] = l;
            memory[(w | (z << 8)) + 1] = h;
            break;
        case 0x23: // INC iy
            alu(iy, 0, ALU_INC16);
            break;
        case 0x29: // ADD iy, iy
            alu(iy, iy, ALU_ADD16);
            break;
        case 0x2A: // LD iy, (nn)
            w = fetchOperand(); // low byte
            z = fetchOperand(); // high byte
            iy = (w | (z << 8));
            break;
        case 0x2B: // DEC iy
            alu(iy, 0, ALU_DEC16);
            break;
        case 0x34: // INC (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)memory[iy+w], 0 , ALU_INC8);
            break;
        case 0x35: // DEC (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)memory[iy+w], 0 , ALU_DEC8);
            break;
        case 0x36: // LD (iy+d), n
            w = (int8_t)fetchOperand();
            memory[iy+w] = fetchOperand();
            break;
        case 0x39: // ADD iy, SP
            alu(iy, sp, ALU_ADD16);
            break;
        case 0x46: // LD B, (iy+d)
            w = (int8_t)fetchOperand();
            b = memory[iy+w];
            break;
        case 0x4E: // LD C, (iy+d)
            w = (int8_t)fetchOperand();
            c = memory[iy+w];
            break;
        case 0x56: // LD D, (iy+d)
            w = (int8_t)fetchOperand();
            d = memory[iy+w];
            break;
        case 0x5E: // LD E, (iy+d)
            w = (int8_t)fetchOperand();
            e = memory[iy+w];
            break;
        case 0x66: // LD H, (iy+d)
            w = (int8_t)fetchOperand();
            h = memory[iy+w];
            break;
        case 0x70: // LD (iy+d), B
            w = (int8_t)fetchOperand();
            memory[iy+w] = b;
            break;
        case 0x71: // LD (iy+d), C
            w = (int8_t)fetchOperand();
            memory[iy+w] = c;
            break;
        case 0x72: // LD (iy+d), D
            w = (int8_t)fetchOperand();
            memory[iy+w] = d;
            break;
        case 0x73: // LD (iy+d), E
            w = (int8_t)fetchOperand();
            memory[iy+w] = e;
            break;
        case 0x74: // LD (iy+d), H
            w = (int8_t)fetchOperand();
            memory[iy+w] = h;
            break;
        case 0x75: // LD (iy+d), L
            w = (int8_t)fetchOperand();
            memory[iy+w] = l;
            break;
        case 0x77: // LD (iy+d), A
            w = (int8_t)fetchOperand();
            memory[iy+w] = a;
            break;
        case 0x7e: // LD A, (iy+d)
            w = (int8_t)fetchOperand();
            a = memory[iy+w];
            break;
        case 0x86: // ADD A, (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[iy+w], ALU_ADD8);
            break;
        case 0x8e: // ADC A, (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[iy+w], ALU_ADC8);
            break;
        case 0x96: // SUB (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[iy+w], ALU_SUB8);
            break;
        case 0x9e: // SBC (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[iy+w], ALU_SBC8);
            break;
        case 0xA6: // AND (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[iy+w], ALU_AND8);
            break;
        case 0xAE: // XOR (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[iy+w], ALU_XOR8);
            break;
        case 0xB6: // OR (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[iy+w], ALU_OR8);
            break;
        case 0xBE: // CP (iy+d)
            w = (int8_t)fetchOperand();
            alu((uint16_t&)a, memory[iy+w], ALU_CP8);
            break;
        case 0xCB: // iy Bit
            cout << "iy BIT Instructions not implemented";
            break;
        case 0xE1: // POP iy
            iy = pop();
            break;
        case 0xE3: // EX (SP), iy
            // TODO
            break;
        case 0xE5: // PUSH iy
            push(iy);
            break;
        case 0xE9: // JP (iy)
            pc = iy;
            break;
        case 0xF9: // LD SP, iy
            sp = iy;
            break;
        default:
            cout << "Invalid FD instruction: " << hex << (int)ins << " at PC: " << (int)pc << endl;
    }
}