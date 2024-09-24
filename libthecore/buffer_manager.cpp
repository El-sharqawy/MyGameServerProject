#include "stdafx.h"
#include "buffer_manager.h"

/***
 * CTempBuffer::CTempBuffer - Constructor for the CTempBuffer class
 * @iSize: Initial size of the buffer
 * @bForceDelete: Boolean flag indicating whether the buffer size should be forced to a minimum
 *
 * This constructor initializes the CTempBuffer object. If the bForceDelete flag is set to true,
 * it ensures that the buffer size is at least 128KB by using the std::max function. The buffer
 * is then created using the buffer_new function, with the final size passed as an argument.
 * 
 * Return: Nothing (Constructor)
 */
CTempBuffer::CTempBuffer(int32_t iSize, bool bForceDelete)
{
	m_bForceDelete = bForceDelete;

	if (bForceDelete)
	{
		iSize = std::max<int32_t>(iSize, 128 * 1024);
	}

	m_bBuffer = buffer_new(iSize);
}

/***
 * CTempBuffer::~CTempBuffer - Destructor for the CTempBuffer class
 *
 * This destructor is responsible for cleaning up the memory allocated for the buffer.
 * It calls buffer_delete to free the buffer, and logs a message indicating that the
 * buffer has been deleted.
 *
 * Return: Nothing (Destructor)
 */
CTempBuffer::~CTempBuffer()
{
	buffer_delete(m_bBuffer);
}

/***
 * CTempBuffer::Write - Writes data to the buffer
 * @pData: Pointer to the data to be written
 * @iLength: Length of the data to be written
 *
 * This function writes a specified length of data from the provided pointer
 * to the internal buffer using the buffer_write function.
 * 
 * Return: Nothing (void)
 */
void CTempBuffer::Write(const void* pData, int32_t iLength)
{
	sys_log(0, "CTempBuffer::Write Called void* Function");
	buffer_write(m_bBuffer, pData, iLength);
}

/***
 * CTempBuffer::ReadPeek - Peeks at the data in the buffer without removing it
 *
 * Returns a constant pointer to the data in the buffer without altering the
 * buffer's state. This allows for examining the data without consuming it.
 *
 * Return: A pointer to the current data in the buffer.
 */
const void* CTempBuffer::ReadPeek()
{
	return (buffer_read_peek(m_bBuffer));
}

/***
 * CTempBuffer::Read - Reads data from the buffer
 * @pData: Pointer to the memory where the read data will be stored
 * @iSize: Number of bytes to read from the buffer
 *
 * This function logs the call to the Read function and reads a specified
 * number of bytes from the buffer into the provided memory location
 * using the buffer_read function.
 *
 * Return: Nothing (void).
 */
void CTempBuffer::Read(void* pData, int32_t iSize)
{
	sys_log(0, "CTempBuffer::Read Called void* Function");
	buffer_read(m_bBuffer, pData, iSize);
}

/***
 * CTempBuffer::GetBuffer - returns a Pointer to the buffer
 *
 * This function returns a pointer to the buffer managed by the
 * CTempBuffer object. This function is marked as const, ensuring that it
 * does not modify the state of the object.
 *
 * Return: A pointer to the buffer (LPBUFFER).
 */
LPBUFFER CTempBuffer::GetBuffer() const
{
	return (m_bBuffer);
}

/***
 * CTempBuffer::GetSize - Gets the size of the buffer
 *
 * This function returns the current size of the buffer using the
 * buffer_size function, providing the caller with information about the
 * amount of data stored in the buffer.
 *
 * Return: The size of the buffer in bytes (uint32_t).
 */
uint32_t CTempBuffer::GetSize()
{
	return (buffer_size(m_bBuffer));
}

/***
 * CTempBuffer::Reset - Resets the buffer to its initial state
 *
 * This function calls buffer_reset to clear the contents of the buffer
 * managed by the CTempBuffer object, effectively resetting its
 * state and preparing it for new data.
 * 
 * Return: Nothing (void).
 */
void CTempBuffer::Reset()
{
	buffer_reset(m_bBuffer);
}