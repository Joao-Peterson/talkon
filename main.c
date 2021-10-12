#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// #include <netdb.h>

#include "inc/doc.h"
#include "inc/doc_json.h"


#if defined(__linux__) || defined(__unix__) || defined(__posix__)
    #include <curses.h>
    #include <panel.h>
#elif defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__NT__)
    #include <pdcurses.h>
#elif defined(__ANDROID__)
    #error android is not supported!
#elif defined(__APPLE__) || defined(__MACH__)
    #error macOS is not supported!
#elif
    #error your OS is not supported!
#endif

// macros for drawing borders
#define medium_border   ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD 
#define light_border    ACS_VLINE, ACS_VLINE, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_VLINE, ACS_VLINE 
#define bold_border     ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK 

// struct for storing window pos and size
typedef struct{
    int h;
    int w;
    int y;
    int x;
}win_size_t;

// struct for storing a frame charset, all sides and corners
typedef struct{
    chtype ls;
    chtype rs;
    chtype ts;
    chtype bs;
    chtype tl;
    chtype tr;
    chtype bl;
    chtype br;
}frame_charset_t;

// type of word break
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

// set the size of a window
void wsetsize(WINDOW *window, win_size_t winsize){
    mvwin(window, winsize.y, winsize.x);
    wresize(window, winsize.h, winsize.w);
}

// draw a rectangle with a frame
void wdraw_rect(WINDOW *win, int h, int w, int y, int x, frame_charset_t frame, chtype fill){
    for(int i = x; i < x+w; i++){
        for(int j = y; j < y+h; j++){

            if(i == x && j == y){
                mvwaddch(win, j, i, frame.tl);
            }
            else if(i == (x+w-1) && j == y){
                mvwaddch(win, j, i, frame.tr);
            }
            else if(i == x && j == (y+h-1)){
                mvwaddch(win, j, i, frame.bl);
            }
            else if(i == (x+w-1) && j == (y+h-1)){
                mvwaddch(win, j, i, frame.br);
            }
            else if(i == x){
                mvwaddch(win, j, i, frame.ls);
            }
            else if(i == (x+w-1)){
                mvwaddch(win, j, i, frame.rs);
            }
            else if(j == y){
                mvwaddch(win, j, i, frame.ts);
            }
            else if(j == (y+h-1)){
                mvwaddch(win, j, i, frame.bs);
            }
            else{
                mvwaddch(win, j, i, fill);
            }
        }
    }
}

// draw a border with a frame
void wborder_frame(WINDOW *win, frame_charset_t frame){
    wborder(win, 
        frame.ls,
        frame.rs,
        frame.ts,
        frame.bs,
        frame.tl,
        frame.tr,
        frame.bl,
        frame.br
    );
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

// run all whitespace chars
void run_whitespace(char **stream){
    while( ((**stream >= '\b' && **stream <= '\r') || **stream == 32) )
        (*stream)++;
}

// run whitespace backwards
void run_whitespace_back(char **stream){
    while( ((**stream >= '\b' && **stream <= '\r') || **stream == 32) )
        (*stream)--;
}

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

// better mvwprintw
void mvwprintwln(WINDOW *win, int y, int x, char *string){
    if(win == NULL || string == NULL)
        return;
    
    char *base_string = calloc(1, strlen(string) + 1);
    if(base_string == NULL)
        return;

    strcpy(base_string, string);

    char *cur_string = base_string;
    int cur_line = 0;

    for(char *cursor = base_string; true; cursor++){
        if(*cursor == '\n'){
            *cursor = '\0';

            mvwprintw(win, y + cur_line, x, cur_string);

            cursor++;
            // run_whitespace(&cursor);
            cur_string = cursor;

            // so the next cursor++ on the for loop goes on top of the next letter
            cursor--;
            cur_line++;
        }
        else if(*cursor == '\0'){
            mvwprintw(win, y + cur_line, x, cur_string);
            break;
        }
    }

    free(base_string);
}

// draw text with a frame
void wdraw_label(WINDOW *win, char *text, int y, int x, int minh, int maxh, 
     int minw, int maxw, strfmt_t fmt, frame_charset_t frame, chtype fill, void *optional_data){

    size_t lines, max_w;

    char *text_fmt = strfmtr(text, minh - 2, maxh - 2, minw - 2, maxw - 2, fmt, optional_data);

    strlnmaxw(text_fmt, &lines, &max_w);

    wdraw_rect(win, lines + 2, max_w + 2, y, x, frame, fill);
    
    mvwprintwln(win, y + 1, x + 1, text_fmt);

    free(text_fmt);
}



int main(int argc, char **argv){

    initscr();
    // cbreak();
    timeout(-1);
    noecho();
    clear();
    curs_set(0);

    // frames
    frame_charset_t frame_normal = { ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER };
    frame_charset_t frame_dotted = { '|', '|', '-', '-', 126 | A_ALTCHARSET, 126 | A_ALTCHARSET, 126 | A_ALTCHARSET, 126 | A_ALTCHARSET };
    frame_charset_t frame_square_brackets = { '[', ']', ' ', ' ', ' ', ' ', ' ', ' ' };
    frame_charset_t frame_double = { '#', '#', '=', '=', '=', '=', '=', '=' };
    frame_charset_t frame_hash = { '#', '#', '#', '#', '#', '#', '#', '#' };
    frame_charset_t frame_grain_thin = { ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD };
    frame_charset_t frame_grain_thick = { ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD };
    frame_charset_t frame_solid = { ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK };

    // setting up windows
    WINDOW *main_win = newwin(0, 0, 0, 0);
    PANEL  *main_pan = new_panel(main_win);

    WINDOW *nav_win = newwin(0, 0, 0, 0);
    PANEL  *nav_pan = new_panel(nav_win);

    WINDOW *nodes_win = newwin(0, 0, 0, 0);
    PANEL  *nodes_pan = new_panel(nodes_win);

    WINDOW *talk_win = newwin(0, 0, 0, 0);
    PANEL  *talk_pan = new_panel(talk_win);

    WINDOW *input_win = newwin(0, 0, 0, 0);
    PANEL  *input_pan = new_panel(input_win);

    refresh();

    // main loop
    bool first_loop = true;
    while(1){
        
        // ----------------------- draw ------------------------
        int height, width;
        getmaxyx(stdscr, height, width);
        clear();

        // main window
        wclear(main_win);
        wresize(main_win, height, width);
        wborder_frame(main_win, frame_normal);
        mvwprintw(main_win, 0, 3, "[Talkon - TCP LAN Messenger v1.0]");

        // navbar / menu
        wclear(nav_win);
        win_size_t nav_win_size = {
            .h = 3, 
            .w = width - 2, 
            .y = 1, 
            .x = 1
        };
                
        wsetsize(nav_win, nav_win_size);
        wborder_frame(nav_win, frame_normal);
        wdraw_rect(nav_win, 1, nav_win_size.w - 2, 1, 1, frame_grain_thin, ' ');
        mvwprintw(nav_win, 1, 3, "Configuration");

        // nodes / network
        wclear(nodes_win);
        win_size_t nodes_win_size = {
            .h = height - 5, 
            .w = width * 0.4, 
            .y = 4, 
            .x = 1
        };

        wsetsize(nodes_win, nodes_win_size);
        wborder_frame(nodes_win, frame_normal);
        mvwprintw(nodes_win, 0, 3, " Network ");
        // wdraw_rect(nodes_win, 7, nodes_win_size.w - 2, 1, 1, frame_dotted, ' ');
        wdraw_label(
            nodes_win, "#1234\njoao-peterson\n192.168.1.255:4092", 
            1, 1, 5, 5, 28, 28,
            strfmt_align_center | strfmt_lines_scroll |strfmt_linebreak_no_wrap_dot_dot_dot,
            frame_dotted, ' ',
            NULL
        );

        // char *aligned = strfmtr("#1234\njoao-peterson\n192.168.1.255:4092", 2, 2, 19, 19, strfmt_align_center | strfmt_lines_scroll |strfmt_linebreak_no_wrap_dot_dot_dot, NULL);
        // mvwprintwln(nodes_win, 1, 1, aligned);

        // input
        wclear(input_win);
        win_size_t input_win_size = {
            .h = 7, 
            .w = width - 2 - nodes_win_size.w, 
            .y = height - 7 - 1, 
            .x = nodes_win_size.w + 1
        };

        wsetsize(input_win, input_win_size);
        wborder_frame(input_win, frame_normal);
        mvwprintw(input_win, 0, 3, " Input ");
        

        // talk / chat
        wclear(talk_win);
        win_size_t talk_win_size = {
            .h = height - 5 - input_win_size.h, 
            .w = input_win_size.w, 
            .y = 4, 
            .x = nodes_win_size.w + 1
        };

        wsetsize(talk_win, talk_win_size);
        wborder_frame(talk_win, frame_normal);
        mvwprintw(talk_win, 0, 3, " Chat ");

        // render
        // wrefresh(nav_win);
        // wrefresh(main_win);
        // wrefresh(nodes_win);
        // wrefresh(talk_win);
        // wrefresh(input_win);
        update_panels();
        refresh();

        // ----------------------- input -----------------------
        if(!first_loop){                                                            // only after first render round
            char input = getch();

            switch(input){
                case 'q':
                    endwin();
                    exit(0);
                    break;

                default:
                    break;
            }

        }

        first_loop = false;
    }

    endwin();

    // int sd = socket(AF_INET, SOCK_STREAM, 0);

    // struct sockaddr_in server_addr;
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(80);

    // inet_pton(AF_INET, "185.199.108.153", &server_addr.sin_addr);

    // // local host
    // // server_addr.sin_addr.s_addr = INADDR_ANY;

    // if(connect(sd, (struct sockaddr*)(&server_addr), sizeof(server_addr))){
    //     printf("Error connecting to server\n");
    //     exit(-1);
    // }

    // char buffer[5000];
    // snprintf(buffer, 5000, "GET / HTTP/1.1\r\n\r\n");

    // if(send(sd, buffer, strlen(buffer), 0) < 0){
    //     printf("Error sending GET\n");
    // }

    // int size = recv(sd, buffer, 5000ULL, 0);

    // printf("[%i]\n%s", size, buffer);

    // shutdown(sd, SHUT_RDWR);

    return 0;
}