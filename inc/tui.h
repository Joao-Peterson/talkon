#ifndef _TUI_HEADER_
#define _TUI_HEADER_

#include <doc.h>
#include <plibsys.h>
#include "db.h"
#include "curses_extra.h"

/* ----------------------------------------- Defines ---------------------------------------- */

#define input_max_len   2000

/* ----------------------------------------- Enums ------------------------------------------ */

typedef enum{
    window_id_main,
    window_id_nav,
    window_id_nodes,
    window_id_talk,
    window_id_input
}window_id_t;

typedef enum{
    color_pair_normal = 1,
    color_pair_border,
    color_pair_border_highlight,
}color_pair_palette_t;

typedef enum{
    color_background = 127,
    color_text,
    color_text_highlight,
    color_border,
    color_border_highlight,
}color_palette_t;

typedef enum{
    tui_layer_base          = 0x01,                   
    tui_layer_animations    = 0x02,                           
    tui_layer_text          = 0x04                           
}tui_layer_t;

/* ----------------------------------------- Struct's ----------------------------------------- */

// tui meta info struct
typedef struct{
    struct{
        curses_window_t main;
        curses_window_t nav;
        curses_window_t nodes;
        curses_window_t talk;
        curses_window_t input;
    }windows;

    size_t window_nodes_scroll;
    size_t cur_sel_node;
    size_t window_talk_scroll;
    size_t window_input_scroll;

    int terminal_h;
    int terminal_w;

    frame_charset_t window_frame_normal;
    frame_charset_t window_frame_selected;

    window_id_t cur_sel_win;

    loading_icon_t *ping_icon;
    bool ping_icon_show;

    doc *nodes;

    char input_buffer[input_max_len];
}tui_t; 

/* ----------------------------------------- Globals ---------------------------------------- */

extern tui_t tui;

#define first_sel_win window_id_nodes

/* ----------------------------------------- Functions -------------------------------------- */

// init windows and main global state
void tui_init(void);

// draw to curses
void tui_draw(tui_layer_t layer);

// close tui
void tui_end(void);

#endif