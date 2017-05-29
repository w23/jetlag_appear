clang++ -std=c++11 -Iatto -O0 -g -Wall -Wextra -pedantic tool.cc ./atto/src/app_linux.c ./atto/src/app_x11.c -o tool -lX11 -lXfixes -lGL
