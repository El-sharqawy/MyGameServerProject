#pragma once

#include <cstdint>

typedef struct SBuffer
{
	/* Pointer to the next buffer in a linked list (used in pooling) */
	SBuffer* next;

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

/* Create a new buffer of the specified size. */
extern LPBUFFER buffer_new(int32_t iSize);

/* Free or return buffer to the pool. */
extern void buffer_delete(LPBUFFER buffer);

/* Reset buffer to its initial state. */
extern void buffer_reset(LPBUFFER buffer);

/* Reallocates the given buffer to accommodate additional memory */
void buffer_realloc(LPBUFFER& buffer, int32_t iLength);

/* Writes data to the buffer and reallocates if necessary */
extern void buffer_write(LPBUFFER& buffer, const void* src, int32_t iLength);

/* Returns the current write position in the buffer */
extern void* buffer_write_peek(LPBUFFER buffer);

/* Advances the write position in the buffer after data is written */
extern void buffer_write_proceed(LPBUFFER buffer, int32_t iLength);

/* Reads data from the buffer */
extern void buffer_read(LPBUFFER buffer, void *buf, int32_t iBytes);

/* Advances the read position after reading data */
extern void buffer_read_proceed(LPBUFFER buffer, int32_t iLength);

/* Returns a pointer to the current read position */
extern const void* buffer_read_peek(LPBUFFER buffer);

/* Determines how much space is left in the buffer */
extern int32_t buffer_has_space(LPBUFFER buffer);

/* Get the current size (length) of the buffer. */
extern uint32_t buffer_size(LPBUFFER buffer);

/* Adjusts the buffer size to accommodate additional data */
extern void buffer_adjust_size(LPBUFFER& buffer, int32_t iAdded_Size);

/* Reads a BYTE value from the buffer */
extern uint8_t buffer_get_byte(LPBUFFER buffer);

/* Reads a WORD value from the buffer */
extern uint16_t buffer_get_word(LPBUFFER buffer);

/* Reads a DWORD value from the buffer */
extern uint32_t buffer_get_dword(LPBUFFER buffer);