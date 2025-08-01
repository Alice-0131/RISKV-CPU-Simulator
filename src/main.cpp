#include <iostream>
#include "CPU.hpp"

int main() {
  CPU cpu;
  cpu.read();
  std::cout << cpu.run();
  return 0;
}