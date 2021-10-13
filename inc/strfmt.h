#ifndef _STRFMT_HEADER_
#define _STRFMT_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ----------------------------------------- Enums ------------------------------------------ */

// string formatting options
typedef enum{
    strfmt_none                             = 0,  

    strfmt_linebreak_bitmask                = 0x000F,
    strfmt_linebreak_no_wrap                = (1<<0),  
    strfmt_linebreak_no_wrap_dot_dot_dot    = (2<<0),              
    strfmt_linebreak_no_wrap_end_str        = (3<<0),              
    strfmt_linebreak_wrap_cut               = (4<<0),      
    strfmt_linebreak_wrap_cut_hyphen        = (5<<0),              
    strfmt_linebreak_wrap_on_word           = (6<<0),  

    strfmt_lines_bitmask                    = 0x00F0,
    strfmt_lines_scroll                     = (1<<4),
    strfmt_lines_cut                        = (2<<4),

    strfmt_align_bitmask                    = 0x0F00,
    strfmt_align_left                       = (1<<8),
    strfmt_align_right                      = (2<<8),
    strfmt_align_center                     = (3<<8) 
}strfmt_t;

/* ----------------------------------------- Auxiliary functions ---------------------------- */

// insert substring on a string at start location.
char *strins_substr(char *string, char *start, char *substr);

// concatenates and reallocate accordingly to the expected size
void strcat_realloc(char **dest, char *string);

// trim string at the end
char *strtrimend(char *string, char *pbrk);

// trim a string from the start
char *strtrimbeg(char *string, char *pbrk);

// trim a string from the start, reallocating the passed string
void strtrimbeg_realloc(char **string, char *pbrk);

// make a string repeating a char
char *makechrspan(char chr, size_t len);

// count text lines and get max width
void strlnmaxw(char *string, size_t *lines, size_t *max_w);

/* ----------------------------------------- Formatting functions --------------------------- */

// align string based on strfmt_align enum options
char *stralign(char *string, size_t minlen, strfmt_t fmt);

// format a single line, null terminated with no line breaks
char *strfmtln(char *line, size_t minlen, size_t maxlen, strfmt_t fmt, void *optional_data, char **line_break_pos);

// string formatter, non destructive, returns new memory
char *strfmtr(char *string, size_t min_lines, size_t max_lines, size_t min_line_len, size_t max_line_len, strfmt_t fmt, void *optional_data);

#endif