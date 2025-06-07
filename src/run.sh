#!/bin/bash
clear
cd ..
ninja
cd src
./tcc prog2.txt | llc -filetype=obj -relocation-model=pic -o=prog1.o
clang -o prog1 prog1.o rtcalc.c
./prog1
