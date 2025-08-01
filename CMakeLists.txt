cmake_minimum_required(VERSION 3.22)
project(RISKV-Simulator)

set(CMAKE_CXX_STANDARD 20)

add_executable(code
        src/main.cpp
        src/ALUs.hpp
        src/CPU.hpp
        src/Decoder.hpp
        src/LSB.hpp
        src/Memory.hpp
        src/RF.hpp
        src/RoB.hpp
        src/RS.hpp)