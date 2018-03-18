#include "common.h"

#include <stdlib.h>

void mutableStringInit(MutableString *ms) {
	ms->str = NULL;
	ms->length = 0;
	ms->capacity = 0;
}

void mutableStringAppendSV(MutableString *ms, StringView sv) {
	const int new_length = ms->length + sv.length;
	if (new_length >= ms->capacity) {
		int new_capacity = ms->capacity + 1;
		while (new_capacity < new_length + 1)
			new_capacity = ((new_capacity * 3 / 2 + 15) / 16) * 16;

		char *new_str = malloc(new_capacity);
		if (ms->str) {
			memcpy(new_str, ms->str, ms->length);
			free(ms->str);
		}

		ms->capacity = new_capacity;
		ms->str = new_str;
	}

	memcpy(ms->str + ms->length, sv.str, sv.length);
	ms->str[new_length] = '\0';
	ms->length = new_length;
}

void mutableStringAppend(MutableString *ms, const char *str, int length) {
	const StringView sv = {str, length};
	mutableStringAppendSV(ms, sv);
}

void mutableStringAppendZ(MutableString *ms, const char *str) {
	mutableStringAppend(ms, str, (int)strlen(str));
}

void mutableStringDestroy(MutableString *ms) {
	if (ms->str)
		free(ms->str);
	mutableStringInit(ms);
}

char *mutableStringRelease(MutableString *ms) {
	char *retval = ms->str;
	ms->str = NULL;
	ms->capacity = 0;
	ms->length = 0;
	return retval;
}
