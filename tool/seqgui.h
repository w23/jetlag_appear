#pragma once
#include "Automation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned char r, g, b, a;
} GuiColor;

typedef struct {
	int x, y;
} GuiPoint;

typedef struct {
	int x, y, w, h;
} GuiRect;

void guiSetViewport(GuiRect v);

void guiRect(int ax, int ay, int bx, int by, GuiColor c);
void guiLine(int ax, int ay, int bx, int by, GuiColor c);

typedef struct {
	int x, y;
	float s;
} GuiTransform;

void guiPaintAutomation(const Automation *a, float now_sec, GuiTransform transform);

typedef struct {
	int x, y;
	int button_mask;
	int xor_button_mask;
} GuiEventPointer;

void guiEventPointer(Automation *a, GuiEventPointer ptr);

#ifdef __cplusplus
} // extern "C"
#endif
