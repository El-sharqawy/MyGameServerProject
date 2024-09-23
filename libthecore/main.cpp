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
    setlocale(LC_ALL, "");
    unsigned long seed = static_cast<unsigned long>(time(0));
    srand(seed);

    if (!logs_init())
    {
        fprintf(stderr, "Failed to Initialize logs");
        return (EXIT_FAILURE);
    }

    // Example 1: Create a buffer from the pool
    std::cout << "Creating a new buffer with size 512" << std::endl;
    LPBUFFER buffer = buffer_new(512);
    if (!buffer)
    {
        std::cerr << "Failed to create buffer!" << std::endl;
        return 1;
    }


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