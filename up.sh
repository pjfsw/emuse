#!/bin/sh

echo U > /dev/tty68k && sleep 0.5 &&  cat build/$1 > /dev/tty68k
