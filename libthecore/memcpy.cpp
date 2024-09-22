#include "stdafx.h" // For C++

#if !defined(_WIN64)
void *(*thecore_memcpy) (void *to, const void *from, size_t len) = memcpy;
#endif