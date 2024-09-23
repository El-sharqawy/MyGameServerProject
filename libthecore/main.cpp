#include "stdafx.h"
#include <iostream>

void test_buffer_get_byte()
{
    LPBUFFER buffer = buffer_new(10);
    const char* testData = "abcdefghij"; // 10 bytes of test data
    buffer_write(buffer, testData, 10); // Write 10 bytes to the buffer

    buffer->read_point = buffer->mem_data; // Reset read point

    BYTE byte = buffer_get_byte(buffer);
    std::cout << "Read BYTE: " << (int)byte << std::endl; // Expect 'a' (97)

    buffer_delete(buffer);
}

void test_buffer_get_word()
{
    LPBUFFER buffer = buffer_new(10);
    const char* testData = "abcdefghij"; // 10 bytes of test data
    buffer_write(buffer, testData, 10); // Write 10 bytes to the buffer

    buffer->read_point = buffer->mem_data; // Reset read point

    WORD word = buffer_get_word(buffer);
    std::cout << "Read WORD: " << word << std::endl; // Expect 'ab' (25985 in little-endian)

    buffer_delete(buffer);
}

void test_buffer_get_dword()
{
    LPBUFFER buffer = buffer_new(33);

    std::cout << "Read DWORD: " << buffer->mem_data << "Buffer Length : " << buffer->length << "sizeof dword : " << sizeof(DWORD) << std::endl; // Expect 'abcd' (1684234849 in little-endian)

    const char* testData = "abcdefghijklmnopqrstuvwxyz"; // 10 bytes of test data

    buffer_write(buffer, testData, 32); // Write 10 bytes to the buffer

    buffer->read_point = buffer->mem_data; // Reset read point

    DWORD dword = buffer_get_dword(buffer);
    std::cout << "Read DWORD: " << buffer->mem_data << "Buffer Length : " << buffer->length << std::endl; // Expect 'abcd' (1684234849 in little-endian)

    buffer_delete(buffer);
}

int main()
{
    // Example 1: Create a buffer from the pool
    std::cout << "Creating a new buffer with size 512" << std::endl;
    LPBUFFER buffer = buffer_new(512);
    if (!buffer) {
        std::cerr << "Failed to create buffer!" << std::endl;
        return 1;
    }

    // Example 2: Write to the buffer
    const char* data = "Hello, this is a test!";
    int dataLength = strlen(data);

    std::cout << "Writing data to buffer..." << std::endl;
    buffer_write(buffer, data, dataLength);

    // Check the buffer content using the peek function
    const char * writePoint = static_cast<const char*>(buffer_write_peek(buffer));

    std::cout << "Peek into the write point: " << writePoint - 1 << std::endl;

    // Example 3: Check remaining space in buffer
    int remainingSpace = buffer_has_space(buffer);
    std::cout << "Remaining space in buffer: " << remainingSpace << " bytes" << std::endl;

    // Example 4: Read from the buffer
    char readData[512];
    std::cout << "Reading data from buffer..." << std::endl;
    buffer_read(buffer, readData, dataLength);
    readData[dataLength] = '\0';  // Null-terminate the read string

    std::cout << "Data read from buffer: " << readData << std::endl;

    // Example 5: Use buffer_get_byte, buffer_get_word, buffer_get_dword
    std::cout << "Writing new data for byte, word, and dword testing" << std::endl;
    const char* newData = "\x01\x02\x03\x04";  // Data: 0x01, 0x02, 0x03, 0x04
    buffer_write(buffer, newData, 4);

    // Reset buffer read/write points for reading
    buffer_reset(buffer);

    // Read byte
    BYTE byteValue = buffer_get_byte(buffer);
    std::cout << "BYTE value read from buffer: " << static_cast<int>(byteValue) << std::endl;

    // Read word (2 bytes)
    WORD wordValue = buffer_get_word(buffer);
    std::cout << "WORD value read from buffer: " << wordValue << std::endl;

    // Read dword (4 bytes)
    DWORD dwordValue = buffer_get_dword(buffer);
    std::cout << "DWORD value read from buffer: " << dwordValue << std::endl;

    // Example 6: Adjust buffer size
    std::cout << "Adjusting buffer size..." << std::endl;
    buffer_adjust_size(buffer, 1024);  // Add 1024 bytes
    std::cout << "Buffer size adjusted. New size: " << buffer->mem_size << std::endl;

    // Example 7: Reuse buffer via pool
    std::cout << "Deleting buffer and returning to pool..." << std::endl;
    buffer_delete(buffer);

    // Create a new buffer to reuse pooled memory
    std::cout << "Creating a new buffer (pooled memory should be reused)" << std::endl;
    buffer = buffer_new(512);
    if (!buffer)
    {
        std::cerr << "Failed to create new buffer!" << std::endl;
        return 1;
    }

    buffer_delete(buffer);

    return 0;
}

/*int main()
{
    setlocale(LC_ALL, "" );
    unsigned long seed = static_cast<unsigned long>(time(0));
    srand(seed);

    if (!logs_init())
    {
        fprintf(stderr, "Failed to Initialize logs");
        return (EXIT_FAILURE);
    }

    printf("%llu\n", get_unsigned_time());
    printf("%.03f\n", get_float_time());
    auto timeVal = timeval{ 5, 1 };
    thecore_sleep(&timeVal);
    return EXIT_SUCCESS;
}*/

/*int main()
{
    char token_string[256];
    char value_string[256];
    char buf[256];

    const char *configName = "CONFIG";
	FILE *fConfigFile = nullptr;

	if (!(fConfigFile = fopen(configName, "r")))
	{
		fprintf(stderr, "Can not open [%s]\n", configName);
		exit(1);
	}

    while (fgets(buf, 256, fConfigFile))
    {
        parse_token(buf, token_string, value_string);
        TOKEN("Name")
        {
            printf("%s: %s\n", token_string, value_string);
            continue;
        }

        TOKEN("From")
        {
            printf("%s: %s\n", token_string, value_string);
            continue;
        }
    }

    int fileSize = filesize(fConfigFile);

    printf("filesize : %d", fileSize);
    fclose(fConfigFile);
    return 0;
}*/