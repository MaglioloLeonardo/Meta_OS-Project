#!/bin/bash
NAME="gestore"
NAMEC="$NAME.c"
NAMEO="$NAME.o"

ipcrm -M 9000
ipcrm -M 9001
ipcrm -M 9002
ipcrm -S 9003
ipcrm -S 9004
clear
echo ---------Compiler------------
gcc -DNDEBUG -w -g  -o $NAMEO $NAMEC
echo -----------Exec--------------
./$NAMEO
echo -----------------------------
echo Press ENTER for terminate the script
read
ipcrm -M 9000
ipcrm -M 9001
ipcrm -M 9002
ipcrm -S 9003
ipcrm -S 9004
rm $NAMEO
killall gestore.o
killall studente.o
clear
