#ifndef LSB_HPP
#define LSB_HPP

#include "RoB.hpp"

const int size_lsb = 28;

struct LSBInfo
{
  bool busy = 0;
  instructionName name = RET;
  int addr = 0;
  int value = 0;
  int ind = 0;
};

class LSB
{
private:
  int cntdown = 3;
  LSBInfo queue[size_lsb];
  int tail;
  int head;
  int size;

public:
  LSBInfo input;
  LSBInfo output;
  bool ready[size_rob] = {false};

  LSB() = default;
  void work();
  void flush();
};

void LSB::work() {
  if (input.busy && size < size_lsb) {
    queue[tail] = input;
    tail  = (tail + 1) % size_lsb;
    ++size;
    input.busy = false;
    ready[input.ind] = false;
  }
  if (size > 0 && (queue[head].name == LB || queue[head].name == LBU || queue[head].name == LH 
    || queue[head].name == LHU || queue[head].name == LW || 
    ready[queue[head].ind] && (queue[head].name == SB || queue[head].name == SW || queue[head].name == SH))) {
      if (!cntdown && !output.busy) {
        cntdown = 3;
        output = queue[head];
        head = (head + 1) % size_lsb;
        --size;
        ready[queue[head].ind] = false;
      } else {
        --cntdown;
      }
  }
}

void LSB::flush() {
  tail = head = size = 0;
  for (int i = 0; i < size_rob; ++i) {
    ready[i] = false;
  }
  cntdown = 3;
  input = LSBInfo();
  output = LSBInfo();
}
#endif