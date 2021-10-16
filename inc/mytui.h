#ifndef _MYTUI_HEADER_
#define _MYTUI_HEADER_

#include "curses_extra.h"

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

/* ----------------------------------------- Structs ---------------------------------------- */

// global tui object
struct mytui_t{
    struct{
        curses_window_t main;
        curses_window_t nav;
        curses_window_t nodes;
        curses_window_t talk;
        curses_window_t input;
    }windows;

    int terminal_h;
    int terminal_w;

    frame_charset_t window_frame_normal;
    frame_charset_t window_frame_selected;

    window_id_t cur_sel_win;
}mytui;

/* ----------------------------------------- Functions -------------------------------------- */

// init windows and main global state
void mytui_init(void){    

    int height, width;
    getmaxyx(stdscr, height, width);
    // colors
    init_color(color_text, 800, 900, 1000);
    init_color(color_text_highlight, 975, 975, 1000);
    
    init_color(color_border, 800, 900, 1000);
    init_color(color_border_highlight, 600, 700, 900);
    
    init_color(color_background, 20, 60, 80);

    init_pair(color_pair_normal, color_text, color_background);
    init_pair(color_pair_border, color_border, color_background);
    init_pair(color_pair_border_highlight, color_border_highlight, color_background);

    mytui.cur_sel_win = window_id_nodes;
    mytui.window_frame_normal = frame_normal;
    mytui.window_frame_selected = frame_normal;

    // setting up windows
    mytui.windows.main.win = newwin(0, 0, 0, 0);
    mytui.windows.main.pan = new_panel(mytui.windows.main.win);

    mytui.windows.nav.win = newwin(0, 0, 0, 0);
    mytui.windows.nav.pan = new_panel(mytui.windows.nav.win);

    mytui.windows.nodes.win = newwin(0, 0, 0, 0);
    mytui.windows.nodes.pan = new_panel(mytui.windows.nodes.win);

    mytui.windows.talk.win = newwin(0, 0, 0, 0);
    mytui.windows.talk.pan = new_panel(mytui.windows.talk.win);

    mytui.windows.input.win = newwin(0, 0, 0, 0);
    mytui.windows.input.pan = new_panel(mytui.windows.input.win);

    // refresh curses
    refresh();
}

// main window draw function
void main_window(void){
    wclear(mytui.windows.main.win);
    wresize(mytui.windows.main.win, mytui.terminal_h, mytui.terminal_w);

    wattron(mytui.windows.main.win, COLOR_PAIR(color_pair_border));
    
    {
        wdraw_rect(mytui.windows.main.win, mytui.terminal_h, mytui.terminal_w, 0, 0, frame_normal, ' ');
        // wborder_frame(mytui.windows.main.win, frame_normal);
        mvwprintw(mytui.windows.main.win, 0, 3, "[Talkon - TCP LAN Messenger v1.0]");
    }

    wattroff(mytui.windows.main.win, COLOR_PAIR(color_pair_border));
}

// nav window draw function
void nav_window(void){
    wclear(mytui.windows.nav.win);
    mytui.windows.nav.size.h = 3;
    mytui.windows.nav.size.w = mytui.terminal_w - 2;
    mytui.windows.nav.size.y = 1;
    mytui.windows.nav.size.x = 1;
    wsetsize(mytui.windows.nav.win, mytui.windows.nav.size);

    wattron(mytui.windows.nav.win, COLOR_PAIR(color_pair_border));

    {
        wdraw_rect(mytui.windows.nav.win, mytui.windows.nav.size.h, mytui.windows.nav.size.w, 0, 0, frame_normal, ' ');
        // wborder_frame(mytui.windows.nav.win, frame_normal);
        wdraw_rect(mytui.windows.nav.win, 1, mytui.windows.nav.size.w - 2, 1, 1, frame_grain_thin, ' ');
        mvwprintw(mytui.windows.nav.win, 1, 3, "Configuration");
    }
    
    wattroff(mytui.windows.nav.win, COLOR_PAIR(color_pair_border));
}

// nodes window draw function
void nodes_window(void){
    wclear(mytui.windows.nodes.win);
    mytui.windows.nodes.size.h = mytui.terminal_h - 5;
    mytui.windows.nodes.size.w = mytui.terminal_w * 0.4;
    mytui.windows.nodes.size.y = 4;
    mytui.windows.nodes.size.x = 1;
    wsetsize(mytui.windows.nodes.win, mytui.windows.nodes.size);

    if(mytui.cur_sel_win == window_id_nodes)
        wattron(mytui.windows.nodes.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattron(mytui.windows.nodes.win, COLOR_PAIR(color_pair_border));

    // draw commands
    {
        wdraw_rect(mytui.windows.nodes.win, mytui.windows.nodes.size.h, mytui.windows.nodes.size.w, 0, 0, frame_normal, ' ');
        mvwprintw(mytui.windows.nodes.win, 0, 3, " Network ");
        
        wdraw_label(
            mytui.windows.nodes.win, "#1234\njoao-peterson\n192.168.1.255:4092", 
            1, 1, 5, 5, mytui.windows.nodes.size.w - 2, mytui.windows.nodes.size.w - 2,
            strfmt_align_center | strfmt_lines_cut | strfmt_linebreak_no_wrap_dot_dot_dot,
            frame_dotted, '#',
            NULL
        );
    }

    if(mytui.cur_sel_win == window_id_nodes)
        wattroff(mytui.windows.nodes.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattroff(mytui.windows.nodes.win, COLOR_PAIR(color_pair_border));
}

// talk window draw function
void talk_window(void){
    wclear(mytui.windows.talk.win);
    mytui.windows.talk.size.h = mytui.terminal_h - 5 - mytui.windows.input.size.h;
    mytui.windows.talk.size.w = mytui.windows.input.size.w;
    mytui.windows.talk.size.y = 4;
    mytui.windows.talk.size.x = mytui.windows.nodes.size.w + 1;
    wsetsize(mytui.windows.talk.win, mytui.windows.talk.size);
    
    if(mytui.cur_sel_win == window_id_talk)
        wattron(mytui.windows.talk.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattron(mytui.windows.talk.win, COLOR_PAIR(color_pair_border));

    // draw commands
    {
        wdraw_rect(mytui.windows.talk.win, mytui.windows.talk.size.h, mytui.windows.talk.size.w, 0, 0, frame_normal, ' ');
        mvwprintw(mytui.windows.talk.win, 0, 3, " Chat ");
    }

    if(mytui.cur_sel_win == window_id_talk)
        wattroff(mytui.windows.talk.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattroff(mytui.windows.talk.win, COLOR_PAIR(color_pair_border));
}

// input window draw function
void input_window(void){
    wclear(mytui.windows.input.win);
    mytui.windows.input.pan = new_panel(mytui.windows.input.win);
    mytui.windows.input.size.h = 7;
    mytui.windows.input.size.w = mytui.terminal_w - 2 - mytui.windows.nodes.size.w;
    mytui.windows.input.size.y = mytui.terminal_h - 7 - 1;
    mytui.windows.input.size.x = mytui.windows.nodes.size.w + 1;
    wsetsize(mytui.windows.input.win, mytui.windows.input.size);
    
    if(mytui.cur_sel_win == window_id_input)
        wattron(mytui.windows.input.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattron(mytui.windows.input.win, COLOR_PAIR(color_pair_border));

    // draw commands
    {
        wdraw_rect(mytui.windows.input.win, mytui.windows.input.size.h, mytui.windows.input.size.w, 0, 0, frame_normal, ' ');
        mvwprintw(mytui.windows.input.win, 0, 3, " Input ");
    }

    if(mytui.cur_sel_win == window_id_input)
        wattroff(mytui.windows.input.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattroff(mytui.windows.input.win, COLOR_PAIR(color_pair_border));
}

// draw to curses
void mytui_draw(void){

    getmaxyx(stdscr, mytui.terminal_h, mytui.terminal_w);

    // main window
    main_window();

    // navbar / menu
    nav_window();

    // nodes / network
    nodes_window();

    // talk / chat
    talk_window();

    // input
    input_window();

}

// logic
void mytui_logic(int input){
    switch(input){
        case '\t':
            {
                switch(mytui.cur_sel_win){
                    case window_id_nodes:
                        mytui.cur_sel_win = window_id_talk;
                        break;
                        
                    case window_id_talk:
                        mytui.cur_sel_win = window_id_input;
                        break;
                        
                    default:
                    case window_id_input:
                        mytui.cur_sel_win = window_id_nodes;
                        break;
                }
            }
            break;
    }
}

#endif