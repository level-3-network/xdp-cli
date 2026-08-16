## XDP-CLI

 - Version: 1.0.0-alpha
 - License: GPL-2.0-only
 - Author: level-3-network

# Compiling from source

The packages you need to install are the following:
 - C/C++ Compilers (mainly GCC and G++)
 - Clang
 - LLVM
 - CMake (minimum 4.4.1 required)
 - Ninja
 - libbpf
 - libelf
 - bpftool

Run the `compile.sh` script and wait for it to finish.

# Executing

Run the compiled program as sudo, below are a list of application arguments (all required):
 - --in-interface - IN interface to attach XDP program to.
 - --out-interface - OUT interface to attach XDP program to.

Copy and paste the `command_list.txt` file into the folder where the program is located.

# Features

 - Blacklist (IPv4/IPv6 support with prefixes)
 - Traffic shaper (IPv4/IPv6 support with prefixes)

# Notes

This is program is still in development (alpha stage), it is not fully 100% tested (especially on IPv6 networks), if you find any bugs please contact me, or submit an issue.
