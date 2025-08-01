#ifndef RF_HPP
#define RF_HPP

class RF
{
public:
  RF(){
    for (int i = 0; i < 32; ++i) {
      rely[i] = -1;
    }
  }
  int reg[32];
  int rely[32];
  void flush() {
    for (int i = 0; i < 32; ++i) {
      rely[i] = -1;
    }
  }
};

#endif