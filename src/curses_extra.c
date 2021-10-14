#include "../inc/curses_extra.h"

/* ----------------------------------------- Globals ---------------------------------------- */

frame_charset_t frame_normal;
frame_charset_t frame_dotted;
frame_charset_t frame_dotted_corner_slash;
frame_charset_t frame_dotted_corner_star;
frame_charset_t frame_square_brackets;
frame_charset_t frame_double;
frame_charset_t frame_hash;
frame_charset_t frame_grain_thin;
frame_charset_t frame_grain_thick;
frame_charset_t frame_solid;

/* ----------------------------------------- Functions -------------------------------------- */

// init frames
void frames_init(void){
    frame_charset_t tmp_frame_normal = { ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER };
    frame_charset_t tmp_frame_dotted = { '|', '|', '-', '-', 126 | A_ALTCHARSET, 126 | A_ALTCHARSET, 126 | A_ALTCHARSET, 126 | A_ALTCHARSET };
    frame_charset_t tmp_frame_square_brackets = { '[', ']', ' ', ' ', ' ', ' ', ' ', ' ' };
    frame_charset_t tmp_frame_double = { '#', '#', '=', '=', '=', '=', '=', '=' };
    frame_charset_t tmp_frame_hash = { '#', '#', '#', '#', '#', '#', '#', '#' };
    frame_charset_t tmp_frame_grain_thin = { ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD, ACS_BOARD };
    frame_charset_t tmp_frame_grain_thick = { ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD };
    frame_charset_t tmp_frame_solid = { ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK, ACS_BLOCK };
    frame_charset_t tmp_frame_dotted_corner_slash = { '|', '|', '-', '-', '/', '\\', '\\', '/' };
    frame_charset_t tmp_frame_dotted_corner_star = { '|', '|', '-', '-', '*', '*', '*', '*' };

    frame_normal = tmp_frame_normal;
    frame_dotted = tmp_frame_dotted;
    frame_square_brackets = tmp_frame_square_brackets;
    frame_double = tmp_frame_double;
    frame_hash = tmp_frame_hash;
    frame_grain_thin = tmp_frame_grain_thin;
    frame_grain_thick = tmp_frame_grain_thick;
    frame_solid = tmp_frame_solid;
    frame_dotted_corner_slash = tmp_frame_dotted_corner_slash;
    frame_dotted_corner_star = tmp_frame_dotted_corner_star;
}

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

