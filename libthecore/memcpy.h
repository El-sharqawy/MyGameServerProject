#pragma once

#include "stdafx.h"

#if defined(_WIN64)
    #define thecore_memcpy memcpy
#else
    extern void *(*thecore_memcpy) (void *to, const void *from, size_t len);
#endif