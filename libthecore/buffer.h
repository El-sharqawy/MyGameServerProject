#pragma once

#include <cstdint>

typedef struct SBuffer
{
	/* Pointer to the next buffer in a linked list (used in pooling) */
	TBuffer* next;

	/* Pointer to the current position where new data will be written */
	char* write_point;

	/* The index (offset) from the start of the buffer where the write point is located */
	int write_point_pos;

	/* Pointer to the current position from where data will be read */
	const char* read_point;

	/* The amount of valid data in the buffer (in bytes) */
	int length;

	/* Pointer to the actual memory allocated for the buffer */
	char* mem_data;

	/* The total size of the allocated memory for this buffer */
	int mem_size;

	/* A field used to store various flags or status indicators */
	long flag;
} TBuffer;

/* a variable to Buffer struct */
typedef TBuffer BUFFER;

/* a pointer to Buffer struct */
typedef TBuffer* LPBUFFER;

/* Safely allocate memory and handle errors. */
bool safe_create(char** pData, int32_t iNum);

extern LPBUFFER buffer_new(int32_t iSize);
extern void buffer_delete(LPBUFFER buffer);
extern void buffer_reset(LPBUFFER buffer);