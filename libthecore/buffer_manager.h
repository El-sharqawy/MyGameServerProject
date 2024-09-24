#pragma once

#include "stdafx.h"

class CTempBuffer
{
public:
	/* Constructor for the CTempBuffer class */
	CTempBuffer(int32_t iSize = 8192, bool bForceDelete = false);

	/* Destructor for the CTempBuffer class */
	~CTempBuffer();

	/* Writes data to the buffer */
	void Write(const void* pData, int32_t iLength);

	/* Writes data to the buffer */
	template <typename T>
	void Write(const T& pData, int32_t iLength)
	{
		sys_log(0, "CTempBuffer::Write Used Template& Function");
		buffer_write(m_bBuffer, &pData, iLength);
	}

	/* Peeks at the data in the buffer without removing it */
	const void* ReadPeek();

	/* Reads data from the buffer */
	void Read(void* pData, int32_t iSize);

	/* Reads data from the buffer */
	template <typename T>
	void Read(T& pData, int32_t iSize)
	{
		sys_log(0, "CTempBuffer::Read Used Template& Function")
			buffer_read(m_bBuffer, &pData, iSize);
	}

	/* Returns a Pointer to the buffer */
	LPBUFFER GetBuffer() const;

	/* Gets the size of the buffer */
	uint32_t GetSize();

	/* Resets the buffer to its initial state */
	void Reset();

private:

	/* a pointer to the buffer of the class */
	LPBUFFER m_bBuffer;

	/* when true, force to delete the buffer after using it */
	bool m_bForceDelete;
};