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
    strfmt_linebreak_wrap_cut               = (3<<0),      
    strfmt_linebreak_wrap_cut_hyphen        = (4<<0),              
    strfmt_linebreak_wrap_on_word           = (5<<0),  

    strfmt_lines_bitmask                    = 0x00F0,
    strfmt_lines_scroll_up                  = (1<<4),
    strfmt_lines_scroll_down                = (2<<4),
    strfmt_lines_scroll_cut                 = (3<<4),
    strfmt_lines_scroll_cut_dot_dot_dot     = (4<<4),

    strfmt_align_bitmask                    = 0x0F00,
    strfmt_align_left                       = (1<<8),
    strfmt_align_right                      = (2<<8),
    strfmt_align_center                     = (3<<8) 
}strfmt_enum;

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
void strlnmaxw(char *string, int *lines, int *max_w){
    if(string == NULL || lines == NULL || max_w == NULL)
        return;

    *lines = 1;
    *max_w = 0;
    int last_max_w = 0;
    size_t i = 0;
    
    while(string[i] != '\0'){
        (*max_w)++;
        
        if(string[i] == '\n'){
            if(*max_w > last_max_w)
                last_max_w = *max_w;
                
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

// insert substring on a string at start location. The base string will be reallocated if necessary, thus we pass a double pointer.
// The return value points to the string right after the inserted substring.
char *strins_substr(char **string_base_address, char *start, char *substr){
    if(*string_base_address == NULL || (start == NULL || start < string_base_address) || substr == NULL)
        return NULL;

    size_t newstr_len = strlen(*string_base_address) + strlen(substr) + 1;
    char *newstr = (char*)calloc(newstr_len, sizeof(char));


    if(start == *string_base_address){
        snprintf(newstr, newstr_len, "%s%s", substr, *string_base_address);
        start = (char*)(newstr + strlen(substr));
    }
    else if(*start == '\0'){
        snprintf(newstr, newstr_len, "%s%s", *string_base_address, substr);
        start = (char*)(newstr + strlen(substr) + strlen(*string_base_address));
    }
    else{
        size_t offset = start - (*string_base_address);
        char intersection = *start;
        *start = '\0';
        start++;

        snprintf(newstr, newstr_len, "%s%c%s%s", *string_base_address, intersection, substr, start);

        start = (char*)(newstr + offset + strlen(substr));
    }


    free(*string_base_address);
    *string_base_address = newstr;

    return start;
}

// make a string repeating a char
char *makechrspan(char chr, size_t len){
    char *span = (char*)calloc(len + 1, sizeof(char));

    for(int i = 0; i < len; i++)
        span[i] = chr;

    span[len] = '\0'; 

    return span;
}

void strfmtln(char **string_base_address, char *line, int minlen, int maxlen, strfmt_enum fmt){
    if(*string_base_address == NULL || (line == NULL || line < (*string_base_address)))
        return NULL;

    size_t offset = line - *string_base_address;
    size_t line_len = strcspn(line, "\n") - 1;
    char *linebreak = strpbrk(line, "\n");
    linebreak = '\0';

    // stuff
    if(line_len < minlen){
        switch(fmt & strfmt_align_bitmask){
            case strfmt_align_center:
                break;

            case strfmt_align_right:
                break;
                
            default:
            case strfmt_align_left:
                {
                    char *spaces = makechrspan(' ', minlen - line_len);
                    strins_substr(&string_base_address, line, )
                    free(spaces);
                }
                break;
        }
    }
}

// string formatter, non destructive, returns new memory
char *strfmtr(char *string, int min_lines, int max_lines, int min_line_len, int max_line_len, strfmt_enum fmt){
    if(string == NULL)
        return NULL;

    if(min_line_len <= 0 || min_lines <=0 || max_lines < min_lines || max_line_len < min_line_len)
        return NULL;
    
    char *base_string = calloc(1, strlen(string) + 1);
    if(base_string == NULL)
        return NULL;

    strcpy(base_string, string);

    char *cur_line = base_string;
    size_t chars = 0;
    size_t lines = 0;

    for(char *cursor = base_string; true; cursor++){
        bool last_loop = false;
        
        if(*cursor == '\0')
            last_loop = true;

        // handle line
        if(*cursor == '\n' || *cursor == '\0'){
            if(chars < min_line_len){
                // stuff with spaces
            }

            lines++;
        }

        if(chars > max_line_len){
            // insert line break
        }

        if(lines > max_lines){
            // do line stuff
        }

        if(last_loop && lines < min_lines){
            // stuff with line breaks
        }

        chars++;

        if(last_loop)
            break;
    }

    return base_string;
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
            run_whitespace(&cursor);
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
void wdraw_label(WINDOW *win, int y, int x, int minh, int minw, int maxh, int maxw, 
    char *text, frame_charset_t frame, chtype fill){

    int lines, max_w;
    
    strlnmaxw(text, &lines, &max_w);


    wdraw_rect(win, lines + 2, max_w + 2, y, x, frame, fill);
    
    mvwprintwln(win, y + 1, x + 1, text);
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
        // wdraw_label(nodes_win, 1, 1, "IP: 0000\nPORT:2222", frame_solid, ' ');
        mvwprintwln(nodes_win, 1, 1, "IP: 0000\nPORT:2222\nMASK:127");

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