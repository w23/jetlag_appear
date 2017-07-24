#pragma once

#include "Automation.h"

#ifdef __cplusplus
extern "C" {
#endif

void audioInit(const char *synth_src, int samplerate);
void audioCheckUpdate();
void audioSynthesize(float *samples, int num_samples, const Automation *a);

#ifdef __cplusplus
} /* extern "C" */
#endif
