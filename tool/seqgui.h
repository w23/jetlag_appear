#pragma once
#include "Automation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned char r, g, b, a;
} GuiColor;

typedef struct {
	int x, y, w, h;
} GuiViewport;

typedef struct {
	int x, y;
} GuiPoint;

void guiSetViewport(GuiViewport v);

void guiRect(int ax, int ay, int bx, int by, GuiColor c);
void guiLine(int ax, int ay, int bx, int by, GuiColor c);

typedef struct {
	int x, y;
	float s;
} GuiTransform;

void guiPaintAutomation(const Automation *a, float now_sec, GuiTransform transform);

#ifdef __cplusplus
} // extern "C"
#endif
