#include "curses_extra.h"
#include <curses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ----------------------------------------- Globals ---------------------------------------- */

frame_charset_t frame_noframe;
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
void curses_extra_init(void){
    frame_charset_t tmp_frame_noframe = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
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

    frame_noframe = tmp_frame_noframe;
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

// creates a loading icon object with a "time_ms" update time and with "size" elements on a side
loading_icon_t *loading_icon_new(uint32_t time_ms, uint32_t size){
    loading_icon_t *icon = (loading_icon_t*)calloc(1, sizeof(loading_icon_t));

    if(size < 2) return NULL;

    icon->time_ms = time_ms;
    icon->size = size;
    icon->buffer_size = (size - 2)*4 + 4; // icon side has size "size", thus the whole perimeter is the 4 corners + the middle sections (size-2)*4
    icon->buffer = (chtype*)calloc(icon->buffer_size, sizeof(chtype));

    for(size_t i = 0; i < icon->buffer_size; i++)
        icon->buffer[i] = ' '; 

    icon->buffer[0] = ACS_CKBOARD;
    icon->buffer[1] = ACS_CKBOARD;
    icon->buffer[2] = ACS_CKBOARD;

    icon->time_profiler = p_time_profiler_new(); 
    icon->last_time = 0;

    return icon;
}

// updates the icon according to the preset time 
void loading_icon_draw(loading_icon_t *icon, WINDOW *win, int y, int x){

    // update buffer on time
    uint64_t time_now = p_time_profiler_elapsed_usecs(icon->time_profiler);
    if((time_now - icon->last_time) > (icon->time_ms*1000)){

        p_time_profiler_reset(icon->time_profiler);

        // shift the buffer one time forwards
        chtype temp = icon->buffer[icon->buffer_size - 1];
        for(size_t i = icon->buffer_size - 1; i > 0; i--){
            icon->buffer[i] = icon->buffer[i-1]; 
        }
        icon->buffer[0] = temp;
    }
    
    // draw the square on screen
    for(uint32_t i = 0; i < 4; i++){
        for(uint32_t j = 0; j < (icon->size-1); j++){
            switch(i){
                // top
                case 0:
                    // mvwaddch(win, y, x + j, (i*(icon->size-1) + j)+48);
                    mvwaddch(win, y, x + j, icon->buffer[i*(icon->size-1) + j]);
                    break;

                // right
                case 1:
                    // mvwaddch(win, y + j, x + (icon->size-1), (i*(icon->size-1) + j)+48);
                    mvwaddch(win, y + j, x + (icon->size-1), icon->buffer[i*(icon->size-1) + j]);
                    
                // bottom
                case 2:
                    // mvwaddch(win, y + (icon->size-1), (icon->size-1 + x) - j, (i*(icon->size-1) + j)+48);
                    mvwaddch(win, y + (icon->size-1), (icon->size-1 + x) - j, icon->buffer[i*(icon->size-1) + j]);
                    break;

                // left
                case 3:
                    // mvwaddch(win, (icon->size-1 + y) - j, x, (i*(icon->size-1) + j)+48);
                    mvwaddch(win, (icon->size-1 + y) - j, x, icon->buffer[i*(icon->size-1) + j]);
                    break;
            }
        }
    }
}

// delete loading icon object
void loading_icon_delete(loading_icon_t *icon){
    free(icon->buffer);
    p_time_profiler_free(icon->time_profiler);
    free(icon);
}
