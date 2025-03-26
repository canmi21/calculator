cmake_minimum_required(VERSION 3.10)
project(LLVMCalculator)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

llvm_map_components_to_libnames(llvm_libs core support)

add_executable(calculator main.cpp parser.cpp)
target_link_libraries(calculator PRIVATE LLVM-19)