#ifndef RS_HPP
#define RS_HPP

const int size_rs = 28;

struct Issinput
{
  bool busy;
  instructionName name;
  int Qi, Qj, Vi, Vj;
  int imm;
  int ind;
};

struct RSInfo
{
  instructionName name;
  int Qi;
  int Qj;
  int Vi;
  int Vj;
  int imm;
  int ind;

  RSInfo &operator=(const Issinput& in) {
    name = in.name;
    Qi = in.Qi;
    Qj = in.Qj;
    Vi = in.Vi;
    Vj = in.Vj;
    imm = in.imm;
    ind = in.ind;
    return *this;
  }
};

struct Cominput
{
  bool busy;
  int value;
  int ind;
};

struct RSout
{
  bool busy;
  instructionName name;
  int left, right;
  int imm;
  int ind;

  RSout &operator=(const RSInfo& in) {
    busy = true;
    name = in.name;
    left = in.Vi;
    right = in.Vj;
    imm = in.imm;
    ind = in.ind;
    return *this;
  }
};

class RS
{
private:
  RSInfo queue[size_rs];
  int cur_size = 0;

public:
  Issinput  issue_in;
  Cominput commit_in;
  RSout   output;

  RS() = default;
  void work();
  void flush();
};

void RS::work(){
  // update dependency
  if (commit_in.busy) {
      if (issue_in.busy && issue_in.Qi == commit_in.ind) {
      issue_in.Vi = commit_in.value;
      issue_in.Qi = -1;
    }
    if (issue_in.busy && issue_in.Qj == commit_in.ind) {
      issue_in.Vj = commit_in.value;
      issue_in.Qj = -1;
    }
    for (int i = 0; i < cur_size; ++i) {
      if (queue[i].Qi == commit_in.ind) {
        queue[i].Vi = commit_in.value;
        queue[i].Qi = -1;
      }
      if (queue[i].Qj == commit_in.ind) {
        queue[i].Vj = commit_in.value;
        queue[i].Qj = -1;
      }
    }
    commit_in.busy = false;
  }
  // add instruction
  if (issue_in.busy && cur_size < size_rs) {
    queue[cur_size] = issue_in;
    cur_size = cur_size + 1;
    issue_in.busy = false;
  }
  // execute the instruction
  for (int i = 0; i < cur_size; ++i) {
    if (queue[i].Qi == -1 && queue[i].Qj == -1) { // dequeue
      output = queue[i];
      --cur_size;
      for (int j = i; j < cur_size; ++j) {
        queue[j] = queue[j + 1];
      }
      break;
    }
  }
}

void RS::flush() {
  cur_size = 0;
}

#endif