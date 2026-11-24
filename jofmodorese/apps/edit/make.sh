#!/bin/sh

cmake -B build -S . && cmake --build build --target clean && cmake --build build
