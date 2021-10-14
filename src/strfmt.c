#include "../inc/strfmt.h"

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
    else if(pbrk = string){
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
                *max_w = 0;
            }
                
            (*lines)++;
        }

        i++;
    }

    // when string is a single line the last_max_w is zero
    if(last_max_w != 0)
        *max_w = last_max_w;
}

/* ----------------------------------------- Formatting functions --------------------------- */

// align string based on strfmt_align enum options
char *stralign(char *string, size_t minlen, strfmt_t fmt){

    size_t string_len = strlen(string);

    if(string_len > minlen)
        minlen = string_len;

    switch(fmt & strfmt_align_bitmask){
        case strfmt_align_center:
            {
                size_t left_padding = (minlen - string_len) / 2;
                size_t right_padding  = minlen - string_len - left_padding;

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
                char *spaces = makechrspan(' ', minlen - string_len);
                char *aligned = strins_substr(string, string, spaces);
                
                free(spaces);

                return aligned;
            }
            break;
            
        default:
        case strfmt_align_left:
            {
                char *spaces = makechrspan(' ', minlen - string_len);
                char *aligned = strins_substr(string, strchr(string, '\0'), spaces);

                free(spaces);

                return aligned;
            }
            break;
    }
}

// format a single line, null terminated with no line breaks
char *strfmtln(char *line, size_t minlen, size_t maxlen, strfmt_t fmt, void *optional_data, char **line_break_pos){
    if(line == NULL || minlen > maxlen)
        return NULL;

    size_t line_len = strlen(line);
    *line_break_pos = NULL;

    // stuff with whitespace
    if(line_len < minlen){
        char *aligned = stralign(line, minlen, fmt);
        return aligned;
    }
    // add line break
    else if(line_len > maxlen){
        switch(fmt & strfmt_linebreak_bitmask){
            case strfmt_linebreak_no_wrap:
            case strfmt_linebreak_wrap_cut:
                {
                    char *trimmed = strtrimend(line, (char*)(line + maxlen));
                    return trimmed;
                }
                break;

            case strfmt_linebreak_no_wrap_dot_dot_dot:
            case strfmt_linebreak_wrap_cut_hyphen:
            case strfmt_linebreak_no_wrap_end_str:
                {
                    char *end;
                    size_t end_len;

                    switch(fmt & strfmt_linebreak_bitmask){
                        case strfmt_linebreak_no_wrap_dot_dot_dot:
                            end = "...";
                            break;
                            
                        case strfmt_linebreak_wrap_cut_hyphen:
                            end = "-";
                            break;

                        case strfmt_linebreak_no_wrap_end_str:

                            if(optional_data != NULL)
                                end = (char*)optional_data;
                            else
                                end = "...";
                                
                            break;
                    }

                    end_len = strlen(end);
                    
                    char *trimmed = strtrimend(line, (char*)(line + maxlen - end_len));

                    char *cutted = (char*)calloc(maxlen + 1, sizeof(char));

                    snprintf(cutted, maxlen + 1, "%s%s", trimmed, end);

                    free(trimmed);
                    return cutted;
                }
                break;

            case strfmt_linebreak_wrap_on_word:
                {
                    char *line_brk = (char*)(line + line_len);

                    // go back until whitespace
                    if(*line_brk != ' '){
                        for(line_brk = line_brk; line_brk > line; line_brk--){
                            if(*line_brk == ' ')
                                break;
                        }
                    }

                    // can't linebreak on word because there is only one word
                    if(line_brk == line)
                        return strfmtln(line, minlen, maxlen, (fmt & ~(strfmt_linebreak_wrap_on_word)) | strfmt_linebreak_wrap_cut_hyphen, optional_data, line_break_pos);

                    *line_brk = '\0';
                    // points to breaked line
                    line_brk++;

                    if(line_break_pos != NULL)
                        *line_break_pos = line_brk;

                    char *newline_1 = stralign(line, minlen, fmt);
                    size_t newline_len = strlen(newline_1) + strlen(line_brk);
                    char *newline = (char*)calloc(newline_len + 1, sizeof(char));

                    snprintf(newline, newline_len, "%s\n%s", newline_1, line_brk);

                    line_brk--;
                    *line_brk = ' ';

                    free(newline_1);

                    return newline;
                }
                break;
        }
    }
    else{
        char *newstr = (char*)calloc(strlen(line) + 1, sizeof(char));
        strcpy(newstr, line);
        return newstr;
    }
}

// string formatter, non destructive, returns new memory
char *strfmtr(char *string, size_t min_lines, size_t max_lines, size_t min_line_len, size_t max_line_len, strfmt_t fmt, void *optional_data){
    if(string == NULL)
        return NULL;

    if(min_line_len <= 0 || min_lines <=0 || max_lines < min_lines || max_line_len < min_line_len)
        return NULL;
    
    char *base_string = calloc(1, strlen(string) + 1);
    if(base_string == NULL)
        return NULL;

    strcpy(base_string, string);

    char *str_fmtd = (char*)calloc(1, sizeof(char));
    size_t lines = 0;

    // loop throught lines
    for(char *line = base_string; *line != '\0'; line++){

        char *linebreak = strchr(line, '\n');
        char c;
        if(linebreak != NULL){
            c = *linebreak;
            *linebreak = '\0';
        }
        else{
            linebreak = strchr(line, '\0');
            c = *linebreak;
        }

        char *line_break_pos;

        //fmt line
        char *line_fmtd = strfmtln(line, min_line_len, max_line_len, fmt, optional_data, &line_break_pos); 

        *linebreak = c;

        // if not on first line
        if(lines != 0)
            strcat_realloc(&str_fmtd, "\n");

        // check if string was line breaked
        if(line_break_pos != NULL){
            char *linebreak_on_fmtd = strchr(line_fmtd, '\n');
            linebreak_on_fmtd = '\0';
            strcat_realloc(&str_fmtd, line_fmtd);
        }
        else{
            strcat_realloc(&str_fmtd, line_fmtd);
        }


        free(line_fmtd);
        lines++;

        // TODO: condition on wrap options 

        // check last line
        if(lines == max_lines && *linebreak == '\0'){
            break;
        }
        // check last line before finishing
        else if(*linebreak == '\0' && lines < min_lines){
            for(int i = lines; i < max_lines; i++)
                strcat_realloc(&str_fmtd, "\n");

            break;
        }
        // check overflow lines
        else if(lines >= max_lines){
            strfmt_t lines_fmt = fmt & strfmt_lines_bitmask; 

            // just cut 
            if(lines_fmt == strfmt_lines_cut || lines_fmt == 0){
                break;
            }
            // scroll string down, removing from the beggining
            else if(lines_fmt == strfmt_lines_scroll){
                char *pbrk = strchr(str_fmtd, '\n');

                if(pbrk == NULL)
                    pbrk = strchr(str_fmtd, '\0');
                else
                    pbrk++;
                
                strtrimbeg_realloc(&str_fmtd, pbrk);
                lines--;
            }
        }


        if(line_break_pos == NULL)
            line = linebreak;
        else
            line = (char*)(line_break_pos - 1);
    }

    free(base_string);
    return str_fmtd;
}
