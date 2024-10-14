








PROGRAM=$1
CC=g++
EXT=cpp
fname=$(basename $PROGRAM .$EXT)


echo "file result: " $fname


$CC -o $fname $PROGRAM 



