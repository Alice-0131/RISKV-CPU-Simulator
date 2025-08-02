#include <iostream>
#include "CPU.hpp"

int main() {
  //freopen("a.out", "w", stdout);
  CPU cpu;
  cpu.read();
  std::cout << cpu.run();
  return 0;
}