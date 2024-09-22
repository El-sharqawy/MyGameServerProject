#pragma once

#if !defined(_WIN64)

#if !defined(__cplusplus)
typedef unsigned char   bool;
#endif

typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned char BOOL;
typedef unsigned long ULONG;
typedef unsigned long long ULONGLONG;
typedef unsigned int UINT;
typedef short SHORT;
typedef int INT;
typedef long LONG;
typedef long long LONGLONG;

typedef int socket_t;

#else

typedef SOCKET socket_t;

typedef unsigned int uint;

#endif