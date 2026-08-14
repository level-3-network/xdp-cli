#!/bin/bash

clang -O2 -g -target bpf -c ./source/data_plane.c -I ./include -o ./data_plane.bpf.o

rm -rf ./include/data_plane.skel.h

sudo bpftool gen skeleton ./data_plane.bpf.o > ./include/data_plane.skel.h

cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -S ./ -B ./cmake -G Ninja

cmake --build ./cmake
