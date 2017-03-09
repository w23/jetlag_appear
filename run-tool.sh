clang++ -g -O0 --std=c++11 -Iatto atto/src/app_x11.c atto/src/app_linux.c tool.cc -o tool -lGL -lm -lX11 -lXfixes -o tool && ./tool trace.glsl post.glsl
