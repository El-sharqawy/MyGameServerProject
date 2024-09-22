#pragma once

/*** Logs struct data ***/
typedef struct SLogFile
{
    char *filename;
    FILE* fp;
    int last_hour;
    int last_day;
} TLogFile;

/*** a pointer to the struct logs ***/
typedef TLogFile *LPLOGFILE;

/*** a variable of the struct logs ***/
typedef TLogFile LOGFILE;

/*** Initialize Logs & Allocate Memory ***/
extern bool logs_init();

/*** Destroy Logs & Free Memory ***/
extern void logs_destroy();

/*** Rotate Syserr log & check all logs ***/
extern void logs_rotate();

/*** Set SysLog Level ***/
extern void log_set_level(unsigned int level);

/*** Unset SysLog Level ***/
extern void log_unset_level(unsigned int level);

/*** Initialize a Log File & Allocate memory and return it ***/
LPLOGFILE log_file_init(const std::string& fileName, const std::string& openMode);

/*** Destroy a Log File & Free memory ***/
void log_file_destroy(LPLOGFILE logFile);

/*** Check Log File ***/
void log_file_check(LPLOGFILE logFile);

/*** Set Log File Directory ***/
void log_file_set_dir(const std::string& dir);

/*** Delete Old Log Files ***/
void log_file_delete_old(const std::string& fileName);

/*** Rotate & Check log file and move/create and modify it ***/
void log_file_rotate(LPLOGFILE logFile);

/*** Print to system Error Output Function ***/
extern void _sys_err(const char* func, int line, const char* format, ...);

/*** Print to system Logs Output Function ***/
extern void sys_log(unsigned int level, const char* format, ...);

/*** Print to system Pts Output Function ***/
extern void pts_log(const char* format, ...);

#if defined(_WIN64)
    #define sys_err(fmt, ...) _sys_err(__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define sys_err(fmt, args...) _sys_err(__FUNCTION__, __LINE__, fmt, ##args)
#endif	// _WIN64
