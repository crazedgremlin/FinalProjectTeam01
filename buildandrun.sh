#!/bin/bash
#
# Compiles project for OSX and passes arguments specified to this script on.
#

killall checkers.exe && \
gcc -framework GLUT -framework OpenGL -framework Cocoa -o checkers.exe *.c && \
./checkers.exe $*
