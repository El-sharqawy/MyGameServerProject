#include "stdafx.h"
#include <iostream>

int main()
{
    setlocale(LC_ALL, "" );
    const char* hello = "Testooooos";
    std::wstring newFileName = convertToWString(hello);
    wprintf(L"%ls\n", newFileName.c_str());  
    auto testString = convertWCharToChar(newFileName);
    printf("%s\n", testString.c_str());  

    unsigned long seed = static_cast<unsigned long>(time(0));
    srand(seed);
    if (!logs_init())
    {
        fprintf(stderr, "Failed to Initialize logs");
        return (EXIT_FAILURE);
    }

    //delete[] newFileName;
    //std::cin.get();
    return EXIT_SUCCESS;
}

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