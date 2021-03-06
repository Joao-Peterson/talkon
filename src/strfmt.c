#include "strfmt.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* ----------------------------------------- Auxiliary functions ---------------------------- */

// insert substring on a string at start location.
char *strins_substr(char *string, char *start, char *substr){
    if(string == NULL || (start == NULL || start < string) || substr == NULL)
        return NULL;

    size_t newstr_len = strlen(string) + strlen(substr) + 1;
    char *newstr = (char*)calloc(newstr_len, sizeof(char));


    if(start == string){
        snprintf(newstr, newstr_len, "%s%s", substr, string);
    }
    else if(*start == '\0'){
        snprintf(newstr, newstr_len, "%s%s", string, substr);
    }
    else{
        char intersection = *start;
        *start = '\0';
        start++;

        snprintf(newstr, newstr_len, "%s%c%s%s", string, intersection, substr, start);

        start--;
        *start = intersection;
    }

    return newstr;
}

// concatenates and reallocate accordingly to the expected size
void strcat_realloc(char **dest, char *string){
    if(dest == NULL || *dest == NULL || string == NULL)
        return;
    
    size_t newlen = strlen(*dest) + strlen(string);

    char *newstr = (char*)calloc(newlen + 1, sizeof(char));

    strcat(newstr, *dest);
    strcat(newstr, string);

    free(*dest);
    *dest = newstr;
}

// trim string at the end
char *strtrimend(char *string, char *pbrk){
    if(string == NULL || (pbrk == NULL || pbrk < string) || *pbrk == '\0' || pbrk == string)
        return NULL;

    char *string_copy = (char*)calloc(strlen(string) + 1, sizeof(char));
    strcpy(string_copy, string); 
    pbrk = (char*)(pbrk - string + string_copy);

    size_t newstr_len = strlen(string_copy) - strlen(pbrk);
    char *newstr = (char*)calloc(newstr_len + 1, sizeof(char));

    char c = *pbrk; 
    *pbrk = '\0';
    strncpy(newstr, string_copy, newstr_len);
    *pbrk = c;

    free(string_copy);
    return newstr;
}

// trim a string from the start
char *strtrimbeg(char *string, char *pbrk){
    if(string == NULL || (pbrk == NULL || pbrk < string))
        return NULL;

    if(*pbrk == '\0'){
        char *newstr = (char*)calloc(1, sizeof(char));
        return newstr;
    }
    else if(pbrk == string){
        char *newstr = (char*)calloc(strlen(string) + 1, sizeof(char));
        strcpy(newstr, string);
        return newstr;
    }
    else{
        size_t newstr_len = strlen(string) - (size_t)(pbrk - string);
        char *newstr = (char*)calloc(newstr_len + 1, sizeof(char));

        strncpy(newstr, pbrk, newstr_len);

        return newstr;
    }
}

// trim a string from the start, reallocating the passed string
void strtrimbeg_realloc(char **string, char *pbrk){
    if(*string == NULL || (pbrk == NULL || pbrk < (*string)))
        return;
        
    char *newstr = strtrimbeg(*string, pbrk);
    free(*string);
    *string = newstr;
}

// make a string repeating a char
char *makechrspan(char chr, size_t len){
    char *span = (char*)calloc(len + 1, sizeof(char));

    for(int i = 0; i < len; i++)
        span[i] = chr;

    span[len] = '\0'; 

    return span;
}

// count text lines and get max width
void strlnmaxw(char *string, size_t *lines, size_t *max_w){
    if(string == NULL || lines == NULL || max_w == NULL)
        return;

    *lines = 1;
    *max_w = 0;
    int last_max_w = 0;
    size_t i = 0;
    
    while(string[i] != '\0'){
        (*max_w)++;
        
        if(string[i] == '\n'){
            (*max_w)--;
            if(*max_w > last_max_w){
                last_max_w = *max_w;
            }
            *max_w = 0;
                
            (*lines)++;
        }

        i++;
    }

    // when string is a single line the last_max_w is zero
    if(last_max_w != 0)
        *max_w = last_max_w;
}

// find char in string by running backwards, thus finding the last occurance of that char
char *strchr_back(char *string, char chr){
    if(string == NULL)
        return NULL;
    
    char *end = strchr(string, '\0');

    for(char *cursor = end; cursor != string; cursor--){
        if(*cursor == chr)
            return cursor;
    }

    return NULL;
}

/* ----------------------------------------- Formatting functions --------------------------- */

// align string based on strfmt_align enum options
char *stralign(char *string, size_t maxlen, strfmt_t fmt){

    size_t string_len = strlen(string);

    switch(fmt & strfmt_align_bitmask){
        case strfmt_align_center:
            {
                size_t left_padding = (maxlen - string_len) / 2;
                size_t right_padding  = maxlen - string_len - left_padding;

                char *spaces_left  = makechrspan(' ', left_padding);
                char *spaces_right = makechrspan(' ', right_padding);
                
                char *padleft = strins_substr(string, string, spaces_left);
                char *aligned = strins_substr(padleft, strchr(padleft, '\0'), spaces_right);

                free(padleft);
                free(spaces_left);
                free(spaces_right);

                return aligned;
            }
            break;

        case strfmt_align_right:
            {
                char *spaces = makechrspan(' ', maxlen - string_len);
                char *aligned = strins_substr(string, string, spaces);
                
                free(spaces);

                return aligned;
            }
            break;
            
        default:
        case strfmt_align_left:
            {
                char *spaces = makechrspan(' ', maxlen - string_len);
                char *aligned = strins_substr(string, strchr(string, '\0'), spaces);

                free(spaces);

                return aligned;
            }
            break;
    }
}

// format a single line, null terminated with no line breaks
char *strfmtln(char *line, size_t minlen, size_t maxlen, strfmt_t fmt, void *optional_data){
    if(line == NULL || minlen > maxlen)
        return NULL;

    size_t line_len = strlen(line);

    // stuff with whitespace if smaller than minlen
    if(line_len < minlen){
        char *aligned = stralign(line, maxlen, fmt);
        return aligned;
    }
    // add line break if string is longer than maxlen
    else if(line_len > maxlen){
        // default align
        if((fmt & strfmt_linebreak_bitmask) == 0)
            fmt |= strfmt_linebreak_wrap;

        switch(fmt & strfmt_linebreak_bitmask){
            case strfmt_linebreak_no_wrap:
                {
                    char *trimmed = strtrimend(line, (char*)(line + maxlen));
                    return trimmed;
                }
                break;

            case strfmt_linebreak_no_wrap_dot_dot_dot:
                {
                    char *trimmed = strtrimend(line, (char*)(line + maxlen - 3));
                    strcat_realloc(&trimmed, "...");
                    return trimmed;
                }
                break;

            case strfmt_linebreak_no_wrap_end_str:
                {
                    char *trimmed;
                    char *end = (char*)optional_data;
                    if(
                        optional_data != NULL && 
                        (end)[0] != '\0' &&
                        strlen(end) <= maxlen
                    ){
                        trimmed = strtrimend(line, (char*)(line + maxlen - strlen(end)));
                        strcat_realloc(&trimmed, end);
                    }
                    else{
                        trimmed = strtrimend(line, (char*)(line + maxlen));
                    }
                    
                    return trimmed;
                }
                break;
            
            case strfmt_linebreak_wrap:
            case strfmt_linebreak_wrap_hyphen:
                {
                    char *line_trimmed;
                    char *line_break_pos;
                    
                    if((fmt & strfmt_linebreak_bitmask) == strfmt_linebreak_wrap_hyphen){
                        line_trimmed = strtrimend(line, (char*)(line + maxlen - 1));
                        line_break_pos = line + maxlen - 1;
                        strcat_realloc(&line_trimmed, "-");
                    }
                    else{
                        line_trimmed = strtrimend(line, (char*)(line + maxlen));
                        line_break_pos = line + maxlen;
                    }

                    strcat_realloc(&line_trimmed, "\n");

                    char *end = strfmtln(line_break_pos, minlen, maxlen, fmt, optional_data);

                    strcat_realloc(&line_trimmed, end);

                    free(end);
                    return line_trimmed;
                }
                break;

            case strfmt_linebreak_wrap_on_word:
                {
                    char *line_brk = (char*)(line + maxlen);

                    // go back until whitespace
                    for(line_brk = line_brk; line_brk > line; line_brk--){
                        if(*line_brk == ' ')
                            break;
                    }

                    // can't linebreak on word because there is only one word
                    if(line_brk == line){
                        line_brk = (char*)(line + maxlen - 1);
                        char *line_trimmed = strtrimend(line, line_brk);
                        char *end = strfmtln(line_brk + 1, minlen, maxlen, fmt, optional_data);

                        strcat_realloc(&line_trimmed, "-");
                        strcat_realloc(&line_trimmed, "\n");
                        strcat_realloc(&line_trimmed, end);

                        free(end);
                        return line_trimmed;
                    }
                    else{
                        char *line_trimmed = strtrimend(line, line_brk);
                        char *end = strfmtln(line_brk + 1, minlen, maxlen, fmt, optional_data);

                        strcat_realloc(&line_trimmed, "\n");
                        strcat_realloc(&line_trimmed, end);

                        free(end);
                        return line_trimmed;
                    }
                }
                break;
        }
    }
    // if string size is equal to minlen and maxlen, just return a copy
    else{
        char *newstr = (char*)calloc(strlen(line) + 1, sizeof(char));
        strcpy(newstr, line);
        return newstr;
    }

    return NULL;
}

// string formatter, non destructive, returns new memory
char *strfmtr(char *string, size_t min_lines, size_t max_lines, size_t min_line_len, size_t max_line_len, strfmt_t fmt, void *optional_data){
    if(string == NULL)
        return NULL;

    if(min_line_len <= 0 || min_lines <=0 || max_lines < min_lines || max_line_len < min_line_len)
        return NULL;

    // copy the passed string    
    char *base_string = (char*)calloc(1, strlen(string) + 1);
    if(base_string == NULL)
        return NULL;

    strcpy(base_string, string);

    // string to be formatted and returned 
    char *str_fmtd = (char*)calloc(1, sizeof(char));
    size_t lines = 0;

    // loop throught lines, formatting each one
    char *line = base_string;
    while(1){

        // look for linebreak or string end
        char *linebreak = strchr(line, '\n');
        bool end_fmt = false;
        if(linebreak != NULL){
            // null terminate line to be passed to the line formatter call
            *linebreak = '\0';
        }
        else{
            linebreak = strchr(line, '\0');
            end_fmt = true;
        }

        // format line
        char *line_fmtd = strfmtln(line, min_line_len, max_line_len, fmt, optional_data); 

        // if not on first line, add linebreak
        if(lines != 0)
            strcat_realloc(&str_fmtd, "\n");

        // concat the formatted line
        strcat_realloc(&str_fmtd, line_fmtd);
        free(line_fmtd);

        lines++;
        line = linebreak + 1;
        
        if(end_fmt)
            break;
    }

    // format scroll options 
    char *str_fmtd_scroll = (char*)calloc(1, sizeof(char));
    lines = 0;

    // default
    if((fmt & strfmt_lines_bitmask) == 0)
        fmt |= strfmt_lines_no_scroll;

    char *str_scroll = (char*)calloc(1, sizeof(char));

    switch(fmt & strfmt_lines_bitmask){
        case strfmt_lines_no_scroll:
            {
                char *line_break;
                char *line = str_fmtd;
                bool end_scroll = false;
                // loop through lines concat them to the output string
                while(1){

                    line_break = strchr(line, '\n');
                    if(line_break == NULL){
                        line_break = strchr(line, '\0');
                        end_scroll = true;
                    }
                    else{
                        *line_break = '\0';
                    }

                    if(lines != 0)
                        strcat_realloc(&str_scroll, "\n");

                    strcat_realloc(&str_scroll, line);
                    line = line_break + 1;
                    lines++;

                    if((lines >= max_lines) || end_scroll){
                        break;
                    }
                }
            }
            break;

        case strfmt_lines_scroll:
            {
                char *line_break;
                char *str_fmtd_copy = (char*)calloc(strlen(str_fmtd) + 1, sizeof(char));
                strcpy(str_fmtd_copy, str_fmtd);
                char *line = str_fmtd;
                // loop through lines concat them to the output string
                while(1){

                    line = strchr_back(str_fmtd, '\n');
                    if(line == NULL){
                        line = str_fmtd;
                        break;
                    }

                    lines++;

                    *line = '\0';
            
                    if(lines >= max_lines){
                        // line is on top of a \n, move to right by one
                        line++; 
                        break;
                    }
                }

                strcat_realloc(&str_scroll, str_fmtd_copy + (line - str_fmtd));
                free(str_fmtd_copy);
            }
            break;
    }

    free(base_string);
    free(str_fmtd);
    return str_scroll;
}
