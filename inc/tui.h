#ifndef _TUI_HEADER_
#define _TUI_HEADER_

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

/* ----------------------------------------- Functions -------------------------------------- */

// init windows and main global state
void tui_init(void);

// draw to curses
void tui_draw(void *data);

// logic
void tui_logic(int input);

#endif