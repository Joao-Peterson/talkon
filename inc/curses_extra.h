#ifndef _CURSES_EXTRA_HEADER_
#define _CURSES_EXTRA_HEADER_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "strfmt.h"

#if defined(__linux__) || defined(__unix__) || defined(__posix__)
    #include <curses.h>
    #include <panel.h>
#elif defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__NT__)
    #include <curses.h>
    #include <panel.h>
#elif defined(__ANDROID__)
    #error android is not supported!
#elif defined(__APPLE__) || defined(__MACH__)
    #error macOS is not supported!
#elif
    #error your OS is not supported!
#endif

/* ----------------------------------------- Structs ---------------------------------------- */

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

// window object
typedef struct{
    WINDOW *win;
    PANEL *pan;
    win_size_t size;
}curses_window_t;

/* ----------------------------------------- Globals ---------------------------------------- */

extern frame_charset_t frame_normal;
extern frame_charset_t frame_dotted;
extern frame_charset_t frame_dotted_chanfer_slash;
extern frame_charset_t frame_square_brackets;
extern frame_charset_t frame_double;
extern frame_charset_t frame_hash;
extern frame_charset_t frame_grain_thin;
extern frame_charset_t frame_grain_thick;
extern frame_charset_t frame_solid;

/* ----------------------------------------- Functions -------------------------------------- */

// init frames
void frames_init(void);

// set the size of a window
void wsetsize(WINDOW *window, win_size_t winsize);

// draw a rectangle with a frame
void wdraw_rect(WINDOW *win, int h, int w, int y, int x, frame_charset_t frame, chtype fill);

// draw a border with a frame
void wborder_frame(WINDOW *win, frame_charset_t frame);

// better mvwprintw
void mvwprintwln(WINDOW *win, int y, int x, char *string);

// draw text with a frame
void wdraw_label(WINDOW *win, char *text, int y, int x, int minh, int maxh, 
     int minw, int maxw, strfmt_t fmt, frame_charset_t frame, chtype fill, void *optional_data);


#endif