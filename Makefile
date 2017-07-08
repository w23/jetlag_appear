.SUFFIXES:
.DEFAULT:

CC ?= cc
CXX ?= c++
CFLAGS += -Wall -Wextra -Werror -pedantic -Iatto -O0 -g
CXXFLAGS += -std=c++11 $(CFLAGS)
LIBS = -lX11 -lXfixes -lGL -lasound -lm -pthread
OBJDIR ?= .obj
MIDIDEV ?=

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
	tool/Automation.c \
	tool/seqgui.c \
	tool/video.c \
	tool/fileres.c \
	tool/tool.cc \
	tool/audio.c \
	tool/Intro.cc \
	tool/timeline.c
TOOL_OBJS = $(TOOL_SRCS:%=$(OBJDIR)/%.o)
TOOL_DEPS = $(TOOL_OBJS:%=%.d)

-include $(TOOL_DEPS)

$(TOOL_EXE): $(TOOL_OBJS)
	$(CXX) $(LIBS) $^ -o $@

clean:
	rm -f $(TOOL_OBJS) $(TOOL_DEPS) $(TOOL_EXE)

run_tool: $(TOOL_EXE)
	$(TOOL_EXE) -m $(MIDIDEV)

.PHONY: all clean run_tool
