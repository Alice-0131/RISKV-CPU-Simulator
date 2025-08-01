#ifndef CPU_HPP
#define CPU_HPP

#include "Decoder.hpp"
#include "ALUs.hpp"
#include "LSB.hpp"
#include "Memory.hpp"
#include "Predictor.hpp"
#include "RF.hpp"
#include "RoB.hpp"
#include "RS.hpp"
#include <iostream>
#include <cstdint>

class CPU
{
private:
  int       clk = 0;
  int       pc = 0;
  Decoder   decoder;
  RoB       rob;
  RS        rs;
  RF        rf;
  LSB       lsb;
  Memory    memory;
  Predictor predictor;
  ALU       alu;
  bool      stop = false;
  bool      running = true;

  void transfer();
  void work();
  void flush();
  
public:
  CPU() = default;
  void advance();
  void read();
  int run();
};

int CPU::run() {
  while (running) {
    advance();
  }
  return rf.reg[10] & 0b11111111;
}

void CPU::advance() {
  ++clk;
  transfer();
  work();
}

void CPU::read() {
  std::string line;
  int pos = 0;
  while (true) {
    if (!getline(std::cin, line)) {
      break;
    }
    if (line[0] == '@') {
      std::string sub = line.substr(1, 8);
      pos = std::stoi(sub, nullptr, 16);
    } else {
      int p = 0;
      while (p < line.size()) {
        std::string sub = line.substr(p, 2);
        memory.mem[pos] = std::stoi(sub, nullptr, 16);
        p += 3;
        ++pos;
      }
    }
  }
}

void CPU::transfer() {
  // mem -> decoder
  if (!stop) {
    decoder.ins[0] = memory.mem[pc];
    decoder.ins[1] = memory.mem[pc + 1];
    decoder.ins[2] = memory.mem[pc + 2];
    decoder.ins[3] = memory.mem[pc + 3];
    decoder.decode();
    // decoder -> rob
    if (decoder.output.name == RET) {
      stop = true;
    }
    if (decoder.output.busy && !rob.decoderinput.busy) {
      decoder.output.busy = false;
      rob.decoderinput.busy = true;
      rob.decoderinput.name = decoder.output.name;
      if (decoder.output.name == JAL) {
        rob.decoderinput.pc = pc;
        pc += decoder.output.imm;
        rob.decoderinput.imm = decoder.output.imm;
        rob.decoderinput.rd = decoder.output.rd;
      } else if (decoder.output.name == JALR) {
        stop = true;
        rob.decoderinput.pc = pc;
        rob.decoderinput.rd = decoder.output.rd;
        rob.decoderinput.rs1 = decoder.output.rs1;
        rob.decoderinput.rs2 = decoder.output.rs2;
        rob.decoderinput.imm = decoder.output.imm;
        pc += 4;
      } else if (decoder.output.name == AUIPC || decoder.output.name == LUI) {
        rob.decoderinput.pc = pc;
        rob.decoderinput.rd = decoder.output.rd;
        rob.decoderinput.imm = decoder.output.imm;
        pc += 4;
      } else {
        rob.decoderinput.pc = pc;
        rob.decoderinput.rd = decoder.output.rd;
        rob.decoderinput.rs1 = decoder.output.rs1;
        rob.decoderinput.rs2 = decoder.output.rs2;
        rob.decoderinput.imm = decoder.output.imm;
        pc += 4;
      }
    }
  } else {
    if (decoder.output.name == JALR && rf.rely[decoder.output.rs1] == -1) {
      stop = false;
      pc = rf.reg[decoder.output.rs1] + decoder.output.imm;
    }
  }
  // rob -> rs(issue)
  if (rob.issueout.busy && !rs.issue_in.busy) {
    rob.issueout.busy = false;
    rs.issue_in.busy = true;
    rs.issue_in.name = rob.issueout.name;
    rs.issue_in.imm = rob.issueout.imm;
    rs.issue_in.ind = rob.issueout.ind;
    if (rf.rely[rob.issueout.rs1] == -1) {
      rs.issue_in.Vi = rf.reg[rob.issueout.rs1];
      rs.issue_in.Qi = -1;
    } else {
      rs.issue_in.Vi = 0;
      rs.issue_in.Qi = rf.rely[rob.issueout.rs1];
    }
    if (rf.rely[rob.issueout.rs2] == -1) {
      rs.issue_in.Vj = rf.reg[rob.issueout.rs2];
      rs.issue_in.Qj = -1;
    } else {
      rs.issue_in.Vi = 0;
      rs.issue_in.Qj = rf.rely[rob.issueout.rs2];
    }
    rf.rely[rob.issueout.rd] = rob.issueout.ind;
  }
  // rob -> rs(write&broadcast)
  if (rob.wbout.busy && !rs.commit_in.busy) {
    rob.wbout.busy = false;
    rs.commit_in.busy = true;
    rs.commit_in.ind = rob.wbout.ind;
    rs.commit_in.value = rob.commitout.value;
  }
  // rob -> rf(commit)
  if (rob.flag){
    lsb.ready[rob.ind] = true;
  }
  if (rob.commitout.busy) {
    switch (rob.commitout.name)
    {
    case ADD: case SUB: case AND: case OR: case XOR: case SLL: case SRL: case SRA: case SLT: case SLTU:
    case ADDI: case ANDI: case ORI: case XORI: case SLLI: case SRLI: case SRAI: case SLTI: case SLTIU:
    case LB: case LBU: case LH: case LHU: case LW: case JAL: case JALR: case AUIPC: case LUI:{
      rf.reg[rob.commitout.rd] = rob.commitout.value;
      for (int i = 0; i < 32; ++i) {
        if (rf.rely[i] == rob.commitout.ind) {
          rf.rely[i] = -1;
          rf.reg[i] = rob.commitout.value;
        }
      }
      break;
    }
    case BEQ: case BGE: case BGEU: case BLT: case BLTU: case BNE: {
      if (rob.commitout.value) { // predict wrongly
        flush();
        pc = rob.commitout.pc + rob.commitout.imm;
      }
      break;
    }
    case RET: running = false;
    default:
      break;
    }
    rob.commitout.busy = false;
  }
  // rs -> lsb/alu
  if (rs.output.busy) {
    rs.output.busy = false;
    switch (rs.output.name)
    {
    case ADD: case SUB: case AND: case OR: case XOR: case SLL: case SRL: case SRA: case SLT: case SLTU:{
      if (!alu.input.busy) {
        alu.input.busy = true;
        alu.input.ind = rs.output.ind;
        alu.input.ins = rs.output.name;
        alu.input.left = rs.output.left;
        alu.input.right = rs.output.right;
      }
      break;
    }
    case ADDI: case ANDI: case ORI: case XORI: case SLLI: case SRLI: case SRAI: case SLTI: case SLTIU: {
      if (!alu.input.busy) {
        alu.input.busy = true;
        alu.input.ind = rs.output.ind;
        alu.input.ins = rs.output.name;
        alu.input.left = rs.output.left;
        alu.input.right = rs.output.imm;
      }
      break;
    }
    case LB: case LBU: case LH: case LHU: case LW: {
      if (!lsb.input.busy) {
        lsb.input.busy = true;
        lsb.input.name = rs.output.name;
        lsb.input.ind = rs.output.ind;
        lsb.input.addr = rs.output.imm + rs.output.left;
      }
      break;
    }
    case SB: case SH: case SW: {
      if (!lsb.input.busy) {
        lsb.input.busy = true;
        lsb.input.name = rs.output.name;
        lsb.input.ind = rs.output.ind;
        lsb.input.addr = rs.output.imm + rs.output.left;
        lsb.input.value = rs.output.right;
      }
      break;
    }
    default:
      break;
    }
  }
  // lsb -> mem
  if (lsb.output.busy && !memory.input.busy) {
    lsb.output.busy = false;
    memory.input.busy = true;
    memory.input.ind = lsb.output.ind;
    memory.input.name = lsb.output.name;
    memory.input.addr = lsb.output.addr;
    memory.input.value = lsb.output.value;
  }
  // mem -> rob
  if (memory.output.busy && !rob.ALULSBinput.busy){
    memory.output.busy = false;
    rob.ALULSBinput.busy = true;
    rob.ALULSBinput.ind = memory.output.ind;
    rob.ALULSBinput.name = memory.output.name;
    rob.ALULSBinput.value = memory.output.value;
  }
  // alu -> rob
  if (alu.output.busy && !rob.ALULSBinput.busy) {
    alu.output.busy = false;
    rob.ALULSBinput.busy = true;
    rob.ALULSBinput.ind = alu.output.ind;
    rob.ALULSBinput.name = alu.output.name;
    rob.ALULSBinput.value = alu.output.value;
  }
}

void CPU::work() {
  rob.work();
  rs.work();
  alu.calculate();
  lsb.work();
  memory.work();
  rf.reg[0] = 0;
  rf.rely[0] = -1;
}

void CPU::flush() {
  rob.flush();
  rs.flush();
  lsb.flush();
  rf.flush();
}

#endif