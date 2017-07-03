#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct VolatileResource_t {
	int updated;
	const void *bytes;
	int size;
} VolatileResource;

VolatileResource *resourceOpenFile(const char *filename);
void resourceClose(VolatileResource *handle);
void resourcesInit();
void resourcesUpdate();

#ifdef __cplusplus
} /* extern "C" */
#endif
