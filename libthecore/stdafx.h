#pragma once

#define __LIBTHECORE__

#include "utils.h"
#include "log.h"
#include "memcpy.h"
#include "typedef.h"
#include "buffer.h"
#include "buffer_manager.h"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cctype>  // For isprint
#include <fstream>
#include <memory>
#include <cassert>
#include <stddef.h>
#include <cmath>
#if defined(_WIN64)
#define strdup _strdup
#include <time.h>
#include <windows.h>
#include <sys/stat.h>
#else
#include <sys/time.h>
//#define snprintf _snprintf
#endif

#define NOMINMAX

/* checks if the given character is a utf8 char or not */
#define isutf8(ch)       (((ch) & 0x80) == 0 || ((ch) & 0xE0) == 0xC0 || ((ch) & 0xF0) == 0xE0 || ((ch) & 0xF8) == 0xF0)

#if defined(_WIN64)
#define strcasecmp(s1, s2) stricmp(s1, s2)
#endif

#if !defined(S_ISDIR)
#define S_ISDIR(m)	(m & _S_IFDIR)
#endif


#define PATH_MAX _MAX_PATH