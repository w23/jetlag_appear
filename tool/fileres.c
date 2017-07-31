#include "common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RESOURCES 16

#if defined(__linux__)
#include <unistd.h>

typedef struct timespec FrFileTime;

static FrFileTime readFileTime(const char *filename) {
	struct stat st;
	stat(filename, &st);
	return st.st_mtim;
}

static int areFileTimesEqual(const FrFileTime *a, const FrFileTime *b) {
	return a->tv_sec == b->tv_sec && a->tv_nsec == b->tv_nsec;
}

static int readFileContents(const char *filename, VolatileResource *res) {
	const int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		MSG("Cannot open file %s", filename);
		return 0;
	}

	struct stat st;
	stat(filename, &st);

	void *bytes = calloc(1, st.st_size + 1);

	if (read(fd, bytes, st.st_size) != st.st_size) {
		MSG("Cannot read file '%s'", filename);
		free(bytes);
		return 0;
	}
	close(fd);

	if (res->bytes)
		free((void*)res->bytes);

	res->size = st.st_size;
	res->bytes = bytes;
	return 1;
}

#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMSG
#include <windows.h>

typedef FILETIME FrFileTime;

static FrFileTime readFileTime(const char *filename) {
	FrFileTime ft = { 0, 0 };
	const HANDLE fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE) {
		MSG("Cannot check file '%s' modification time: %x", filename, GetLastError());
		return ft;
	}

	if (!GetFileTime(fh, NULL, NULL, &ft)) {
		MSG("Cannot get file '%s' modification time", filename);
		ft.dwHighDateTime = ft.dwLowDateTime = 0;
		return ft;
	}

	CloseHandle(fh);
	return ft;
}

static int areFileTimesEqual(const FrFileTime *a, const FrFileTime *b) {
	return a->dwHighDateTime == b->dwHighDateTime && a->dwLowDateTime == b->dwLowDateTime;
}

static int readFileContents(const char *filename, VolatileResource *res) {
	const HANDLE fh = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fh == INVALID_HANDLE_VALUE) {
		MSG("Cannot open file '%s'", filename);
		return 0;
	}

	LARGE_INTEGER splurge_integer;
	if (!GetFileSizeEx(fh, &splurge_integer) || splurge_integer.QuadPart > 1024*1024) {
		MSG("Cannot get file '%s' size or size is too large", filename);
		return 0;
	}

	int size = (int)splurge_integer.QuadPart;
	void *bytes = calloc(1, size + 1);
	DWORD bytes_read = 0;
	if (!ReadFile(fh, bytes, size, &bytes_read, NULL) || bytes_read != size) {
		MSG("Cannot read %d bytes from file '%s'", size, filename);
		free(bytes);
		return 0;
	}
	CloseHandle(fh);

	if (res->bytes)
		free((void*)res->bytes);

	res->size = size;
	res->bytes = bytes;
	return 1;
}


#else
#error not ported
#endif

typedef struct {
	VolatileResource public;
	FrFileTime prev_time;
	char filename[256];
} ResourceEntry;

static struct {
	ResourceEntry resources[MAX_RESOURCES];
} g;

static void resourcePoll(ResourceEntry *e) {
	if (!e->filename[0])
		return;

	e->public.updated = 0;

	const FrFileTime new_time = readFileTime(e->filename);
	if (areFileTimesEqual(&new_time, &(e->prev_time)))
		return;

	if (!readFileContents(e->filename, &e->public))
		return;

	e->prev_time = new_time;
	e->public.updated = 1;

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
	for (int i = 0; i < MAX_RESOURCES; ++i)
		resourcePoll(g.resources + i);
}
