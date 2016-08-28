#ifdef INSTR

#define SHL(reg,imm)        (0x0000 + (reg << 5) + ((uint8_t)imm & 0x1F))                       //GN     (0b0000000aaaaiiiii) 
#define SHR(reg,imm)        (0x0200 + (reg << 5) + ((uint8_t)imm & 0x1F))                       //GN     (0b0000001aaaaiiiii)
#define ROR(reg,imm)        (0x0400 + (reg << 5) + ((uint8_t)imm & 0x1F))                       //GN     (0b0000010aaaaiiiii)
#define XBN(reg,imm)        (0x0600 + (reg << 5) + ((uint8_t)imm & 0x1F))                       //GN     (0b0000011aaaaiiiii)
#define JR(reg)             (0x0800 + reg)                                                      //G1     (0b000010000000rrrr)
#define JALR(reg)           (0x0900 + reg)                                                      //G1     (0b000010010000rrrr)
#define STL(reg1,reg2)      (0x1000 + (reg1 << 4) + reg2)                                       //G2     (0b00010000aaaabbbb)
#define STH(reg1,reg2)      (0x1100 + (reg1 << 4) + reg2)                                       //G2     (0b00010001aaaabbbb)
#define STW(reg1,reg2)      (0x1200 + (reg1 << 4) + reg2)                                       //G2     (0b00010010aaaabbbb)
#define CMP(reg1,reg2)      (0x1300 + (reg1 << 4) + reg2)                                       //G2     (0b00010011aaaabbbb)
#define LDL(reg1,reg2)      (0x1400 + (reg1 << 4) + reg2)                                       //G2     (0b00010100aaaabbbb)
#define LDH(reg1,reg2)      (0x1500 + (reg1 << 4) + reg2)                                       //G2     (0b00010101aaaabbbb)
#define LDW(reg1,reg2)      (0x1600 + (reg1 << 4) + reg2)                                       //G2     (0b00010110aaaabbbb)  
#define OUT(reg1,reg2)      (0x1700 + (reg1 << 4) + reg2)                                       //G2     (0b00010111aaaabbbb)
#define MUL(reg1,reg2)      (0x1800 + (reg1 << 4) + reg2)                                       //G2     (0b00011000aaaabbbb)
#define AND(reg1,reg2)      (0x1900 + (reg1 << 4) + reg2)                                       //G2     (0b00011001aaaabbbb)
#define OR(reg1,reg2)       (0x1A00 + (reg1 << 4) + reg2)                                       //G2     (0b00011010aaaabbbb)
#define XOR(reg1,reg2)      (0x1B00 + (reg1 << 4) + reg2)                                       //G2     (0b00011011aaaabbbb)
#define BIC(reg1,reg2)      (0x1C00 + (reg1 << 4) + reg2)                                       //G2     (0b00011100aaaabbbb)
#define NOT(reg1,reg2)      (0x1D00 + (reg1 << 4) + reg2)                                       //G2     (0b00011101aaaabbbb)
#define SH(reg1,reg2)       (0x1E00 + (reg1 << 4) + reg2)                                       //G2     (0b00011110aaaabbbb)
#define IN(reg1,reg2)       (0x1F00 + (reg1 << 4) + reg2)                                       //G2     (0b00011111aaaabbbb)
#define ADD(reg1,reg2,reg3) (0x2000 + (reg1 << 8) + (reg2 << 4) + reg3)                         //G3     (0b0010aaaabbbbcccc)
#define SUB(reg1,reg2,reg3) (0x3000 + (reg1 << 8) + (reg2 << 4) + reg3)                         //G3     (0b0011aaaabbbbcccc)
#define MOVB(off, reg,imm)  (0x4000 + (((uint8_t)off & 0x3) << 12) + (reg << 8) + (uint8_t)imm) //MB     (0b01oorrrriiiiiiii)
#define ADDB(reg,imm)       (0x8000 + (reg << 8) + (uint8_t)imm)                                //GN     (0b1000rrrriiiiiiii)
#define SUBB(reg,imm)       (0x9000 + (reg << 8) + (uint8_t)imm)                                //GN     (0b1001rrrriiiiiiii)
#define B(addr)             (0xA000 + (((uint16_t)addr) & 0x0FFF))                              //AD     (0b1010aaaaaaaaaaaa)
#define BAL(addr)           (0xB000 + (((uint16_t)addr) & 0x0FFF))                              //AD     (0b1011aaaaaaaaaaaa)
#define BZ(addr)            (0xC000 + (((uint16_t)addr) & 0x0FFF))                              //AD     (0b1100aaaaaaaaaaaa)
#define BNZ(addr)           (0xD000 + (((uint16_t)addr) & 0x0FFF))                              //AD     (0b1101aaaaaaaaaaaa)
#define BLZ(addr)           (0xE000 + (((uint16_t)addr) & 0x0FFF))                              //AD     (0b1110aaaaaaaaaaaa)
#define BGEZ(addr)          (0xF000 + (((uint16_t)addr) & 0x0FFF))                              //AD     (0b1111aaaaaaaaaaaa)

#define TOKEN(n)    (tokens.at(tokensPos + n))
#define REG(n)      (TOKEN(n)->Num)

#define IS_REG(n)           (TOKEN(n)->Type == TokenType::Reg)

#define IS_IMM(n)           (TOKEN(n)->Type == TokenType::Num)

#define G3(instr)                                                                                   \
    if (!IS_REG(1) || !IS_REG(2) || !IS_REG(3)) return false;                                       \
    *binDataPos = instr(REG(1), REG(2), REG(3));                                                    \
    tokensPos += 3;      

#define G2(instr)                                                                                   \
    if (!IS_REG(1) || !IS_REG(2)) return false;                                                     \
    *binDataPos = instr(REG(1), REG(2));                                                            \
    tokensPos += 2;    

#define G1(instr)                                                                                   \
    if (!IS_REG(1)) return false;                                                                   \
    *binDataPos = instr(REG(1));                                                                    \
    tokensPos += 1;    

#define GN(instr)                                                                                   \
    if (!IS_REG(1) || !IS_IMM(2)) return false;                                                     \
    *binDataPos = instr(REG(1), TOKEN(2)->Num);                                                     \
    tokensPos += 2;

#define AD(instr)                                                                                   \
    if (TOKEN(1)->Type != TokenType::FromLabel) return false;                                       \
    *binDataPos = instr((TOKEN(1)->Num));                                                           \
    tokensPos++;                                                             

#define MB(instr)                                                                                    \
    if (!IS_IMM(1) || !IS_REG(2) || !IS_IMM(3)) return false;                                        \
    *binDataPos = instr(TOKEN(1)->Num, REG(2), TOKEN(3)->Num);                                       \
    tokensPos += 3;

#endif
