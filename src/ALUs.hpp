#ifndef ALUS_HPP
#define ALUS_HPP

#include <cstdint>

struct ALUin
{
  instructionName ins = RET;
  int left = 0;
  int right = 0;
  bool busy = false;
  int ind = 0;
};

struct ALUout
{
  bool busy = false;
  int ind = 0;
  instructionName name = RET;
  uint32_t value = 0;
};


class ALU
{ 
public:
  ALUin input;
  ALUout output;
  ALU() = default;
  void calculate();
};

void ALU::calculate() {
  if (input.busy && !output.busy) {
    output.ind = input.ind;
    output.name = input.ins;
    output.busy = true;
    input.busy = false;
    if (input.ins == ADD || input.ins == ADDI || input.ins == RET || input.ins == LB 
      || input.ins == LBU || input.ins == LH || input.ins == LHU || input.ins == LW 
      || input.ins == SB || input.ins == SH || input.ins == SW) {
      output.value = input.left + input.right;
    } else if (input.ins == SUB) {
      output.value = input.left - input.right;
    } else if (input.ins == AND || input.ins == ANDI) {
      output.value = input.left & input.right;
    } else if (input.ins == OR || input.ins == ORI) {
      output.value = input.left | input.right;
    } else if (input.ins == XOR || input.ins == XORI) {
      output.value = input.left ^ input.right;
    } else if (input.ins == SLL || input.ins == SLLI) {
      output.value = input.left << input.right;
    } else if (input.ins == SRA || input.ins == SRAI) {
      output.value = input.left >> input.right;
    } else if (input.ins == SRL || input.ins == SRLI) {
      output.value = static_cast<unsigned int>(input.left) >> input.right;
    } else if (input.ins == SLT || input.ins == SLTI) {
      output.value = (input.left < input.right) ? 1 : 0;
    } else if (input.ins == SLTU || input.ins == SLTIU) {
      output.value = (static_cast<unsigned int>(input.left) < static_cast<unsigned int>(input.right)) ? 1 : 0;
    } else if (input.ins == BEQ) {
      output.value = input.left == input.right;
    } else if (input.ins == BGE) {
      output.value = input.left >= input.right;
    } else if (input.ins == BGEU) {
      output.value = static_cast<unsigned int>(input.left) >= static_cast<unsigned int>(input.right);
    } else if (input.ins == BLT) {
      output.value = input.left < input.right;
    } else if (input.ins == BLTU) {
      output.value = static_cast<unsigned int>(input.left) < static_cast<unsigned int>(input.right);
    } else if (input.ins == BNE) {
      output.value = input.left != input.right;
    }
  }
}

#endif