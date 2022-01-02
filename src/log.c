#include "log.h"
#include <stdio.h>

/* ----------------------------------------- Functions ---------------------------------------- */

void log_set_output_file(char *filename, char *restric_mode){
    freopen(filename, "wb", stderr);
}

void logprintf(const char *format, ...){
    va_list arg_list;

    va_start(arg_list, format);
    
    vfprintf(stderr, format, arg_list);
    fflush(stderr);
    
    va_end(arg_list);
}
