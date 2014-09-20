#!/bin/sh
./convert.py
gcc -pg uvm-prof.c -o uvm
./uvm sandmark.umz
echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
gprof a.out -p
