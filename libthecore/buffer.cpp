#include "stdafx.h"

#define BUFFER_REALLOC_SIZE 10240
/***
 * normalized_buffer_pool - The normalized_buffer_pool is an array of pointers to linked lists of buffers,
 * categorized by size. Each index in the array represents buffers of sizes that 
 * are powers of two (e.g., index 0 for size 1 byte, index 1 for size 2 bytes,
 * index 2 for size 4 bytes, ..., index 31 for size 2^31 bytes).
 * This allows efficient management of buffers by reusing memory from the pool
 * instead of constantly allocating and deallocating buffers, which can be costly.
 * When a buffer of a certain size is needed, the corresponding pool (index) is
 * checked first. If a buffer is available, it is reused; otherwise, a new buffer
 * is allocated. This approach helps reduce memory fragmentation and allocation overhead.
 */
static LPBUFFER normalized_buffer_pool[32] = { nullptr, };

/**
 * buffer_get_pool_index - Get the index of the buffer pool size for allocation.
 * @iSize: The size of the buffer needed.
 *
 * This function calculates the index of the buffer pool that can accommodate
 * the specified size. The buffer sizes are powers of 2, and the function
 * returns the index of the first buffer size that is greater than or equal to
 * the requested size. If the size is too large to be pooled, returns -1.
 *
 * Return: Index of the buffer pool, or -1 if the size is too large.
 */
static int32_t buffer_get_pool_index(int32_t iSize)
{
	for (int32_t i = 0; i < 32; i++)
	{
		/* i is powers of 2, ex. when i = 3, result is 8 (2*2*2) */
		if ((1 << i) >= iSize)
		{
			return (i);
		}
	}
	return (-1); /* Too big, not pooled. */
}


/**
 * buffer_get_exact_pool_index - Get the exact buffer pool index.
 * @iSize: The exact size of the buffer needed.
 *
 * This function calculates the index of the buffer pool that exactly matches
 * the requested size. The sizes in the pool are powers of 2, and the function
 * returns the index if it finds an exact match. If the size is too large or
 * doesn't exactly match, it returns -1.
 *
 * Return: Exact index of the buffer pool, or -1 if no exact match is found.
 */
static int32_t buffer_get_exact_pool_index(int32_t iSize)
{
	for (int32_t i = 0; i < 32; i++)
	{
		if ((1 << i) == iSize)
		{
			return (i);
		}
	}
	return (-1); /* Too big, not pooled. */
}

/**
 * buffer_pool_free - Free all buffer pools.
 *
 * This function frees all buffers in all pool levels. It iterates over each
 * buffer pool, releasing the memory of each buffer in the pool and setting the
 * pool to NULL.
 * Return: Nothing (void.)
 */
static void buffer_pool_free()
{
	for (int32_t i = 31; i >= 0; i--)
	{
		if (normalized_buffer_pool[i] != nullptr)
		{
			LPBUFFER nextBuffer = nullptr;
			for (LPBUFFER buffer = normalized_buffer_pool[i]; buffer != nullptr; buffer = nextBuffer)
			{
				nextBuffer = buffer->next;
				free(buffer->mem_data);
				free(buffer);
			}
			normalized_buffer_pool[i] = nullptr;
		}
	}
}

/**
 * buffer_larger_pool_free - Free one larger buffer pool.
 * @iIndex: The index of the current pool size.
 *
 * This function frees one buffer from the pool larger than the specified index
 * @iIndex. It searches the buffer pools from index 31 down to @iIndex, frees a buffer if
 * found, and returns true. If no larger buffer is found, it returns false.
 *
 * Return: True if a buffer was freed, false otherwise.
 */
static bool buffer_larger_pool_free(int32_t iIndex)
{
	for (int32_t i = 31; i > iIndex; i--)
	{
		if (normalized_buffer_pool[i])
		{
			LPBUFFER buffer = normalized_buffer_pool[i];
			LPBUFFER next = buffer->next;
			free(buffer->mem_data);
			free(buffer);
			normalized_buffer_pool[i]->next = next;
			return (true);
		}
	}

	return(false);
}

/***
 * safe_create - Safely allocate memory and handle errors.
 * @pData: Double pointer to the data to be allocated.
 * @iNum: The number of elements to allocate.
 *
 * This function allocates memory for @pData using calloc and handles memory
 * allocation failure by logging an error message and returning false. If the
 * allocation is successful, it returns true.
 *
 * Return: True if memory allocation is successful, otherwise false.
 */
bool safe_create(char** pData, int32_t iNum)
{
	(*pData) = (char*)calloc(iNum, sizeof(char));
	if (!(*pData))
	{
		sys_err("calloc failed, Error[%d] : %s", errno, strerror(errno));
		return (false);
	}
	return(true);
}

/***
 * buffer_new - Create a new buffer of the specified size.
 * @iSize: The size of the buffer to be created.
 *
 * This function creates a new buffer from the pool if the requested size can
 * be pooled. If a suitable buffer is not available, it tries to allocate memory.
 * If allocation fails, it frees buffers from the pool and retries.
 *
 * Return: A pointer to the newly created buffer, or nullptr if the size is invalid.
 */

LPBUFFER buffer_new(int32_t iSize)
{
	if (iSize < 0)
	{
		return (nullptr);
	}

	LPBUFFER buffer = nullptr;
	/* etermine the index of the buffer pool that can accommodate a buffer of the requested size,
	 * The function returns the index based on the size as a power of two (e.g., sizes of 1, 2, 4, 8, etc.)
	 * if none is available with request size then create new one.
	 */
	int32_t iPoolIndex = buffer_get_pool_index(iSize);

	if (iPoolIndex >= 0)
	{
		/* gets a pointer to the appropriate buffer pool from the normalized_buffer_pool array using the iPoolIndex
		 * It points to the linked list of buffers that can be reused, moves the pointer forward by iPoolIndex elements, not bytes
		 */
		BUFFER** buffer_pool = normalized_buffer_pool + iPoolIndex;

		/* updates iSize to the actual size of the buffer that corresponds to the pool index,
		 * The expression 1 << iPoolIndex calculates the size as a power of two (e.g., if iPoolIndex is 3, then iSize becomes 8).
		 */
		iSize = 1 << iPoolIndex;

		/* check if there is an existing buffer in the pool */
		if (*buffer_pool)
		{
			/* retrieves the first buffer from the pool and assigns it to buffer */
			buffer = *buffer_pool;

			/* updates the head of the pool to point to the next buffer in the linked list (buffer->next),
			 * effectively removing the retrieved buffer from the pool for use.
			 */
			*buffer_pool = buffer->next;
		}
	}
	
	if (buffer == nullptr)
	{
		/* create new buffer */
		CREATE(buffer, BUFFER, 1);
		buffer->mem_size = iSize;
		/* calloc failures frequently occur in buffer_new(especially on low - end machines in Turkiye),
		 * so if calloc fails, the buffer pool is cleared and the operation is retried.
		 */
		if (!safe_create(&buffer->mem_data, iSize))
		{
			/* Releases one buffer from the pool that is larger than the required buffer. */
			if (buffer_larger_pool_free(iPoolIndex))
			{
				/* If this fails, as a last resort, all pools are cleared. */
				buffer_pool_free();
			}
			CREATE(buffer->mem_data, char, iSize);
			sys_err("buffer pool free success");
		}
	}

	assert(buffer != nullptr);
	assert(buffer->mem_size == iSize);
	assert(buffer->mem_data != nullptr);

	/* Reset buffer data, ensuring everything is clean and clear. */
	buffer_reset(buffer);

	return(buffer);
}

/***
 * buffer_delete - Free or return buffer to the pool.
 * @buffer: Pointer to the buffer to delete.
 *
 * This function resets the buffer, then returns it to the pool if it belongs
 * to a pooled size. If it does not belong to a pool, the buffer's memory is
 * freed.
 * Return: Nothing (void.)
 */
void buffer_delete(LPBUFFER buffer)
{
	if (!buffer)
	{
		return;
	}

	/* reset buffer data, make it's state clear */
	buffer_reset(buffer);

	/* store the buffer size, to use getting exact correct buffer pool index */
	int32_t iSize = buffer->mem_size;

	/* get the exact pool index for the size of the buffer */
	int32_t iPoolIndex = buffer_get_exact_pool_index(iSize);

	/* checks if a valid pool index was found, it means there’s a pool that can hold buffers of this size */
	if (iPoolIndex >= 0)
	{
		/* gets a pointer to the appropriate buffer pool from the normalized_buffer_pool array using the iPoolIndex
		 * It points to the linked list of buffers that can be reused, moves the pointer forward by iPoolIndex elements, not bytes
		 */
		BUFFER** buffer_pool = normalized_buffer_pool + iPoolIndex;

		/* links the current buffer (buffer) to the start of the existing buffer pool by setting its next pointer to point to the current first buffer in the pool */
		buffer->next = *buffer_pool;

		/* updates the buffer pool to point to the current buffer, effectively adding it to the front of the pool. This means the current buffer is now available for reuse */
		*buffer_pool = buffer;
	}
	else /* no valid pool index was found */
	{
		/* free the memory allocated for mem_data */
		free(buffer->mem_data);
		/* free the buffer itself, ensuring that all allocated resources are properly released to avoid memory leaks */
		free(buffer);
	}
}

/***
 * buffer_reset - Reset buffer to its initial state.
 * @buffer: The buffer to reset.
 *
 * This function resets the buffer's read and write pointers and other internal
 * fields, effectively clearing it for reuse.
 * Return: Nothing (void.)
 */
void buffer_reset(LPBUFFER buffer)
{
	buffer->read_point = buffer->mem_data;
	buffer->write_point = buffer->mem_data;
	buffer->write_point_pos = 0;
	buffer->length = 0;
	buffer->next = nullptr;
	buffer->flag = 0;
}

/**
 * buffer_realloc - Reallocates the given buffer to accommodate additional memory
 * @buffer: The buffer to be reallocated (passed by reference)
 * @length: The new desired length for the buffer
 *
 * This function checks if the buffer's current size is less than the requested
 * size. If so, it allocates a new buffer with the requested size, copies the
 * data from the old buffer into the new one, and updates the read and write
 * pointers to maintain their relative positions. It then frees the old buffer
 * and assigns the new buffer in its place. If the requested length is less than
 * or equal to the current size, no reallocation is performed.
 *
 * Return: Nothing (void.)
 */
void buffer_realloc(LPBUFFER& buffer, int32_t iLength)
{
	int32_t i;
	LPBUFFER tempBuf;

	assert(iLength >= 0 && "buffer_realloc: buffer length is less than zero!");

	/* buffer size already bigger or equal to the needed, don't reallocate */
	if (buffer->mem_size >= iLength)
	{
		return;
	}

	/* i is the difference between the newly allocated size and the previous size,
	 * which is actually the size of the newly created memory (the amount of memory needed).
	 */
	i = iLength - buffer->mem_size;

	/* if there's no extra memory needed, don't reallocate */
	if (i <= 0)
	{
		return;
	}

	/* allocate new temporary buffer with the bigger size */
	tempBuf = buffer_new(iLength);
	sys_log(0, "reallocating buffer to [%d], current [%d]", tempBuf->mem_size, buffer->mem_size);

	/* copy the existing data to the new created buffer */
	thecore_memcpy(tempBuf->mem_data, buffer->mem_data, buffer->mem_size);

	/* The current position of the read_point (the point in the buffer where the next read will occur)
	 * is saved by calculating its offset from the start of the buffer's data (mem_data).
	 * This allows the function to maintain the correct read position in the new buffer after reallocation.
	 */
	ptrdiff_t read_point_pos = tempBuf->read_point - buffer->mem_data;

	/* Set the next write point position in the new buffer's data */
	tempBuf->write_point = tempBuf->mem_data + buffer->write_point_pos;

	/* Transfer the write point position to the new buffer */
	tempBuf->write_point_pos = buffer->write_point_pos;

	/* Update Read point and make it point to the correct position */
	tempBuf->read_point = read_point_pos + buffer->mem_data;

	/* Copy Buffer flag */
	tempBuf->flag = buffer->flag;

	/* set next buffer to null since this is a new buffer */
	tempBuf->next = nullptr;

	/* set the data length that is copied from previous buffer to the new one */
	tempBuf->length = buffer->length;

	/* delete the old buffer since we don't need it anymore */
	buffer_delete(buffer);

	/* set the buffer to the newly allocated one */
	buffer = tempBuf;
}

/***
 * buffer_write - Writes data to the buffer and reallocates if necessary
 * @buffer: The buffer to write data into (passed by reference)
 * @src: Pointer to the source data to be written
 * @iLength: The length of the data to be written
 *
 * This function checks if the buffer has enough space to accommodate the
 * new data. If the current buffer's write position plus the length of the
 * data exceeds the buffer's allocated size, it reallocates the buffer to
 * make room for the additional data. After ensuring there is enough space,
 * the function copies the data from the source to the buffer's write point
 * and updates the buffer's internal write position.
 *
 * Return: None
 */
void buffer_write(LPBUFFER& buffer, const void* src, int32_t iLength)
{
	/* if the buffer actual write position and the given data length is equal or bigger than the allocated memory size */
	if (buffer->write_point_pos + iLength >= buffer->mem_size)
	{
		/* then reallocate the buffer to have a space for the new data to be written */
		sys_log(0, "buffer_write: realloc buffer : write_point_pos [%d] + iLength [%d] >= mem_size [%d]", buffer->write_point_pos, iLength, buffer->mem_size);
		buffer_realloc(buffer, buffer->mem_size + iLength + std::min<int32_t>(BUFFER_REALLOC_SIZE, iLength));
	}

	/* write the data from that is given to the write point */
	thecore_memcpy(buffer->write_point, src, iLength);
	buffer_write_proceed(buffer, iLength);
}

/***
 * buffer_write_peek - Returns the current write position in the buffer
 * @buffer: The buffer whose write point is to be returned
 *
 * This function returns a pointer to the current position in the buffer
 * where the next write operation will begin. It does not modify the buffer,
 * but allows for inspection of the write point, which is usually empty point
 * right after the last character.
 *
 * Return: A pointer to the current write position in the buffer.
 */
void* buffer_write_peek(LPBUFFER buffer)
{
	return (buffer->write_point);
}

/**
 * buffer_write_proceed - Advances the write position in the buffer after data is written
 * @buffer: The buffer whose write position is to be updated
 * @iLength: The number of bytes written to the buffer
 *
 * This function updates the buffer's internal tracking of the amount of data
 * written. It increases the buffer's total length, adjusts the write pointer
 * to reflect the new position, and increments the write point position to
 * account for the newly written data.
 *
 * Return: Nothing (void.)
 */
void buffer_write_proceed(LPBUFFER buffer, int32_t iLength)
{
	buffer->length += iLength;
	buffer->write_point += iLength;
	buffer->write_point_pos += iLength;
}

/***
 * buffer_read - Reads data from the buffer
 * @buffer: The buffer to read data from
 * @buf: The destination buffer where the data will be copied
 * @iBytes: The number of bytes to read
 *
 * This function copies the specified number of bytes from the buffer's
 * read position into the provided destination buffer. After copying the data,
 * it advances the buffer's read position by the number of bytes read.
 *
 * Return: Nothing (void.)
 */
void buffer_read(LPBUFFER buffer, void* buf, int32_t iBytes)
{
	thecore_memcpy(buf, buffer->read_point, iBytes);
	buffer_read_proceed(buffer, iBytes);
}

/***
 * buffer_read_proceed - Advances the read position after reading data
 * @buffer: The buffer whose read position will be updated
 * @iLength: The number of bytes to advance the read position by
 *
 * This function moves the read position forward by the specified number of
 * bytes. If the length to advance is zero, it does nothing. If the length is
 * negative or exceeds the available data in the buffer, an error is logged.
 * If the buffer has more data than the length to process, only the read point
 * is moved. Otherwise, the buffer is reset.
 *
 * Return: Nothing (void.)
 */
void buffer_read_proceed(LPBUFFER buffer, int32_t iLength)
{
	if (iLength == 0)
	{
		return;
	}

	if (iLength < 0)
	{
		sys_err("buffer_read_proceed: length argument is lower than zero [%d]", iLength);
	}
	else if (iLength > buffer->length)
	{
		sys_err("buffer_read_proceed: length argument is bigger than buffer length [Length: %d] [Buffer Length: %d]", iLength, buffer->length);
		iLength = buffer->length;
	}

	/* If the length to be processed is less than the buffer length, only the read point is adjusted. */
	if (iLength < buffer->length)
	{
		/* Ensure the read point is within bounds. */
		if (buffer->read_point + iLength - buffer->mem_data > buffer->mem_size)
		{
			sys_err("buffer_read_proceed: buffer overflow! [Length: %d] [Read Point: %d]", iLength, buffer->read_point - buffer->mem_data);
			abort();
		}

		buffer->read_point += iLength;
		buffer->length = iLength;
	}
	else
	{
		/* If the entire buffer has been processed, reset the buffer. */
		buffer_reset(buffer);
	}
}

/***
 * buffer_read_peek - Returns a pointer to the current read position
 * @buffer: The buffer to peek into
 *
 * This function returns a constant pointer to the current read position
 * within the buffer. It allows inspection of the buffer contents at the read
 * position without modifying any internal state.
 *
 * Return: A constant pointer to the read point within the buffer
 */
const void* buffer_read_peek(LPBUFFER buffer)
{
	return (static_cast<const void*>(buffer->read_point));
}

/***
 * buffer_has_space - Determines how much space is left in the buffer
 * @buffer: The buffer to check for available space
 *
 * This function calculates the remaining space in the buffer by subtracting
 * the current write position from the total size of the allocated memory.
 * It returns the number of bytes left that can still be written to the buffer.
 *
 * Return: The amount of free space (in bytes) available in the buffer.
 */
int32_t buffer_has_space(LPBUFFER buffer)
{
	return (buffer->mem_size - buffer->write_point_pos);
}

/***
 * buffer_size - Get the current size of the buffer.
 * @buffer: The buffer to query.
 *
 * This function returns the current length of data in the buffer.
 *
 * Return: The length of the buffer.
 */
uint32_t buffer_size(LPBUFFER buffer)
{
	return (buffer->length);
}

/***
 * buffer_adjust_size - Adjusts the buffer size to accommodate additional data
 * @buffer: The buffer to be resized (passed by reference)
 * @iAdded_Size: The additional size needed in the buffer
 *
 * This function checks if the current size of the buffer is sufficient to
 * accommodate the specified additional size. If the buffer already has enough
 * space, no action is taken. If additional space is needed, the buffer is
 * reallocated to increase its size. A log entry is made to record the
 * adjustment.
 *
 * Return: Nothing (void.)
 */
void buffer_adjust_size(LPBUFFER& buffer, int32_t iAdded_Size)
{
	/* if current buffer size is bigger or equal to needed size, do nothing */
	if (buffer->mem_size >= buffer->write_point_pos + iAdded_Size)
	{
		return;
	}

	sys_log(0, "buffer_adjust_size: %d size have been added to the buffer, current : %d/%d", iAdded_Size, buffer->length, buffer->mem_size);
	buffer_realloc(buffer, buffer->mem_size + iAdded_Size);
}

/***
 * buffer_get_byte - Reads a BYTE value from the buffer
 * @buffer: The buffer to read from
 *
 * This function retrieves a single BYTE value from the current read position
 * of the buffer. It first checks if there is enough data available to read
 * a BYTE. If not, an error is logged, and a default value of 0 is returned.
 * After reading, the read position is updated.
 *
 * Return: The BYTE value read from the buffer, or 0 if there is not enough data.
 */
uint8_t buffer_get_byte(LPBUFFER buffer)
{
	/* Ensure the buffer has enough data to read a BYTE */
	if (buffer->length < sizeof(uint8_t))
	{
		sys_err("buffer_get_byte: not enough data in buffer to read a BYTE");
		return 0;
	}

	/* Read the BYTE value from the current read point */
	uint8_t val = *(uint8_t*)buffer->read_point;

	/* Advance the read position by the size of a BYTE */
	buffer_read_proceed(buffer, sizeof(uint8_t));

	/* Return the read BYTE value */
	return val;
}

/***
 * buffer_get_word - Reads a WORD value from the buffer
 * @buffer: The buffer to read from
 *
 * This function retrieves a WORD value from the current read position of
 * the buffer. It checks if there is enough data to read a WORD. If
 * insufficient data is available, an error is logged, and 0 is returned.
 * The read position is updated after reading the value.
 *
 * Return: The WORD value read from the buffer, or 0 if there is not enough data.
 */
uint16_t buffer_get_word(LPBUFFER buffer)
{
	/* Ensure the buffer has enough data to read a WORD */
	if (buffer->length < sizeof(uint16_t))
	{
		sys_err("buffer_get_word: not enough data in buffer to read a WORD");
		return 0;
	}

	/* Read the WORD value from the current read point */
	uint16_t val = *(uint16_t*)buffer->read_point;

	/* Advance the read position by the size of a WORD */
	buffer_read_proceed(buffer, sizeof(uint16_t));

	/* Return the read WORD value */
	return val;
}

/***
 * buffer_get_dword - Reads a DWORD value from the buffer
 * @buffer: The buffer to read from
 *
 * This function retrieves a DWORD value from the current read position of
 * the buffer. It checks if there is sufficient data available to read a
 * DWORD. If not, an error is logged, and 0 is returned. The read position
 * is updated after reading the value.
 *
 * Return: The DWORD value read from the buffer, or 0 if there is not enough data.
 */
uint32_t buffer_get_dword(LPBUFFER buffer)
{
	/* Ensure the buffer has enough data to read a DWORD */
	if (buffer->length < sizeof(uint32_t))
	{
		sys_err("buffer_get_dword: not enough data in buffer to read a DWORD");
		return 0;
	}

	/* Read the DWORD value from the current read point */
	uint32_t val = *(uint32_t*)buffer->read_point;

	/* Advance the read position by the size of a DWORD */
	buffer_read_proceed(buffer, sizeof(uint32_t));

	/* Return the read DWORD value */
	return val;
}
