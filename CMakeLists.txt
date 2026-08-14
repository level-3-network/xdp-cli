cmake_minimum_required(VERSION 4.4.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

project(xdp-cli VERSION 1.0.0 LANGUAGES C CXX)

add_executable(xdp-cli source/sdk/sdk_common.cxx
                                source/controller/xdp.cxx
                                source/controller/command_parser.cxx
                                source/main.cxx)

target_include_directories(xdp-cli PRIVATE include)

target_link_libraries(xdp-cli bpf elf z pthread)
