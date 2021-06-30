cmake_minimum_required(VERSION 3.19)
project(riscv_simulator)

set(CMAKE_CXX_STANDARD 14)

add_executable(riscv_simulator
        src/base_run.cpp
        src/memory.h
        src/register.h src/core.h)