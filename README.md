## XDP-CLI

Version: 1.0.0-alpha
License: GPL-2.0-only
Author: level-3-network

# Compiling from source

The packages you need to install are the following:
 - C/C++ Compilers
 - Clang
 - LLVM
 - CMake (minimum 4.4.1 required)
 - Ninja
 - libbpf
 - libelf
 - bpftool

Run the `compile.sh` script and wait for it to finish

# Executing

Run the compiled program as sudo, below are a list of application arguments (some are required):
 - --in-interface - IN interface to attach XDP program to
 - --out-interface - OUT interface to attach XDP program to

Copy and paste the `command_list.txt` file into the folder where the program is located

# Notes

This is program is still in development (alpha stage), it is not fully tested, if you find any bugs please contact me
