#pragma once

#include <fstream>
#if defined(_WIN64)
    #include <windows.h>
#endif

/* safely freeing pointer from the memory */
#define SAFE_FREE(p)    { \
                            if (p) \
                            { \
                                free((void *) p); \
                                (p) = nullptr;\
                            } \
                        } \

/* safely deleting pointer from the memory */
#define SAFE_DELETE(p)    { \
                            if (p) \
                            { \
                                delete(p); \
                                (p) = nullptr;\
                            } \
                        } \

/* safely deleting an array pointer from the memory */
#define SAFE_DELETE_ARRAY(p)    { \
                            if (p) \
                            { \
                                delete [] (p); \
                                (p) = nullptr;\
                            } \
                        } \

/* safely releasing pointer from the memory */
#define SAFE_RELEASE(p)    { \
                            if (p) \
                            { \
                                p->Release(); \
                                (p) = nullptr;\
                            } \
                        } \

/* Make the character in lower case */
#define LOWER(c) (((c) >='A' && (c) <= 'Z') ? ((c) + ('a' - 'A')) : (c))

/* Make the character in upper case */
#define UPPER(c) (((c) >='a' && (c) <= 'z') ? ((c) + ('A' - 'a')) : (c))

/* compare strings */
#define str_cmp strcasecmp

/* copies a string to new one */
#define STRNCPY(dest, src, len) do { \
                                    strncpy(dest, src, len); \
                                    dest[len] = '\0'; \
                                } while (0) \

/* allocate new memory with specific type and size */
#define CREATE(result, type, number)    do { \
                                            if (!((result) = (type *) calloc ((number), sizeof(type)))) { \
                                                sys_err("calloc failed [%d] %s", errno, strerror(errno)); \
                                                abort(); \
                                            } \
                                        } while (0)

/* reallocate new memory with specific type and size */
#define RECREATE(result, type, number)    do { \
                                            if (!((result) = (type *) realloc ((number), sizeof(type)))) { \
                                                sys_err("realloc failed [%d] %s", errno, strerror(errno)); \
                                                abort(); \
                                            } \
                                        } while (0)

/* Must name the char variable used in the TOKEN as "token_string" */
#define TOKEN(string)   if (!str_cmp(token_string, string))

#define core_dump() core_dump_unix(__FILE__, __LINE__)

/* generates a random integer number in given range */
#define number(from, to) number_ex(from, to, __FILE__, __LINE__)

/* generates a random float number in given range */
#define fnumber(from, to) fnumber_ex(from, to, __FILE__, __LINE__)

/* Allocate memory, copy the source, and return it */
extern char *str_dup(const char *source);

/* Print data in both hex and ASCII (used for packet analysis, etc.) */
extern void printData(const unsigned char *data, int bytes);

/* Returns given file Size */
extern int filesize(FILE *fp);

/* Parses Token, Example : Name: Osama, being able to get the value Osama, and Name Token */
extern void parse_token(char *src, char *token, char *value);

/* Dump the Core */
extern void core_dump_unix(const char *who, WORD line);

/* trim and lower string (remove the whitespace around the string) */
extern void trim_and_lower(char *src, char* dest, size_t dest_size);

/* just lower string (ex: OSAMA to osama) */
extern void lower_string(char *src, char* dest, size_t dest_size);

/***
* Time Modification Functions
*/

/* returns current time in string format */
extern char *time_str(time_t curtime);

#if defined(_WIN64)
/* a function that gets the time of given struct */
extern void gettimeofday(struct timeval* time, struct timezone *dummy);
#endif

/* calculates the differnce between two given time structs and return it */
extern struct timeval *timediff(const struct timeval *a, const struct timeval *b);

/* adds the time of b to the time of a and returns it */
extern struct timeval *timeadd(struct timeval *a, struct timeval *b);

/* returns the date that is 'days' days after the current time 'curr_tm' */
extern struct tm *tm_calculate(const struct tm* curr_tm, int days);

/* generates an unsigned random number */
extern unsigned int thecore_random();

/* generates a random number in given range */
extern int number_ex(int from, int to, const char *file, int line);

/* generates a random number in given range with float type */
extern float fnumber_ex(float from, float to, const char *file, int line);

/* converts normal C String into wstring to use in Windows */
extern std::wstring convertToWString(const char *c);

/* converts wchar String into normal string to use in Windows */
extern std::string convertWCharToChar(const std::wstring& wideStr);

/* get the time since booting the application in unsigned numbers (1000 = 1 second) */
extern uint64_t get_unsigned_time();

/* get the time since booting the application in float numbers (1.000 = 1 second) */
extern float get_float_time();

/* a function that pauses the whole core process for amount of time */
extern void thecore_sleep(struct timeval* timeout);