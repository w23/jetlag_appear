#pragma once
#include "atto/app.h"

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

#define MSG(...) \
	aAppDebugPrintf(__VA_ARGS__)
