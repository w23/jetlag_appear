#include "common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	VolatileResource public;
	struct timespec mtime;
	char filename[256];
} ResourceEntry;

#define MAX_RESOURCES 16

static struct {
	ResourceEntry resources[MAX_RESOURCES];
} g;

static void resourcePoll(ResourceEntry *e) {
	if (!e->filename[0])
		return;

	e->public.updated = 0;

	struct stat st;
	stat(e->filename, &st);
	if (st.st_mtim.tv_sec == e->mtime.tv_sec &&
			st.st_mtim.tv_nsec == e->mtime.tv_nsec)
		return;

	e->mtime = st.st_mtim;

	int fd = open(e->filename, O_RDONLY);
	if (fd < 0) {
		MSG("Cannot open file %s", e->filename);
		return;
	}

	void *bytes = calloc(1, st.st_size + 1);

	if (read(fd, bytes, st.st_size) != st.st_size) {
		MSG("Cannot read file");
		free(bytes);
		return;
	}
	close(fd);

	if (e->public.bytes)
		free((void*)e->public.bytes);

	e->public.updated = 1;
	e->public.size = st.st_size;
	e->public.bytes = bytes;

	MSG("Reread file %s", e->filename);
}

VolatileResource *resourceOpenFile(const char *filename) {
	for (int i = 0; i < MAX_RESOURCES; ++i) {
		ResourceEntry *e = g.resources + i;
		if (e->filename[0] == '\0') {
			memset(e, 0, sizeof(*e));
			strncpy(e->filename, filename, sizeof(e->filename));
			return &e->public;
		}
	}
	ASSERT("All resource slots busy");
	return 0;
}

void resourceClose(VolatileResource *handle) {
	ResourceEntry *e = (ResourceEntry*)handle;
	if (e->public.bytes)
		free((void*)e->public.bytes);
	e->filename[0] = '\0';
}

void resourcesInit() {
	memset(&g, 0, sizeof(g));
}

void resourcesUpdate() {
	for (int i = 0; i < MAX_RESOURCES; ++i) {
		resourcePoll(g.resources + i);
	}
}
