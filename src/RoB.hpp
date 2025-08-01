#ifndef ROB_HPP
#define ROB_HPP


const int size_rob = 100;

struct DetoRoB
{
  bool busy;
  instructionName name;
  int rs1;
  int rs2;
  int imm;
  int rd;
  int pc;
};

struct RoBInfo
{
  instructionName name;
  bool ready;
  int rd;
  int value;
  int pc;
  int imm;

  RoBInfo &operator=(const DetoRoB& in) {
    name = in.name;
    ready = false;
    rd = in.rd;
    pc = in.pc;
    imm = in.imm;
    return *this;
  }
};

struct A_LtoRoB
{
  bool busy;
  instructionName name;
  int value;
  int ind;
};

struct Issoutput
{
  bool busy;
  instructionName name;
  int rs1;
  int rs2;
  int imm;
  int pc;
  int ind;
  int rd;

  Issoutput &operator=(const DetoRoB& in) {
    busy = true;
    name = in.name;
    rs1 = in.rs1;
    rs2 = in.rs2;
    imm = in.imm;
    pc = in.pc;
    rd = in.rd;
    return *this;
  }
};

struct Comoutput
{
  bool busy;
  instructionName name;
  int rd;
  int value;
  int ind;
  int pc;
  int imm;

  Comoutput &operator=(const RoBInfo& in) {
    busy = true;
    rd = in.rd;
    value = in.value;
    name = in.name;
    pc = in.pc;
    imm = in.imm;
    return *this;
  }
};

class RoB
{
private:
  RoBInfo queue[size_rob];
  int head;
  int tail;

public:
  DetoRoB   decoderinput;
  A_LtoRoB  ALULSBinput;
  Issoutput issueout;
  Comoutput commitout;
  Comoutput wbout;
  bool flag = false;
  int ind = 0;

  RoB() = default;
  void work();
  void flush();
};

void RoB::work(){
  if (queue[head].name == SB || queue[head].name == SH || queue[head].name == SW) {
    flag = true;
    ind = head;
  } else {
    flag = false;
  }
  // enqueue
  if (decoderinput.busy) {
    queue[tail] = decoderinput;
    tail = (tail + 1) % size_rob;
    switch (decoderinput.name)
    {
    case JAL: {
      queue[tail - 1].value = decoderinput.pc + 4;
      queue[tail - 1].ready = true;
      break;
    }
    case JALR: {
      queue[tail - 1].value = decoderinput.pc + 4;
      queue[tail - 1].ready = true;
      break;
    }
    case AUIPC: {
      queue[tail - 1].value = decoderinput.pc + decoderinput.imm;
      queue[tail - 1].ready = true;
      break;
    }
    case LUI: {
      queue[tail - 1].value = decoderinput.imm;
      queue[tail - 1].ready = true;
      break;
    }
    default:
      issueout = decoderinput;
      issueout.ind = tail - 1;
      break;
    }
    decoderinput.busy = false;
  }
  // transmit value
  if (ALULSBinput.busy && !wbout.busy) {
    queue[ALULSBinput.ind].value = ALULSBinput.value;
    queue[ALULSBinput.ind].ready = true;
    wbout.busy = true;
    wbout.name = ALULSBinput.name;
    wbout.rd = queue[ALULSBinput.ind].rd;
    wbout.ind = ALULSBinput.ind;
    wbout.value = ALULSBinput.value;
    ALULSBinput.busy = false;
  }
  // dequeue
  if (queue[head].ready && !commitout.busy) {
    commitout = queue[head];
    commitout.ind = head;
    head = (head + 1) % size_rob;
  }
}

void RoB::flush() {
  head = tail = 0;
}
#endif