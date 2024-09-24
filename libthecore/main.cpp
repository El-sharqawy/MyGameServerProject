#include "stdafx.h"
#include <iostream>

#define PLAYER_MAX_NAME 64

enum Packets
{
    HEADER_GC_NONE,
    HEADER_GC_TEST,
};

typedef struct SPacketGCTest
{
    int32_t iHeader;
    char szPlayerName[PLAYER_MAX_NAME + 1];
} TPacketGCTest;

int main()
{
    // Create new Buffer
    CTempBuffer buf;

    TPacketGCTest testPack;
    testPack.iHeader = HEADER_GC_TEST;
    const char* playerName = "sharqawy";
    strncpy(testPack.szPlayerName, playerName, sizeof(testPack.szPlayerName));

    buf.Write(testPack, sizeof(testPack));

    std::cout << "Buffer Size: " << buf.GetSize() << std::endl;

    TPacketGCTest testPackReceived;
    buf.Read(&testPackReceived, sizeof(testPackReceived));

    std::cout << "Header Num: " << testPackReceived.iHeader << std::endl;
    std::cout << "Header szPlayerName:: " << testPackReceived.szPlayerName << std::endl;
    //buffer_write(buffer, str.c_str(), stringLen);

    // free the buffer before leaving the app
    //buffer_delete(buffer);

    return (EXIT_SUCCESS);
}