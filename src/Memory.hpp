#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <iostream>
#include <string>
#include <cstdint>
#include "RF.hpp"

const int size_mem = 1500000;

class Memory
{
public:
  uint8_t mem[size_mem] = {0};
  LSBInfo input;
  LSBInfo output;
  Memory() = default;
  void work() {
    if (input.busy && !output.busy) {
      output.name = input.name;
      output.ind = input.ind;
      if (input.name == LB || input.name == LBU) {
        output.value = mem[input.addr];
      } else if (input.name == LH || input.name == LHU) {
        output.value = mem[input.addr];
        output.value = output.value << 8;
        output.value += mem[input.addr + 1];
      } else if (input.name == LW) {
        output.value = mem[input.addr];
        output.value = output.value << 8;
        output.value += mem[input.addr + 1];
        output.value = output.value << 8;
        output.value += mem[input.addr + 2];
        output.value = output.value << 8;
        output.value += mem[input.addr + 3];
      } else if (input.name == SB) {
        mem[input.addr] = input.value;
      } else if (input.name == SH) {
        mem[input.addr + 1] = input.value & 0xff;
        input.value = input.value >> 8;
        mem[input.addr] = input.value & 0xff;
      } else if (input.name == SW) {
        mem[input.addr + 3] = input.value & 0xff;
        input.value = input.value >> 8;
        mem[input.addr + 2] = input.value & 0xff;
        input.value = input.value >> 16;
        mem[input.addr + 1] = input.value & 0xff;
        input.value = input.value >> 24;
        mem[input.addr] = input.value & 0xff;
      }
      input.busy = false;
      output.busy = true;
    }
    
  }

  void flush() {
    input = LSBInfo();
    output = LSBInfo();
  }
};

#endif