#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void videoInit(int width, int height);
void videoPaint(float *signals, int num_signals, int force_redraw);

#ifdef __cplusplus
} /* extern "C" */
#endif
