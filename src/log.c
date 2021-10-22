#include "log.h"

/* ----------------------------------------- Functions ---------------------------------------- */

void log_set_output_file(char *filename, char *restric_mode){
    FILE *stdout_save = stdout;
    
    stdout = freopen(filename, restric_mode, stdout);

    if(stdout == NULL)
        stdout = stdout_save;
}

void logprintf(FILE *out, const char *format, ...){
    if(out == NULL)
        return;

    va_list arg_list;

    va_start(arg_list, format);
    
    vfprintf(out, format, arg_list);
    
    va_end(arg_list);
}
