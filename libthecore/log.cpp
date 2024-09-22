#include "stdafx.h"

LPLOGFILE log_file_syserr = nullptr;
LPLOGFILE log_file_syslog = nullptr;
LPLOGFILE log_file_pts = nullptr;

#if defined(_WIN64)
    #define SYSERR_FILENAME "syserr.txt"
    #define SYSLOG_FILENAME "syslog.txt"
    #define PTS_FILENAME "PTS.txt"
#else
    #define SYSERR_FILENAME "syserr"
    #define SYSLOG_FILENAME "syslog"
    #define PTS_FILENAME "PTS"
#endif

static char log_dir[32] = {0, };
int log_keep_days = 3;
static unsigned int log_level_bits = 0;

/***
 * logs_init - Initialize Logs & Allocate Memory
 * @return: true on success, otherwise false.
 */
bool logs_init()
{
    log_file_set_dir("./log");

    do
    {
        log_file_syslog = log_file_init(SYSLOG_FILENAME, "a+");
        if (!log_file_syslog)
        {
            break;
        }

        log_file_syserr = log_file_init(SYSERR_FILENAME, "a+");
        if (!log_file_syserr)
        {
            break;
        }

        log_file_pts = log_file_init(PTS_FILENAME, "a+");
        if (!log_file_pts)
        {
            break;
        }

        return (true);
    } while (false);

    return (false);
}

/***
 * logs_destroy - Destroy Logs & Free Memory
 * @return: Nothing (void).
 */
void logs_destroy()
{
    log_file_destroy(log_file_syserr);
    log_file_destroy(log_file_syslog);
    log_file_destroy(log_file_pts);

    log_file_syserr = nullptr;
    log_file_syslog = nullptr;
    log_file_pts = nullptr;
}

/***
 * log_set_level - Set SysLog Level
 * @return: Nothing (void).
 */
void log_set_level(unsigned int level)
{
    log_level_bits |= level;
}

/***
 * log_unset_level - Unset SysLog Level
 * @return: Nothing (void).
 */
void log_unset_level(unsigned int level)
{
    log_level_bits &= ~level;
}

/***
 * log_file_init - Initialize a Log File & Allocate memory and return it
 * @fileName: the name of the log file.
 * @openMode: the mode which opening the file in.
 * @return: the new created & Initialized Log file.
 */
LPLOGFILE log_file_init(const char *fileName, const char *openMode)
{
    LPLOGFILE logFile = nullptr;
    FILE *fp = nullptr;;
    struct tm currTime;
    time_t time_string;

    time_string = time(0);
    currTime = *localtime(&time_string);
    
    fp = fopen(fileName, openMode);

    if (!fp)
    {
        sys_err("Failed to Open File %s", fileName);
        return (nullptr);
    }

    logFile = (LPLOGFILE) malloc(sizeof(LPLOGFILE));

    if (!logFile)
    {
        sys_err("Failed to malloc Log File %s", fileName);
        return (nullptr);
    }

    logFile->filename = strdup(fileName);
    logFile->fp = fp;
    logFile->last_hour = currTime.tm_hour;
    logFile->last_day = currTime.tm_mday;

    return (logFile);
}

void log_file_destroy(LPLOGFILE logFile)
{
    if (logFile == nullptr)
    {
        return;
    }

    if (logFile->filename)
    {
        free(logFile->filename);
        logFile->filename = nullptr;
    }

    if (logFile->fp)
    {
        fclose(logFile->fp);
        logFile->fp = nullptr;
    }

    free(logFile);
}

void log_file_check(LPLOGFILE logFile)
{
    struct stat sb;

    /*** if the file does not exist, reopen it then ***/
    if (stat(logFile->filename, &sb) != 0 && errno == ENOENT)
    {
        fclose(logFile->fp);
        logFile->fp = fopen(logFile->filename, "a+");
    }
}

void log_file_set_dir(const char *dir)
{
    strcpy(log_dir ,dir);
    log_file_delete_old(log_dir);
}

void log_file_delete_old(const char *fileName)
{
    struct stat sb;
    long num1 = 0, num2 = 0;
    char buf[32];
    char system_cmd[64]{};
    struct tm newTime;

    if (stat(fileName, &sb) == -1)
    {
        perror("log_file_delete_old: stat");
        return;
    }

    if (!S_ISDIR(sb.st_mode))
    {
        return;
    }

    newTime = *tm_calculate(nullptr, -log_keep_days);
    sprintf(buf, "%04d%02d%02d", newTime.tm_year + 1900, newTime.tm_mon + 1, newTime.tm_mday);
    num1 = atol(buf);

#if defined(_WIN64)
    WIN32_FIND_DATAW fData;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    PVOID OldValue = NULL;

    std::wstring newFileName = convertToWString(fileName);
    if(Wow64DisableWow64FsRedirection(&OldValue))
    {
        hFile = FindFirstFileW(newFileName.c_str(), &fData);
        if (FALSE == Wow64RevertWow64FsRedirection(OldValue))
        {
            //  Failure to re-enable redirection should be considered
            //  a critical failure and execution aborted.
            return;
        }
    }
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            const wchar_t* fileNameWChar = fData.cFileName;
            for (size_t i = 0; fileNameWChar[i] != L'\0'; ++i)
            {
                if (!iswdigit(fileNameWChar[i]))
                {
                    return;
                }
            }

            std::wstring newFileName = convertToWString(fileName);
            num2 = wcstol(fData.cFileName, nullptr, 10);
            if (num2 <= num1)
            {
                std::string narrowFileNameData = convertWCharToChar(fData.cFileName);
                sprintf(system_cmd, "del %s\\%s", fileName, narrowFileNameData.c_str());
                system(system_cmd);

                sys_log(0, "SYSTEM_CMD: %s", system_cmd);
            }

        } while (FindNextFileW(hFile, &fData));
    }
#else
    struct dirent** nameList;
    int n = 0;

    n = scandir(fileName, &nameList, 0, alphasort);
    if (n < 0)
    {
        perror("scandir Error");
    }
    else
    {
        char name[MAXNAMLEN + 1];
        while (n-- > 0)
        {
            strncpy(name, nameList[n]->d_name, 255);
            name[255] = '\0';

            free(nameList[n]);

            if (*name == '.')
            {
                continue;
            }

            if (!isdigit(*name))
            {
                continue;
            }

            num2 = atoi(name);
            if (num2 <= num1)
            {
                sprintf(system_cmd, "rm -rf %s/%s", fileName, name);
                system(system_cmd);

                sys_log(0, "%s: SYSTEM_CMD: %s", __FUNCTION__, system_cmd);
                fprintf(stderr, "%s: SYSTEM_CMD: %s %s: %d %s:%d\n", __FUNCTION__, system_cmd, buf, num1, name, num2);
            }
        }
    }
#endif
}

void _sys_err(const char *func, int line, const char *format, ...)
{
    va_list args;
    time_t ct = time(0);
    std::tm* localTime = localtime(&ct);
    char* time_string = nullptr;

    if (localTime != nullptr)
    {
        time_string = asctime(localTime);
    }

    if (time_string == nullptr)
    {
        return;
    }
    char buf[4096 + 2]; /*** to add \n at the end ***/
    int len;

    /*** make sure the system error log file is initialized ***/
    if (!log_file_syserr)
    {
        return;
    }

    /*** set last character in time string as null terminator ***/
    time_string[strlen(time_string) - 1] = '\0';

    /*** calculate the len of the buffer ***/
    len = snprintf(buf, 4096, "SYSERR: %-15.15s :: %s: ", time_string + 4, func);

    /*** set last character in the buffer string as null terminator ***/
    buf[4096 + 1] = '\0';

    /*** if there is a space in the file length */
    if (len < 4096)
    {
        va_start(args, format);
        vsnprintf(buf + len, 4096 - len, format, args);
        va_end(args);
    }

    /*** Add newline to the end of the string ***/
    strcat(buf, "\n");

    /*** print the output into log_file_syserr ***/
    fputs(buf, log_file_syserr->fp);
    fflush(log_file_syserr->fp);

    /*** print the output into log_file_syslog, since it contains both logs and errors of our application, but make sure it's initialized ***/
    if (log_file_syslog)
    {
        fputs(buf, log_file_syslog->fp);
        fflush(log_file_syslog->fp);
    }

#if defined(_WIN64)
    fputs(buf, stdout);
    fflush(stdout);
#endif
}

long lastSysLog = 0;
void sys_log(unsigned int level, const char *format, ...)
{
    va_list args;

    struct timeval timeVal;
    int iMilliSec = 0;
    gettimeofday(&timeVal, nullptr);

    if (level != 0 && !(log_level_bits & level))
    {
        return;
    }

    if (log_file_syslog)
    {
        time_t curTime = time(0);
        std::tm* localTime = localtime(&curTime);
        char* time_string = nullptr;

        if (localTime != nullptr)
        {
            time_string = asctime(localTime);
        }

        if (time_string == nullptr)
        {
            return;
        }

        time_string[strlen(time_string) - 1] = '\0';

        long calcTime;
        if (timeVal.tv_usec > lastSysLog)
        {
            calcTime = timeVal.tv_usec - lastSysLog;
        }
        else
        {
            calcTime = 1000000 - lastSysLog + timeVal.tv_usec;
        }

        fprintf(log_file_syslog->fp, "%-15.15s.%d [%d] :: ", time_string + 4, timeVal.tv_usec, calcTime);
        lastSysLog = timeVal.tv_usec;

        va_start(args, format);
        vfprintf(log_file_syslog->fp, format, args);
        va_end(args);

        fputc('\n', log_file_syslog->fp);
        fflush(log_file_syslog->fp);
    }

#if !defined(_WIN64)
    // If log_level is 1 or higher, it is often a test, so it is also printed to stdout.
    if (log_level_bits > 1)
    {
#endif

        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);

        fputc('\n', stdout);
        fflush(stdout);
#if !defined(_WIN64)
    }
#endif
}

void pts_log(const char *format, ...)
{
    va_list args;

    if (!log_file_pts)
    {
        return;
    }

    va_start(args, format);
    vfprintf(log_file_pts->fp, format, args);
    va_end(args);

    fputc('\n', log_file_pts->fp);
    fflush(log_file_pts->fp);
}