.SUFFIXES:
.DEFAULT:

CC ?= cc
CXX ?= c++
CFLAGS += -Wall -Wextra -Werror -pedantic -Iatto -O0 -g
CXXFLAGS += -std=c++11 $(CFLAGS)
LIBS = -lX11 -lXfixes -lGL -lasound -lm -pthread
OBJDIR ?= .obj
MIDIDEV ?= ''
WIDTH ?= 1280
HEIGHT ?= 720
SHMIN=mono shader_minifier.exe
INTRO=nikak

DEPFLAGS = -MMD -MP
COMPILE.c = $(CC) -std=gnu99 $(CFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d
COMPILE.cc = $(CXX) $(CXXFLAGS) $(DEPFLAGS) -MT $@ -MF $@.d

all: run_tool

$(OBJDIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(COMPILE.c) -c $< -o $@

$(OBJDIR)/%.cc.o: %.cc
	@mkdir -p $(dir $@)
	$(COMPILE.cc) -c $< -o $@

TOOL_EXE = $(OBJDIR)/tool/tool
TOOL_SRCS = \
	atto/src/app_linux.c \
	atto/src/app_x11.c \
	tool/syntmash.c \
	tool/syntasm.c \
	tool/parser.c \
	tool/Automation.c \
	tool/video.c \
	tool/fileres.c \
	tool/tool.c \
	tool/audio.c \
	tool/timeline.c
TOOL_OBJS = $(TOOL_SRCS:%=$(OBJDIR)/%.o)
TOOL_DEPS = $(TOOL_OBJS:%=%.d)

-include $(TOOL_DEPS)

$(TOOL_EXE): $(TOOL_OBJS)
	$(CXX) $(LIBS) $^ -o $@

clean:
	rm -f $(TOOL_OBJS) $(TOOL_DEPS) $(TOOL_EXE)
	rm -f $(INTRO).sh $(INTRO).gz $(INTRO).elf $(INTRO) $(INTRO).dbg

run_tool: $(TOOL_EXE)
	$(TOOL_EXE) -w $(WIDTH) -h $(HEIGHT) -m $(MIDIDEV)

debug_tool: $(TOOL_EXE)
	gdb --args $(TOOL_EXE) -w $(WIDTH) -h $(HEIGHT) -m $(MIDIDEV)

$(INTRO).sh: linux_header $(INTRO).gz
	cat linux_header $(INTRO).gz > $@
	chmod +x $@

$(INTRO).gz: $(INTRO).elf
	cat $< | 7z a dummy -tGZip -mx=9 -si -so > $@

%.h: %.glsl
	$(SHMIN) -o $@ --no-renaming-list Z,T,P,X,k,F,t,E,PI,main --preserve-externals $<

4klang.o: 4klang.asm 4klang.inc
	nasm -f elf32 4klang.asm -o 4klang.o

#.h.seq: timepack
#	timepack $< $@

#timepack: timepack.c
#	$(CC) -std=c99 -Wall -Werror -Wextra -pedantic -lm timepack.c -o timepack

# '-nostartfiles -DCOMPACT' result in a libSDL crash on my machine (making older 1k/4k also crash) :(
$(INTRO).elf: 4klang.o intro_c.c
	$(CC) -m32 -Os -Wall -Wno-unknown-pragmas \
		-DFULLSCREEN `pkg-config --cflags --libs sdl` -lGL \
		$^ -o $@
	sstrip $@

$(INTRO).dbg: intro_c.c 4klang.o
	$(CC) -m32 -O0 -g -Wall -Wno-unknown-pragmas \
		`pkg-config --cflags --libs sdl` -lGL \
		$^ -o $@

.PHONY: all clean run_tool debug_tool
