#include "stdafx.h"

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