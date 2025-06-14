cmake_minimum_required (VERSION 3.16)

project(picorv32)

find_package(verilator)

set(CMAKE_CXX_FLAGS "-mbmi2")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(picorv32_axi)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

set(PARAMS
  -GENABLE_IRQ=1
  -GENABLE_MUL=1
  -GENABLE_DIV=1
  -GCOMPRESSED_ISA=1
  -GENABLE_TRACE=1
)

verilate(
    picorv32_axi
    SOURCES picorv32.v
    PREFIX Vpicorv32_axi
    TOP_MODULE picorv32_axi
    OUTPUT_DIRECTORY picorv32_axi
    TRACE_FST
    VERILATOR_ARGS ${PARAMS}
)

add_executable(picorv32_tb main.cpp)
target_link_libraries(picorv32_tb picorv32_axi)