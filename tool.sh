#!/bin/sh
CC=clang
CXX=clang++
CFLAGS="-Wall -Wextra -pedantic -Iatto -O0 -g"
CXXFLAGS="-std=c++11 $CFLAGS"
LIBS="-lX11 -lXfixes -lGL -lasound -lm -pthread"

mkdir -p ./.build

OBJS=""

for src in ./atto/src/app_linux.c ./atto/src/app_x11.c tool/syntmash.c
do
	OBJ="./.build/$(basename "$src" .c).o"
	echo $CC -c -std=gnu99 $CFLAGS $src -o $OBJ
	$CC -c -std=gnu99 $CFLAGS $src -o $OBJ
	OBJS="$OBJS $OBJ"
done

for src in tool/tool.cc tool/Video.cc tool/Audio.cc
do
	OBJ="./.build/$(basename "$src" .cc).oo"
	echo $CXX -c $CXXFLAGS $src -o $OBJ
	$CXX -c $CXXFLAGS $src -o $OBJ
	OBJS="$OBJS $OBJ"
done

$CXX $LIBS $OBJS -o ./tool/tool
