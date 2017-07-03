#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "Automation.h"

void videoInit(int width, int height);
void videoPaint(const Frame *frame, int force_redraw);

#ifdef __cplusplus
} /* extern "C" */
#endif
