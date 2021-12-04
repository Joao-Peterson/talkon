#ifndef _CURSES_EXTRA_HEADER_
#define _CURSES_EXTRA_HEADER_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <plibsys.h>
#include "strfmt.h"

/* ----------------------------------------- Defines ---------------------------------------- */

#if defined(P_OS_LINUX)
    #include <curses.h>
    #include <panel.h>
#elif defined(P_OS_WIN) || defined(P_OS_WIN64)
    #include <curses.h>
    #include <panel.h>
#elif defined(P_OS_ANDROID)
    #error android is not supported because of curses library compatibility. \
Feel free to port this application to android. 
#elif defined(P_OS_MAC) || defined(P_OS_MAC9)
    #error macOS9 and macOS are not supported because of curses library compatibility. \
Feel free to port this application to macOS.
#elif
    #error your OS is not supported because of curses library compatibility. \
Feel free to port this application to your OS.
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
extern frame_charset_t frame_dotted_corner_slash;
extern frame_charset_t frame_dotted_corner_star;
extern frame_charset_t frame_square_brackets;
extern frame_charset_t frame_double;
extern frame_charset_t frame_hash;
extern frame_charset_t frame_grain_thin;
extern frame_charset_t frame_grain_thick;
extern frame_charset_t frame_solid;

/* ----------------------------------------- Functions -------------------------------------- */

// init frames
void curses_extra_init(void);

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