.SUFFIXES:
.DEFAULT:

CC ?= cc
CXX ?= c++
CFLAGS += -Wall -Wextra -Werror -pedantic -Iatto -O0 -g
CXXFLAGS += -std=c++11 $(CFLAGS)
LIBS = -lX11 -lXfixes -lGL -lasound -lm -pthread

DEPFLAGS = -MMD -MP
COMPILE.c = $(CC) -std=gnu99 $(CFLAGS) $(DEPFLAGS) -MT $*.o -MF $*.d
COMPILE.cc = $(CXX) $(CXXFLAGS) $(DEPFLAGS) -MT $*.oo -MF $*.dd

all: run_tool

%.o: %.c
	$(COMPILE.c) -c $< -o $@

%.oo: %.cc
	$(COMPILE.cc) -c $< -o $@

TOOL_CSRC = atto/src/app_linux.c atto/src/app_x11.c tool/syntmash.c tool/Automation.c tool/seqgui.c
TOOL_CCSRC = tool/tool.cc tool/Video.cc tool/Audio.cc tool/Intro.cc tool/Timeline.cc
TOOL_OBJS = $(TOOL_CSRC:.c=.o) $(TOOL_CCSRC:.cc=.oo)
TOOL_DEPS = $(TOOL_CSRC:.c=.d) $(TOOL_CCSRC:.cc=.dd)

-include $(TOOL_DEPS)

tool/tool: $(TOOL_OBJS)
	$(CXX) $(LIBS) $^ -o $@

clean:
	rm -f $(TOOL_OBJS) $(TOOL_DEPS) tool/tool

run_tool: tool/tool
	tool/tool -m ''

.PHONY: all clean run_tool depend
