#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "Automation.h"

void video_init(int width, int height);
void video_paint(const Frame *frame, int force_redraw);

#ifdef __cplusplus
} /* extern "C" */
#endif
