#ifndef _LOG_HEADER_
#define _LOG_HEADER_

#include <stdio.h>
#include <stdarg.h>

/* ----------------------------------------- Defines ------------------------------------------ */

#define LOG_FILE_LINE_STR   "[%s.%i] "

/* ----------------------------------------- Prototypes --------------------------------------- */

void logprintf(const char *format, ...);

void log_set_output_file(char *filename, char *restric_mode);

/* ----------------------------------------- Macros ------------------------------------------- */

#define log(format_string, ...)         logprintf(LOG_FILE_LINE_STR format_string,            __FILE__, __LINE__, ##__VA_ARGS__)

#define log_debug(format_string, ...)   logprintf(LOG_FILE_LINE_STR "[DEBUG] " format_string, __FILE__, __LINE__, ##__VA_ARGS__)

#define log_error(format_string, ...)   logprintf(LOG_FILE_LINE_STR "[ERROR] " format_string, __FILE__, __LINE__, ##__VA_ARGS__)

#endif