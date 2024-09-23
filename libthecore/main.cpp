#include "stdafx.h"
#include <iostream>


#define PLAYER_MAX_NAME 64
enum Packets
{
    HEADER_GC_TEST = 1,
};

typedef struct SPacketGCTest
{
    int32_t iHeader;
    char szPlayerName[PLAYER_MAX_NAME + 1];
} TPacketGCTest;

int main()
{
    // Create new Buffer
    auto buffer = buffer_new(128);
    if (!buffer)
    {
        std::cerr << "Failed to create new buffer!" << std::endl;
        return (EXIT_FAILURE);
    }

    TPacketGCTest testPack{};
    testPack.iHeader = HEADER_GC_TEST;
    const char* playerName = "sharqawy";
    strncpy(testPack.szPlayerName, playerName, sizeof(testPack.szPlayerName));

    buffer_write(buffer, &testPack, sizeof(testPack));

    TPacketGCTest testPackReceived{};
    buffer_read(buffer, &testPackReceived, sizeof(testPackReceived));

    std::cout << "Header Num: " << testPackReceived.iHeader << std::endl;
    std::cout << "Header szPlayerName:: " << testPackReceived.szPlayerName << std::endl;
    // free the buffer before leaving the app
    buffer_delete(buffer);

    return (EXIT_SUCCESS);
}