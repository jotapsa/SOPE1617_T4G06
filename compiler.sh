#!/bin/bash

clang -c finder.c
clang sfind.c finder.o -o sfind
