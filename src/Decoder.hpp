#ifndef DECODER_HPP
#define DECODER_HPP

#include <cstdint>

enum instructionName 
{
  RET,
  // R-type
  ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
  // I-type
  JALR, LB, LH, LW, LBU, LHU, ADDI, SLTI, SLTIU, ORI, XORI, ANDI, SLLI, SRLI, SRAI,
  // S-type
  SB, SH, SW, 
  // B-type
  BEQ, BNE, BLT, BGE, BLTU, BGEU,
  // U-type
  LUI, AUIPC,
  // J-type
  JAL
};

struct Instruction
{
  instructionName name;
  int rs1 = 0;
  int rs2 = 0;
  int rd  = 0;
  int imm = 0;
  bool busy;
};

class Decoder
{ 
public:
  uint8_t ins[4];
  Instruction output;
  Decoder() = default;
  void decode();
};

void Decoder::decode() {
  Instruction instruction;
  int opcode = ins[0] & 0x7F;
  if (opcode == 0b0110011) {
    instruction.rd = ins[0] >> 7;
    instruction.rd += (ins[1] & 0x0F) << 1;
    int funct3 = (ins[1] >> 4) & 0b0111;
    instruction.rs1 = ins[1] >> 7;
    instruction.rs1 += (ins[2] & 0x0F) << 1;
    instruction.rs2 = ins[2] >> 4;
    instruction.rs2 += (ins[3] & 0x01) << 4;
    int funct7 = ins[3] >> 1;
    if (funct3 == 0b000) {
      if (funct7 == 0) {
        instruction.name = ADD;
      } else if (funct7 == 0b0100000) {
        instruction.name = SUB;
      }
    } else if (funct3 == 0b001) {
      if (funct7 == 0) {
        instruction.name = SLL;
      }
    } else if (funct3 == 0b010) {
      if (funct7 == 0) {
        instruction.name = SLT;
      }
    } else if (funct3 == 0b011) {
      if (funct7 == 0) {
        instruction.name = SLTU;
      }
    } else if (funct3 == 4) {
      if (funct7 == 0) {
        instruction.name = XOR;
      }
    } else if (funct3 == 0b101) {
      if (funct7 == 0){
        instruction.name == SRL;
      } else if (funct7 == 0b0100000) {
        instruction.name = SRA;
      }
    } else if (funct3 == 0b110) {
      if (funct7 == 0) {
        instruction.name = OR;
      }
    } else if (funct3 == 0b111) {
      if (funct7 == 0) {
        instruction.name = AND;
      }
    }
  } else if (opcode == 0b1100111 || opcode == 0b0000011 || opcode == 0b0010011) {
    instruction.rd = ins[0] >> 7;
    instruction.rd += (ins[1] & 0x0F) << 1;
    int funct3 = (ins[1] >> 4) & 0b0111;
    instruction.rs1 = ins[1] >> 7;
    instruction.rs1 += (ins[2] & 0x0F) << 1;
    instruction.imm = ins[2] >> 4;
    instruction.imm += ins[3] << 4;
    if (instruction.imm >> 11) {
      instruction.imm |= 0xfffff000;
    }
    if (opcode == 0b1100111) {
      if (funct3 == 0) {
        instruction.name = JALR;
      }
    } else if (opcode == 0b0000011) {
      if (funct3 == 0b000) {
        instruction.name = LB;
      } else if (funct3 == 0b001) {
        instruction.name = LH;
      } else if (funct3 == 0b010) {
        instruction.name = LW;
      } else if (funct3 == 0b100) {
        instruction.name = LBU;
      } else if (funct3 == 0b101) {
        instruction.name = LHU;
      }
    } else if (opcode == 0b0010011) {
      if (funct3 == 0b000) {
        instruction.name = ADDI;
        if (ins[0] == 0x13 && ins[1] == 0x5 && ins[2] == 0xf0 && ins[3] == 0xf) {
          instruction.name = RET;
        }
      } else if (funct3 == 0b001) {
        if ((instruction.imm >> 5) == 0) {
          instruction.name = SLLI;
        }
        
      } else if (funct3 == 0b010) {
        instruction.name = SLTI;
      } else if (funct3 == 0b011) {
        instruction.name = SLTIU;
      } else if (funct3 == 0b100) {
        instruction.name = XORI;
      } else if (funct3 == 0b101) {
        if ((instruction.imm >> 5) == 0) {
          instruction.name = SRLI;
        } else if ((instruction.imm >> 5) == 0b0100000) {
          instruction.imm &= 0b11111;
          instruction.name = SRAI;
        }
      } else if (funct3 == 0b110) {
        instruction.name = ORI;
      } else if (funct3 == 0b111) {
        instruction.name = ANDI;
      }
    }
  } else if (opcode == 0b0100011) {
    instruction.imm = ins[0] >> 7;
    instruction.imm += (ins[1] & 0b1111) << 1;
    instruction.imm += (ins[3] >> 1) << 5;
    if (instruction.imm >> 11) {
      instruction.imm |= 0xfffff000;
    }
    int funct3 = (ins[1] >> 4) & 0b0111;
    instruction.rs1 = ins[1] >> 7;
    instruction.rs1 += (ins[2] & 0x0F) << 1;
    instruction.rs2 = ins[2] >> 4;
    instruction.rs2 += (ins[3] & 0x01) << 4;
    if (funct3 == 0b000) {
      instruction.name = SB;
    } else if (funct3 == 0b001) {
      instruction.name = SH;
    } else if (funct3 == 0b010) {
      instruction.name = SW;
    }
  } else if (opcode == 0b1100011) {
    instruction.imm = (ins[0] >> 7) << 11;
    instruction.imm += (ins[1] & 0b1111) << 1;
    instruction.imm += ((ins[3] >> 1) & 0b111111) << 5;
    instruction.imm += (ins[3] >> 7) << 12;
    if (instruction.imm >> 12) {
      instruction.imm |= 0xffffe000;
    }
    int funct3 = (ins[1] >> 4) & 0b0111;
    instruction.rs1 = ins[1] >> 7;
    instruction.rs1 += (ins[2] & 0x0F) << 1;
    instruction.rs2 = ins[2] >> 4;
    instruction.rs2 += (ins[3] & 0x01) << 4;
    if (funct3 == 0b000) {
      instruction.name = BEQ;
    } else if (funct3 == 0b001) {
      instruction.name = BNE;
    } else if (funct3 == 0b100) {
      instruction.name = BLT;
    } else if (funct3 == 0b101) {
      instruction.name = BGE;
    } else if (funct3 == 0b110) {
      instruction.name = BLTU;
    } else if (funct3 == 0b111) {
      instruction.name = BGEU;
    }
  } else if (opcode == 0b0110111 || opcode == 0b0010111) {
    instruction.rd = ins[0] >> 7;
    instruction.rd += (ins[1] & 0x0F) << 1;
    instruction.imm = (ins[1] >> 4) << 12;
    instruction.imm += ins[2] << 16;
    instruction.imm += ins[3] << 24;
    if (opcode == 0b0110111) {
      instruction.name = LUI;
    } else if (opcode == 0b0010111) {
      instruction.name = AUIPC;
    }
  } else if (opcode == 0b1101111) {
    instruction.rd = ins[0] >> 7;
    instruction.rd += (ins[1] & 0x0F) << 1;
    instruction.imm = (ins[1] >> 4) << 12;
    instruction.imm += (ins[2] & 0b1111) << 16;
    instruction.imm += ((ins[2] >> 4) & 1) << 11;
    instruction.imm += (ins[2] >> 5) << 1;
    instruction.imm += (ins[3] & 0x7F) << 4;
    instruction.imm += (ins[3] >> 7) << 20;
    if (instruction.imm >> 20) {
      instruction.imm |= 0xffe00000;
    }
    instruction.name = JAL;
  }
  output = instruction;
  output.busy = true;
}

#endif