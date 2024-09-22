#define __LIBTHECORE__
#include "stdafx.h"

static struct timeval null_time = {0, 0};

/*** check if the char in UTF-8 printable range or Having higher bits that match certain conditions (specifically, if the top 3 bits indicate a value greater than 0x90). ***/
#define isHexPrint(x)   (isutf8(x) || isprint(x))

/**
 * str_dup - Allocate memory, copy the source, and return it.
 * @source: source to copy from.
 * @return: the copied allocated memory characters.
 */
char *str_dup(const char *source)
{
    char *newline;
    CREATE(newline, char, strlen(source) + 1);
    return (strcpy(newline, source));
}

/**
 * printData - Print data in both hex and ASCII (used for packet analysis, etc.)
 * @data: given data to print.
 * @bytes: number of bytes given.
 * @return: Nothing (void).
 * 
 * Example of Usage : 
 * unsigned char data[] = {
 *      0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x21, 0x0A, 0x90, 0xA3,   // Hello World
 *      0x01, 0x02, 0x03, 0x04, 0x45, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65, 0x65                      // Example
 * };
 * int size = sizeof(data);
 * printData(data, size);
 *
 */
void printData(const unsigned char *data, int bytes)
{
    int k, j;
    const unsigned char *p;

    fprintf(stderr, "------------------------------------------------------------------\n");
    j = bytes;

    while (true)
    {
        k = j >= 32 ? 32 : j;
        p = data;

        for (int i = 0; i < 32; i++)
        {
            if (i >= k)
            {
                fprintf(stderr, " ");
            }
            else
            {
                fprintf(stderr, "%02x ", *p);
            }
            p++;
        }

        fprintf(stderr, "| ");

        p = data;

        for (int i = 0; i < k; i++)
        {
            if (i >= k)
            {
                fprintf(stderr, " ");
            }
            else
            {
                fprintf(stderr, "%c", isHexPrint(*p) && isHexPrint(*(p + 1)) ? *p : '.');
            }
            p++;
        }

        fprintf(stderr, "\n");

        j -= 32;
        data += 32;

        if (j <= 0)
        {
            break;
        }
    }
    fprintf(stderr, "------------------------------------------------------------------\n");
}

/**
 * filesize - Returns given file size
 * @fp: given file.
 * @return: file size.
 */
int filesize(FILE *fp)
{
    int pos, size;

    pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, pos, SEEK_SET);

    return (size);
}

/**
 * parse_token - a string which is in form of "Item: Value" like "Name: Osama"
 * Extracts the item as a token and value as value then returns them (in memory).
 * @src: given string to parse.
 * @token: the token which will be extracted from the string.
 * @value: the value of the item which will be extracted as well.
 * @return: Nothing (void).
 */
void parse_token(char *src, char *token, char *value)
{
    char *temp;

    /*** loop through the string ***/
    for (temp = src; *temp && *temp != ':'; temp++)
    {
        /*** if there's a space, ignore it and continue ***/
        if (isspace(*temp))
        {
            continue;
        }

        /*** lower the string of the token ***/
        *(token++) = LOWER(*temp);
    }

    /*** Add Null Terminator At the end of the token ***/
    *token = '\0';

    /*** Loop Through Value starting from : + space after (+= 2) ***/
    for (temp += 2; *temp; temp++)
    {
        /*** ignore any newlines chars */
        if (*temp == '\n' || *temp == '\r')
        {
            continue;
        }

        *(value++) = *temp;
    }

    /*** Add Null Terminator At the end of the value ***/
    *value = '\0';
}

/**
 * core_dump_unix - Dumping the Core Outputs
 * @who: the file name which the function have been called in.
 * @line: the line which this function have been called at.
 * @return: Nothing (void).
 */
void core_dump_unix(const char *who, WORD line)
{
#if !defined(_WIN64)
    sys_err("*** Dumping Core %s:%d", who, line);

    fflush(stderr);
    fflush(stdout);

    if (fork() == 0)
    {
        abort();
    }
#endif
}

/***
 * trim_and_lower - a function that trims white space around the string (space between chars not included)
 * and Lower case it
 * @src: given string to trim and lower.
 * @dest: the result string to put the end-result into.
 * @dest_size: the size of the given result string.
 * @return: Nothing.
 */
void trim_and_lower(char *src, char* dest, size_t dest_size)
{
    const char *temp = src;
    size_t len = 0;

    if (!dest || dest_size == 0)
    {
        return;
    }

    if (!src)
    {
        *dest = '\0';
        return;
    }

    /*** Skip blank space in front ***/
    while (*temp)
    {
        if (!isspace(*temp))
        {
            break;
        }

        temp++;
    }

    /*** Get "\0" ***/
    --dest_size;

    while (*temp && len < dest_size)
    {
        /*** LOWER is a macro, so you shouldn't use ++ ***/
        *(dest++) = LOWER(*temp);
        ++temp;
        ++len;
    }

    /*** Add the null terminator at the end ***/
    *dest = '\0';

    /*** Erase blank space at the end ***/
    if (len > 0)
    {
        --dest;

        while (*dest && isspace(*dest) && len--)
        {
            /*** swap whitespace with null terminator ***/
            *(dest--) = '\0';
        }
    }
}

/***
 * lower_string - a function that lower cases the entire string, keeping the whitespaces.
 * @src: given string to trim and lower.
 * @dest: the result string to put the end-result into.
 * @dest_size: the size of the given result string.
 * @return: Nothing.
 */
void lower_string(char *src, char* dest, size_t dest_size)
{
    const char *temp = src;
    size_t len = 0;

    if (!dest || dest_size == 0)
    {
        return;
    }

    /*** if the given string is null ***/
    if (!src)
    {
        /*** make the destination string just empty string ***/
        *dest = '\0';
        return;
    }

    /*** Get "\0" ***/
    --dest_size;

    while (*temp && len < dest_size)
    {
        /*** LOWER is a macro, so you shouldn't use ++ ***/
        *(dest++) = LOWER(*temp);
        ++temp;
        ++len;
    }

    /*** Add the null terminator at the end ***/
    *dest = '\0';
}


/***
 * time_str - a function that takes current time, modify and trim it
 * then return it as string.
 * @curtime: given current time.
 * @return: Current time as string.
 */
char *time_str(time_t curtime)
{
    static char *time_s;
    /*** Example of this line time_s: Wed Sep 20 13:45:30 2024\n\0 ***/
    time_s = asctime(localtime(&curtime));

    /*** trim last 6 characters (which is newline and the year) ***/
    time_s[strlen(time_s) - 6] = '\0';

    /*** trim first 4 characters, which is the name of the day followed by whitespace ***/
    time_s += 4;

    /*** time_s final result example : Sep 20 13:45:30 ***/
    return (time_s);
}

/***
 * gettimeofday - a function that gets the time of given struct.
 * @time: given time to calculate.
 * @dummy: a dummy parameter that is needed for Linux function.
 * @return: Nothing (void)
 */
void gettimeofday(struct timeval* time, struct timezone *dummy)
{
    unsigned int uiMilliseconds = GetTickCount();
    time->tv_sec = (uiMilliseconds / 1000);
    time->tv_usec = (uiMilliseconds % 1000) * 1000;
}
/***
 * timediff - a function that calculates the differnce between two given time structs and return it
 * @a: first time as base time
 * @b: the second time which will be used in the calculation from time A
 * @return: the result 
 */
struct timeval *timediff(const struct timeval *a, const struct timeval *b)
{
    /*** declare difference result struct ***/
    static struct timeval result;

    /*** if given b seconds is bigger than A, return null time ***/
    if (a->tv_sec < b->tv_sec)
    {
        return (&null_time);
    }
    else if (a->tv_sec == b->tv_sec)
    {
        /*** if given b mico seconds is bigger than A, return null time ***/
        if (a->tv_usec < b->tv_usec)
        {
            return (&null_time);
        }
        else
        {
            result.tv_sec = 0;
            result.tv_usec = a->tv_usec - b->tv_usec; /*** calculate the difference between a and b micro seconds ***/
            return (&result);
        }
    }
    else /*** a->tv_sec > b->tv_sec ***/
    {
        /*** calculate the difference between a and b seconds ***/
        result.tv_sec = a->tv_sec - b->tv_sec;

        /*** if b micro seconds bigger than a micro seconds ***/
        if (a->tv_usec < b->tv_usec)
        {
            /*** add a whole one second (1,000,000 micro second) into micro second field and remove 1 second from the seconds field  ***/
            result.tv_usec = a->tv_usec + 1000000 - b->tv_usec;
            result.tv_sec--;
        }
        else
        {
            result.tv_usec = a->tv_usec - b->tv_usec;
        }

        return (&result);
    }
    return (&null_time);
}

/***
 * timeadd - a function that adds the time of b to the time of a and returns it
 * @a: first time as base time
 * @b: the second time which will be used to be added to time A
 * @return: the result 
 */
struct timeval *timeadd(struct timeval *a, struct timeval *b)
{
    static struct timeval result;

    result.tv_sec = a->tv_sec + b->tv_sec;
    result.tv_usec = a->tv_usec + b->tv_usec;

    /*** every 1,000,000 micro second is equal to 1 second ***/
    while (result.tv_usec >= 1000000)
    {
        result.tv_usec -= 1000000;
        result.tv_sec++;
    }

    return (&result);
}

/***
 * tm_calculate - a function that returns the date that is 'days' days after the current time 'curr_tm'
 * @curr_tm: current time.
 * @days: number of days
 * @return: the new time calculated.
 */
struct tm *tm_calculate(const struct tm* curr_tm, int days)
{
    bool bYoonYear = false;
    static struct tm new_time;
    int monthdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    /*** if given time is null ***/
    if (!curr_tm)
    {
        /*** set new_time to current time ***/
        time_t time_s = time(0);
        new_time = *localtime(&time_s);
    }
    else
    {
        /*** copy current given time to new_time ***/
        thecore_memcpy(&new_time, curr_tm, sizeof(struct tm));
    }

    if (new_time.tm_mon == 1)
    {
        /*** check if yoon year, division result by 4 should be 0 ***/
        if (!((new_time.tm_year + 1900) % 4))
        {
            /*** check if yoon year, division result by 100 should be 1, otherwise conitnue ***/
            if (!((new_time.tm_year + 1900) % 100))
            {
                /*** check if yoon year, division result by 400 should be 0 ***/
                if (!((new_time.tm_year + 1900) % 400))
                {
                    bYoonYear = true;
                }
            }
            else /*** division result by 100 is 1, means its yoon year ***/
            {
                bYoonYear = true;
            }
        }

        /*** if it's yoon year, add day to current days ***/
        if (bYoonYear)
        {
            new_time.tm_mday += 1;
        }
    }

    /*** set the maximum days in the calendar to 29 instead of 28 in yoon year ***/
    if (bYoonYear)
    {
        monthdays[1] = 29;
    }
    else
    {
        monthdays[1] = 28;
    }

    new_time.tm_mday += days;

    if (new_time.tm_mday <= 0)
    {
        new_time.tm_mon--;
    
        if (new_time.tm_mon <= 0)
        {
            new_time.tm_year--;
            new_time.tm_mon = 11;
        }

        new_time.tm_mday = monthdays[new_time.tm_mon] + new_time.tm_mday;
    }
    else if (new_time.tm_mday > monthdays[new_time.tm_mon])
    {
        new_time.tm_mon++;

        if (new_time.tm_mon > 11)
        {
            new_time.tm_year++;
            new_time.tm_mon = 0;
        }

        new_time.tm_mday = monthdays[new_time.tm_mon] - new_time.tm_mday;
    }

    return (&new_time);
}

/***
 * thecore_random - generates an unsigned random number.
 * @return: the generated number.
 */
unsigned int thecore_random()
{
#if defined(_WIN64)
    return (rand());
#else
    return (random());
#endif
}

/***
 * number_ex - generates a random number in given range.
 * @from: the first number to generate from (minimum value).
 * @to: the second number to generate to (maximum value).
 * @file: the name of the file which the function have been called at.
 * @line: the number of the line in the file which the function have been called at.
 * @return: the generated random number.
 */
int number_ex(int from, int to, const char *file, int line)
{
    /*** if the minimum value is bigger than the maxmimum value ***/
    if (from > to)
    {
        int temp = from;

        /*** print in the stderr to send warning ***/
        sys_err("number(): first argument is bigger than second argument %d -> %d, file: %s line: %d", from, to, file, line);

        /*** modify it, and swap the values to make from has the value of to, and continue. */
        from = to;
        to = temp;
    }

    int randomNum = 0;

    /*** make sure it is not 0, so we do not divide by 0 */
    if ((to - from + 1) != 0)
    {
        randomNum = ((thecore_random() % (to - from + 1) + from));
    }
    else
    {
        sys_err("number(): divide by 0!");
    }

    return (randomNum);
}

/***
 * fnumber - generates a random number in given range with float type.
 * @from: the first number to generate from (minimum value).
 * @to: the second number to generate to (maximum value).
 * @file: the name of the file which the function have been called at.
 * @line: the number of the line in the file which the function have been called at.
 * @return: the generated random number.
 */
float fnumber_ex(float from, float to, const char *file, int line)
{
    /*** if the minimum value is bigger than the maxmimum value ***/
    if (from > to)
    {
        float temp = from;

        /*** print in the stderr to send warning ***/
        sys_err("fnumber(): first argument is bigger than second argument %.1f -> %.1f, file: %s line: %d", from, to, file, line);

        /*** modify it, and swap the values to make from has the value of to, and continue. */
        from = to;
        to = temp;
    }

    float randomNum = 0;

    randomNum = (static_cast<float>(thecore_random()) / static_cast<float>(RAND_MAX)) * (to - from) + from;
    return (randomNum);
}

// Convert char* to LPCWSTR
/***
 * convertToWString - converts normal C String into wstring to use in Windows
 * @c: given normal C string to convert.
 * @return: a wstring that C string converted to and will be used.
 */
std::wstring convertToWString(const char *c)
{
    const size_t cSize = strlen(c)+1;
    const size_t length = strlen(c);
    std::wstring text_wchar(length, L'#');
    mbstowcs(&text_wchar[0], c , length);

    return text_wchar;
}

// Convert char* to LPCWSTR
/***
 * convertWCharToChar - converts wchar String into normal string to use in Windows
 * @c: given normal wstring string to convert into string.
 * @return: a string that is converted from wstring and will be used.
 */
std::string convertWCharToChar(const std::wstring& wideStr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string narrowStr(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &narrowStr[0], size_needed, NULL, NULL);
    return narrowStr;
}
