cmake_minimum_required(VERSION 4.4.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

project(xdp-cli VERSION 1.0.0 LANGUAGES C CXX)

add_executable(xdp-cli ./source/xdp_cli/controller_xdp.cxx
                       ./source/xdp_cli/controller_command_parser.cxx
                       ./source/xdp_cli/main.cxx)

set_target_properties(xdp-cli PROPERTIES ENABLE_EXPORTS TRUE)

target_include_directories(xdp-cli PRIVATE ./include/)
target_link_libraries(xdp-cli bpf elf pthread)

project(xdp-cli-commands VERSION 1.0.0 LANGUAGES C CXX)

add_library(xdp-cli-commands SHARED ./source/xdp_cli_commands/main.cxx)

set_target_properties(xdp-cli-commands PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(xdp-cli-commands PRIVATE ./include/)
