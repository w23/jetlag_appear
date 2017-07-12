#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void audioInit(const char *synth_src, int samplerate);
void audioSynthesize(float *samples, int num_samples);

#ifdef __cplusplus
} /* extern "C" */
#endif
