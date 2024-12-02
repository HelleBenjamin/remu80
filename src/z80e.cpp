
#include "../include/z80e.h"

#define clear() printf("\033[H\033[J") // macro to clear the screen

using namespace std;

vector<uint8_t> program;

/*Z80 EMULATOR by Benjamin Helle (2024)*/

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
}; // TODO: add all opcodes


Z80_Core::Z80_Core() {
    reset(); // initialize the cpu
}

void Z80_Core::loadProgram(vector<uint8_t>& inputProgram) {
    for (int i = 0; i < inputProgram.size(); i++) {
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
    for (int i = 0; i < program.size(); i++) {
        cout << i << ": " << program[i] << endl;
    }
    system("pause");
}


void Z80_Core::reset() {
    a = b = c = d = e = h = l = 0;
    afa = bca = dea = hla = 0;
    ix = iy = 0;
    pc = 0;
    sp = MEMORY_SIZE;
    acc = 0;
    f = 0;
    halt = false;
    interrupts = true;
    isInput = false;
}

void Z80_Core::run() {
    reset();
    pc = 0;
    while (!halt) {
        fetchInstruction();
        decode_execute();
        //Sleep(10);
    }
    if (DEBUG) {
        printInfo();
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

void Z80_Core::alu(size_t& op1, size_t& op2, uint8_t ins){
    /* 
    OP1 is the destination register
    OP2 is the source register/immediate
    INS is the ALU operation
    
    ALU instructions
        0x00, 8-bit ADD reg, reg
        0x01, 8-bit ADD reg, n
        0x02, 16-bit ADD reg, reg
        0x03, 16-bit ADD reg, nn
        0x04, 8-bit ADC reg, reg
        0x05, 8-bit ADC reg, n
        0x06, 16-bit ADC reg, reg
        0x07, 16-bit ADC reg, nn
        0x08, 8-bit SUB reg, reg
        0x09, 8-bit SUB reg, n
        0x0A, 16-bit SUB reg, reg
        0x0B, 16-bit SUB reg, nn
        0x0C, 8-bit SBC reg, reg
        0x0D, 8-bit SBC reg, n
        0x0E, 16-bit SBC reg, reg
        0x0F, 16-bit SBC reg, nn
        0x10, 8-bit AND reg, reg
        0x11, 8-bit AND reg, n
        0x12, 8-bit OR reg, reg
        0x13, 8-bit OR reg, n
        0x14, 8-bit XOR reg, reg
        0x15, 8-bit XOR reg, n
        0x16, 8-bit CP reg, reg
        0x17, 8-bit CP reg, n
    */
    bool bitShort = false; // false = 8-bit, true = 16-bit
    bool isCompare = false;
    int accumulator = 0;
    switch (ins) {
        case 0x00: // ADD reg, reg
            accumulator = op1 + op2;
            break;
        case 0x01: // ADD reg, n
            accumulator = op1 + op2;
            break;
        case 0x02: // ADD reg, reg
            accumulator = op1 + op2;
            bitShort = true;
            break;
        case 0x03: // ADD reg, nn
            accumulator = op1 + op2;
            bitShort = true;
            break;
        case 0x04: // ADC reg, reg
            accumulator = op1 + op2 + (f & 0x01);
            bitShort = true;
            break;
        case 0x05: // ADC reg, n
            accumulator = op1 + op2 + (f & 0x01);
            break;
        case 0x06: // ADC reg, reg
            accumulator = op1 + op2 + (f & 0x01);
            bitShort = true;
            break;
        case 0x07: // ADC reg, nn
            accumulator = op1 + op2 + (f & 0x01);
            bitShort = true;
            break;
        case 0x08: // SUB reg, reg
            accumulator = op1 - op2;
            break;
        case 0x09: // SUB reg, n
            accumulator = op1 - op2;
            break;
        case 0x0A: // SUB reg, reg
            accumulator = op1 - op2;
            bitShort = true;
            break;
        case 0x0B: // SUB reg, nn
            accumulator = op1 - op2;
            bitShort = true;
            break;
        case 0x0C: // SBC reg, reg
            accumulator = op1 - op2 - (f & 0x01);
            break;
        case 0x0D: // SBC reg, n
            accumulator = op1 - op2 - (f & 0x01);
            break;
        case 0x0E: // SBC reg, reg
            accumulator = op1 - op2 - (f & 0x01);
            bitShort = true;
            break;
        case 0x0F: // SBC reg, nn
            accumulator = op1 - op2 - (f & 0x01);
            bitShort = true;
            break;
        case 0x10: // AND reg, reg
            accumulator = op1 & op2;
            break;
        case 0x11: // AND reg, n
            accumulator = op1 & op2;
            break;
        case 0x12: // OR reg, reg
            accumulator = op1 | op2;
            break;
        case 0x13: // OR reg, n
            accumulator = op1 | op2;
            break;
        case 0x14: // XOR reg, reg
            accumulator = op1 ^ op2;
            break;
        case 0x15: // XOR reg, n
            accumulator = op1 ^ op2;
            break;
        case 0x16: // CP reg, reg
            accumulator = op1 - op2;
            break;
        case 0x17: // CP reg, n
            accumulator = op1 - op2;
            break;
        default:
            cout << "Invalid ALU instruction: " << hex << ins << endl;
    }

    //TODO: Add more flags and check if they should be set

    if (accumulator == 0) {
        f |= 0x40; // Set Z flag
    } else {
        f &= ~0x40; // Clear Z flag
    }

    if (accumulator > 0xff && !bitShort) { //8-bit overflow
        f |= 0x01 | 0x04; // carry and overflow
    } else {
        f &= ~(0x01 | 0x04); // clear carry and overflow
    }

    if (accumulator > 0xffff && bitShort) { //16-bit overflow
        f |= 0x01 | 0x04; // carry and overflow
    } else {
        f &= ~(0x01 | 0x04); // clear carry and overflow
    }

    if (accumulator < 0) {
        f |= 0x02 | 0x80; // negative and sign
    } else {
        f &= ~(0x02 | 0x80); // clear negative and sign
    }

    if (isCompare){
        if (accumulator == 0) {
            f |= 0x40; // Set Z flag
        } else {
            f &= ~0x40; // Clear Z flag
        }
        return;
    }

    if (bitShort) {
        op1 = bitset<16>(accumulator).to_ullong(); // Dest reg = 16-bit acc
    } else {
        op1 = bitset<8>(accumulator).to_ullong(); // Dest reg = 8-bit acc
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

uint8_t Z80_Core::inputHandler() {
    uint8_t input;
    scanf("%hhx", &input);
    return input;
}

uint8_t Z80_Core::outputHandler(uint8_t &reg) {
    printf("%c", reg);
    return reg;
}

void Z80_Core::swapRegs(uint8_t& temp1, uint8_t& temp2) {
    uint8_t temp3 = temp1;
    temp1 = temp2;
    temp2 = temp3;
}
void Z80_Core::flagsAfterCalcA() {
    if (acc > 255) {
        f |= 0x01 | 0x04; // carry and overflow
        a = acc;
    } else if (acc < 0) {
        f |= 0x02 | 0x80; // negative and sign
        a = acc;
    } else if (acc == 0) {
        f |= 0x40; // zero
        a = acc;
    } else if (acc == a) {
        bitset<8> acc_bits(acc);
        size_t num_set_bits = acc_bits.count();
        if (num_set_bits % 2 == 0) {
            f |= 0x04; // set if even
        } else {
            f &= ~0x04; // clear if odd
        }
    } else {
        a = acc;
    }
}

void Z80_Core::decode_execute() {
    uint8_t temp;
    switch (ins) {
        case 0x00: // NOP
            break;
        case 0x01: // LD BC, nn
            c = fetchOperand();
            b = fetchOperand();
            break;
        case 0x02: // LD (BC), A
            memory[b | (c << 8)] = a;
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
            b++;
            break;
        case 0x05: // DEC B
            b--;
            break;
        case 0x06: // LD B, n
            b = fetchOperand();
            break;
        case 0x07: // RLCA
            a = a << 1;
            break;
        case 0x08: // EX AF, AF'
            temp = (uint8_t)(afa << 8);
            swapRegs(a, temp);
            temp = (uint8_t)(afa & 0xff);
            swapRegs(f, temp);
            break;
        case 0x09: // ADD HL, BC
            acc = (h << 8 | l) + (c << 8 | b);
            if (acc > 65535) {
                f |= 0x01;
            }
            h = acc >> 8;
            l = acc & 0xff;
            break;
        case 0x0A: // LD A, (BC)
            a = memory[b | (c << 8)];
            break;
        case 0x0B: // DEC BC
            if (b == 0) {
                c--;
            } else {
                b--;
            }
            break;
        case 0x0C: // INC C
            c++;
            break;
        case 0x0D: // DEC C
            c--;
            break;
        case 0x0E: // LD C, n
            c = fetchOperand();
            break;
        case 0x0F: // RRCA
            a = a >> 1;
            f = a & 0x01;
            break;
        case 0x10: // DJNZ nn
            b--;
            if (b != 0) {
                pc = pc + fetchOperand() | (fetchOperand() << 8);
            }
            break;
        case 0x11: // LD DE, nn
            e = fetchOperand();
            d = fetchOperand();
            break;
        case 0x12: // LD (DE), A
            memory[d | (e << 8)] = a;
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
            d++;
            break;
        case 0x15: // DEC D
            d--;
            break;
        case 0x16: // LD D, n
            d = fetchOperand();
            break;
        case 0x17: // RLA
            a = a << 1;
            break;
        case 0x18: // JR n
            pc = pc + fetchOperand();
            break;
        case 0x19: // ADD HL, DE
            acc = (h << 8 | l) + (e << 8 | d);
            if (acc > 65535) {
                f |= 0x01;
            }
            h = acc >> 8;
            l = acc & 0xff;
            break;
        case 0x1A: // LD A, (DE)
            a = memory[d | (e << 8)];
            break;
        case 0x1B: // DEC DE
            if (d == 0) {
                e--;
            } else {
                d--;
            }
            break;
        case 0x1C: // INC E
            e++;
            break;
        case 0x1D: // DEC E
            e--;
            break;
        case 0x1E: // LD E, n
            e = fetchOperand();
            break;
        case 0x1F: // RRA
            a = a >> 1;
            break;
        case 0x20: // JR NZ, n
            if (!(f & 0x40)){
                pc = pc + fetchOperand();
            }
            break;
        case 0x21: // LD HL, nn
            l = fetchOperand();
            h = fetchOperand();
            break;
        case 0x22: // LD (nn), HL
            memory[fetchOperand() | (fetchOperand() << 8)] = l;
            memory[fetchOperand() | (fetchOperand() << 8) + 1] = h;
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
            h++;
            break;
        case 0x25: // DEC H
            h--;
            break;
        case 0x26: // LD H, n
            h = fetchOperand();
            break;
        case 0x27: // DAA
            //TODO
            break;
        case 0x28: // JR Z, n
            if (f & 0x40) {
                pc = pc + fetchOperand();
            }
            break;
        case 0x29: // ADD HL, HL
            acc = (h << 8 | l) + (h << 8 | l);
            if (acc > 65535) {
                f |= 0x01;
            }
            h = acc >> 8;
            l = acc & 0xff;
            break;
        case 0x2A: // LD HL, (nn)
            l = memory[fetchOperand() | (fetchOperand() << 8)];
            h = memory[fetchOperand() | (fetchOperand() << 8) + 1];
            break;
        case 0x2B: // DEC HL
            if (h == 0) {
                l--;
            } else {
                h--;
            }
            break;
        case 0x2C: // INC L
            l++;
            break;
        case 0x2D: // DEC L
            l--;
            break;
        case 0x2E: // LD L, n
            l = fetchOperand();
            break;
        case 0x2F: // CPL
            a = ~a;
            f ^= (1 << 4) | (1 << 1);
            break;
        case 0x30: // JR NC, n
            if (!(f & 0x01)) {
                pc = pc + fetchOperand();
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
            memory[l | (h << 8)]++;
            break;
        case 0x35: // DEC (HL)
            memory[l | (h << 8)]--;
            break;
        case 0x36: // LD (HL), n
            memory[l | (h << 8)] = fetchOperand();
            break;
        case 0x37: // SCF
            f |= 1;
            break;
        case 0x38: // JR C, n
            if (f & 0x01) {
                pc = pc + fetchOperand();
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
            a++;
            break;
        case 0x3D: // DEC A
            a--;
            break;
        case 0x3E: // LD A, n
            a = fetchOperand();
            break;
        case 0x3F: // CCF, invert carry flag
            f |= !(f & 0x01);
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
            acc = a + b;
            flagsAfterCalcA();
            break;
        case 0x81: // ADD A, C
            acc = a + c;
            flagsAfterCalcA();
            break;
        case 0x82: // ADD A, D
            acc = a + d;
            flagsAfterCalcA();
            break;
        case 0x83: // ADD A, E
            acc = a + e;
            flagsAfterCalcA();
            break;
        case 0x84: // ADD A, H
            acc = a + h;
            flagsAfterCalcA();
            break;
        case 0x85: // ADD A, L
            acc = a + l;
            flagsAfterCalcA();
            break;
        case 0x86: // ADD A, (HL)
            acc = a + memory[l | (h << 8)];
            flagsAfterCalcA();
            break;
        case 0x87: // ADD A, A
            acc = a + a;
            flagsAfterCalcA();
            break;
        case 0x88: // ADC A, B
            acc = a + b + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x89: // ADC A, C
            acc = a + c + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x8A: // ADC A, D
            acc = a + d + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x8B: // ADC A, E
            acc = a + e + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x8C: // ADC A, H
            acc = a + h + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x8D: // ADC A, L
            acc = a + l + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x8E: // ADC A, (HL)
            acc = a + memory[l | (h << 8)] + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x8F: // ADC A, A
            acc = a + a + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x90: // SUB B
            acc = a - b;
            flagsAfterCalcA();
            break;
        case 0x91: // SUB C
            acc = a - c;
            flagsAfterCalcA();
            break;
        case 0x92: // SUB D
            acc = a - d;
            flagsAfterCalcA();
            break;
        case 0x93: // SUB E
            acc = a - e;
            flagsAfterCalcA();
            break;
        case 0x94: // SUB H
            acc = a - h;
            flagsAfterCalcA();
            break;
        case 0x95: // SUB L
            acc = a - l;
            flagsAfterCalcA();
            break;
        case 0x96: // SUB (HL)
            acc = a - memory[l | (h << 8)];
            flagsAfterCalcA();
            break;
        case 0x97: // SUB A
            acc = a - a;
            flagsAfterCalcA();
            break;
        case 0x98: // SBC A, B
            acc = a - b - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x99: // SBC A, C
            acc = a - c - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x9A: // SBC A, D
            acc = a - d - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x9B: // SBC A, E
            acc = a - e - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x9C: // SBC A, H
            acc = a - h - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x9D: // SBC A, L
            acc = a - l - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x9E: // SBC A, (HL)
            acc = a - memory[l | (h << 8)] - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0x9F: // SBC A, A
            acc = a - a - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0xA0: // AND B
            acc = a & b;
            flagsAfterCalcA();
            break;
        case 0xA1: // AND C
            acc = a & c;
            flagsAfterCalcA();
            break;
        case 0xA2: // AND D
            acc = a & d;
            flagsAfterCalcA();
            break;
        case 0xA3: // AND E
            acc = a & e;
            flagsAfterCalcA();
            break;
        case 0xA4: // AND H
            acc = a & h;
            flagsAfterCalcA();
            break;
        case 0xA5: // AND L
            acc = a & l;
            flagsAfterCalcA();
        case 0xA6: // AND (HL)
            acc = a & memory[l | (h << 8)];
            flagsAfterCalcA();
            break;
        case 0xA7: // AND A
            acc = a & a;
            flagsAfterCalcA();
            break;
        case 0xA8: // XOR B
            acc = a ^ b;
            flagsAfterCalcA();
            break;
        case 0xA9: // XOR C
            acc = a ^ c;
            flagsAfterCalcA();
            break;
        case 0xAA: // XOR D
            acc = a ^ d;
            flagsAfterCalcA();
            break;
        case 0xAB: // XOR E
            acc = a ^ e;
            flagsAfterCalcA();
            break;
        case 0xAC: // XOR H
            acc = a ^ h;
            flagsAfterCalcA();
            break;
        case 0xAD: // XOR L
            acc = a ^ l;
            flagsAfterCalcA();
            break;
        case 0xAE: // XOR (HL)
            acc = a ^ memory[l | (h << 8)];
            flagsAfterCalcA();
            break;
        case 0xAF: // XOR A
            acc = a ^ a;
            flagsAfterCalcA();
            break;
        case 0xB0: // OR B
            acc = a | b;
            flagsAfterCalcA();
            break;
        case 0xB1: // OR C
            acc = a | c;
            flagsAfterCalcA();
            break;
        case 0xB2: // OR D
            acc = a | d;
            flagsAfterCalcA();
            break;
        case 0xB3: // OR E
            acc = a | e;
            flagsAfterCalcA();
            break;
        case 0xB4: // OR H
            acc = a | h;
            flagsAfterCalcA();
            break;
        case 0xB5: // OR L
            acc = a | l;
            flagsAfterCalcA();
            break;
        case 0xB6: // OR (HL)
            acc = a | memory[l | (h << 8)];
            flagsAfterCalcA();
            break;
        case 0xB7: // OR A
            acc = a | a;
            flagsAfterCalcA();
            break;
        case 0xB8: // CP B
            temp = a;
            acc = a - b;
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xB9: // CP C
            temp = a;
            acc = a - c;
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xBA: // CP D
            temp = a;
            acc = a - d;
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xBB: // CP E
            temp = a;
            acc = a - e;
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xBC: // CP H
            temp = a;
            acc = a - h;
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xBD: // CP L
            temp = a;
            acc = a - l;
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xBE: // CP (HL)
            temp = a;
            acc = a - memory[l | (h << 8)];
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xBF: // CP A
            temp = a;
            acc = a - a;
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xC0: // RET NZ
            if (!(f & 0x40)) {
                pc = memory[sp] | (memory[sp+1] << 8);
                sp += 2;
            }
            break;
        case 0xC1: // POP BC
            c = memory[sp];
            sp++;
            b = memory[sp];
            break;
        case 0xC2: // JP NZ, nn
            if (!(f & 0x40)) {
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xC3: // JP nn
            pc = (fetchOperand() | (fetchOperand() << 8));
            break;
        case 0xC4: // CALL NZ, nn
            pc += 3;
            if (!(f & 0x40)) {
                sp--;
                memory[sp] = pc >> 8; // high byte
                sp--;
                memory[sp] = pc & 0xFF; // low byte
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xC5: // PUSH BC
            sp--;
            memory[sp] = b;
            sp--;
            memory[sp] = c;
            break;
        case 0xC6: // ADD A, n
            acc += fetchOperand();
            flagsAfterCalcA();
            break;
        case 0xC7: // RST 00h
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = 0x00;
            break;
        case 0xC8: // RET Z
            if(f & 0x40){
                pc = memory[sp] | (memory[sp+1] << 8);
                sp += 2;
            }
            break;
        case 0xC9: // RET
            pc = memory[sp] | (memory[sp+1] << 8);
            sp += 2;
            break;
        case 0xCA: // JP Z, nn
            if(f & 0x40){
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xCB: //BIT INSTRUCTION
            //TODO
            break;
        case 0xCC: // CALL Z, nn
            pc += 3;
            if(f & 0x40){
                sp--;
                memory[sp] = pc >> 8; // high byte
                sp--;
                memory[sp] = pc & 0xFF; // low byte
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xCD: // CALL nn
            pc += 3;
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = (fetchOperand() | (fetchOperand() << 8));
            break;
        case 0xCE: // ADC A, n
            acc = a + fetchOperand() + (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0xCF: // RST 08h
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = 0x08;
            break;
        case 0xD0: // RET NC
            if(!(f & 0x01)){
                pc = memory[sp] | (memory[sp+1] << 8);
                sp += 2;
            }
            break;
        case 0xD1: // POP DE
            e = memory[sp];
            sp++;
            d = memory[sp];
            break;
        case 0xD2: // JP NC, nn
            if(!(f & 0x01)){
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xD3: // OUT (n), A
            outputHandler(a);
            fetchOperand();
            break;
        case 0xD4: // CALL NC, nn
            pc += 3;
            if(!(f & 0x01)){
                sp--;
                memory[sp] = pc >> 8; // high byte
                sp--;
                memory[sp] = pc & 0xFF; // low byte
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xD5: // PUSH DE
            sp--;
            memory[sp] = d;
            sp--;
            memory[sp] = e;
            break;
        case 0xD6: // SUB n
            acc = a - fetchOperand();
            flagsAfterCalcA();
            break;
        case 0xD7: // RST 10h
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = 0x10;
            break;
        case 0xD8: // RET C
            if(f & 0x01){
                pc = memory[sp] | (memory[sp+1] << 8);
                sp += 2;
            }
            break;
        case 0xD9: // EXX
            temp = (uint8_t)(bca >> 8);
            swapRegs(b, temp);
            temp = (uint8_t)(bca & 0xFF);
            swapRegs(c, temp);
            temp = (uint8_t)(dea >> 8);
            swapRegs(d, temp);
            temp = (uint8_t)(dea & 0xFF);
            swapRegs(e, temp);
            temp = (uint8_t)(hla >> 8);
            swapRegs(h, temp);
            temp = (uint8_t)(hla & 0xFF);
            swapRegs(l, temp);
            break;
        case 0xDA: // JP C, nn
            if(f & 0x01){
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xDB: // IN A, (n)
            a = inputHandler();
            fetchOperand();
            break;
        case 0xDC: // CALL C, nn
            pc += 3;
            if(f & 0x01){
                sp--;
                memory[sp] = pc >> 8; // high byte
                sp--;
                memory[sp] = pc & 0xFF; // low byte
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xDD: // IX PREFIX
            //TODO
            break;
        case 0xDE: // SBC A, n
            acc = a - fetchOperand() - (f & 0x01);
            flagsAfterCalcA();
            break;
        case 0xDF: // RST 18h
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = 0x18;
            break;
        case 0xE0: // RET PO
            if(!(f & 0x04)){
                pc = memory[sp] | (memory[sp+1] << 8);
                sp += 2;
            }
            break;
        case 0xE1: // POP HL
            l = memory[sp];
            sp++;
            h = memory[sp];
            break;
        case 0xE2: // JP PO, nn
            if(!(f & 0x04)){
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xE3: // EX (SP), HL
            temp = memory[sp];
            memory[sp] = l;
            l = temp;
            sp++;
            temp = memory[sp];
            memory[sp] = h;
            h = temp;
            break;
        case 0xE4: // CALL PO, nn
            pc += 3;
            if(!(f & 0x04)){
                sp--;
                memory[sp] = pc >> 8; // high byte
                sp--;
                memory[sp] = pc & 0xFF; // low byte
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xE5: // PUSH HL
            sp--;
            memory[sp] = h;
            sp--;
            memory[sp] = l;
            break;
        case 0xE6: // AND n
            acc = a & fetchOperand();
            flagsAfterCalcA();
            break;
        case 0xE7: // RST 20h
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = 0x20;
            break;
        case 0xE8: // RET PE
            if(!(f & 0x04)){
                pc = memory[sp] | (memory[sp+1] << 8);
                sp += 2;
            }
            break;
        case 0xE9: // JP (HL)
            pc = l | (h << 8);
            break;
        case 0xEA: // JP PE, nn
            if(!(f & 0x04)){
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xEB: // EX DE, HL
            temp = (uint8_t)(dea >> 8);
            swapRegs(d, temp);
            temp = (uint8_t)(dea & 0xFF);
            swapRegs (e, temp);
            temp = (uint8_t)(hla >> 8);
            swapRegs (h, temp);
            temp = (uint8_t)(hla & 0xFF);
            swapRegs (l, temp);
            break;
        case 0xEC: // CALL PE, nn
            pc += 3;
            if((f & 0x04)){
                sp--;
                memory[sp] = pc >> 8; // high byte
                sp--;
                memory[sp] = pc & 0xFF; // low byte
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xED: // ED PREFIX
            //TODO
            break;
        case 0xEE: // XOR n
            acc = a ^ fetchOperand();
            flagsAfterCalcA();
            break;
        case 0xEF: // RST 28h
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = 0x28;
            break;
        case 0xF0: // RET P
            if(!(f & 0x80)){
                pc = memory[sp] | (memory[sp+1] << 8);
                sp += 2;
            }
            break;
        case 0xF1: // POP AF
            f = memory[sp];
            sp++;
            a = memory[sp];
            break;
        case 0xF2: // JP P, nn
            if(!(f & 0x80)){
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xF3: // DI
            //TODO
            break;
        case 0xF4: // CALL P, nn
            pc += 3;
            if(!(f & 0x80)){
                sp--;
                memory[sp] = pc >> 8; // high byte
                sp--;
                memory[sp] = pc & 0xFF; // low byte
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xF5: // PUSH AF
            sp--;
            memory[sp] = a;
            sp--;
            memory[sp] = f;
            break;
        case 0xF6: // OR n
            acc = a | fetchOperand();
            flagsAfterCalcA();
            break;
        case 0xF7: // RST 30h
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = 0x30;
            break;
        case 0xF8: // RET M
            if(!(f & 0x80)){
                pc = memory[sp] | (memory[sp+1] << 8);
                sp += 2;
            }
            break;
        case 0xF9: // LD SP, HL
            sp = l | (h << 8);
            break;
        case 0xFA: // JP M, nn
            if((f & 0x80)){
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xFB: // EI
            //TODO
            break;
        case 0xFC: // CALL M, nn
            pc += 3;
            if((f & 0x80)){
                sp--;
                memory[sp] = pc >> 8; // high byte
                sp--;
                memory[sp] = pc & 0xFF; // low byte
                pc = (fetchOperand() | (fetchOperand() << 8));
            }
            break;
        case 0xFD: // IY PREFIX
            //TODO
            break;
        case 0xFE: // CP n
            temp = a;
            acc = a - fetchOperand();
            flagsAfterCalcA();
            a = temp;
            break;
        case 0xFF: // RST 38h
            sp--;
            memory[sp] = pc >> 8; // high byte
            sp--;
            memory[sp] = pc & 0xFF; // low byte
            pc = 0x38;
            break;

    }
}

void Z80_Core::ed_instruction(uint8_t ins) {
    switch (ins) {
        case 0x40: // IN B, (C)
            b = inputHandler();
            break;
        case 0x41: // OUT (C), B
            outputHandler(b);
            break;
    }
}