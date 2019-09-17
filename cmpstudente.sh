#!/bin/bash
NAME="studente"

#-----Program--------
clear
echo ---------Compiler------------
NAMEC="$NAME.c"
NAMEO="$NAME.o"
gcc -DNDEBUG -w -g  -o $NAMEO $NAMEC

