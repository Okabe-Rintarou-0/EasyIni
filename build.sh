#!/bin/bash

clang++ \
    -std=c++26 \
    -freflection-latest \
    -fexpansion-statements \
    ini_parser.cpp \
    -o ini_parser \
    -L/opt/p2996/clang/lib/x86_64-unknown-linux-gnu -Wl,-rpath,/opt/p2996/clang/lib/x86_64-unknown-linux-gnu
