cmake_minimum_required(VERSION 3.10)
project(LLVMCalculator)

find_package(LLVM REQUIRED CONFIG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(calculator main.cpp)

target_link_libraries(calculator PRIVATE LLVM::LLVMCore LLVM::LLVMSupport)
