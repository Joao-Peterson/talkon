#ifndef _MYTUI_HEADER_
#define _MYTUI_HEADER_

#include "curses_extra.h"

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
}mytui;

/* ----------------------------------------- Functions -------------------------------------- */

// init windows
void mytui_init(){    

    int height, width;
    getmaxyx(stdscr, height, width);
    
    // setting up windows
    mytui.windows.main.win = newwin(0, 0, 0, 0);
    mytui.windows.main.pan = new_panel(mytui.windows.main.win);

    mytui.windows.nav.win = newwin(0, 0, 0, 0);
    mytui.windows.nav.pan = new_panel(mytui.windows.nav.win);
    mytui.windows.nav.size.h = 3;
    mytui.windows.nav.size.w = width - 2;
    mytui.windows.nav.size.y = 1;
    mytui.windows.nav.size.x = 1;

    mytui.windows.nodes.win = newwin(0, 0, 0, 0);
    mytui.windows.nodes.pan = new_panel(mytui.windows.nodes.win);
    mytui.windows.nodes.size.h = height - 5;
    mytui.windows.nodes.size.w = width * 0.4;
    mytui.windows.nodes.size.y = 4;
    mytui.windows.nodes.size.x = 1;

    mytui.windows.talk.win = newwin(0, 0, 0, 0);
    mytui.windows.talk.pan = new_panel(mytui.windows.talk.win);
    mytui.windows.talk.size.h = height - 5 - mytui.windows.input.size.h;
    mytui.windows.talk.size.w = mytui.windows.input.size.w;
    mytui.windows.talk.size.y = 4;
    mytui.windows.talk.size.x = mytui.windows.nodes.size.w + 1;

    mytui.windows.input.win = newwin(0, 0, 0, 0);
    mytui.windows.input.pan = new_panel(mytui.windows.input.win);
    mytui.windows.input.size.h = 7;
    mytui.windows.input.size.w = width - 2 - mytui.windows.nodes.size.w;
    mytui.windows.input.size.y = height - 7 - 1;
    mytui.windows.input.size.x = mytui.windows.nodes.size.w + 1;

    // refresh curses
    refresh();
}

// draw to curses
void mytui_draw(){
    int height, width;
    getmaxyx(stdscr, height, width);
    // clear();

    // main window
    wclear(mytui.windows.main.win);
    wresize(mytui.windows.main.win, height, width);
    wborder_frame(mytui.windows.main.win, frame_normal);
    mvwprintw(mytui.windows.main.win, 0, 3, "[Talkon - TCP LAN Messenger v1.0]");

    // navbar / menu
    wclear(mytui.windows.nav.win);
    mytui.windows.nav.size.h = 3;
    mytui.windows.nav.size.w = width - 2;
    mytui.windows.nav.size.y = 1;
    mytui.windows.nav.size.x = 1;
    wsetsize(mytui.windows.nav.win, mytui.windows.nav.size);
    wborder_frame(mytui.windows.nav.win, frame_normal);
    wdraw_rect(mytui.windows.nav.win, 1, mytui.windows.nav.size.w - 2, 1, 1, frame_grain_thin, ' ');
    mvwprintw(mytui.windows.nav.win, 1, 3, "Configuration");

    // nodes / network
    wclear(mytui.windows.nodes.win);
    mytui.windows.nodes.size.h = height - 5;
    mytui.windows.nodes.size.w = width * 0.4;
    mytui.windows.nodes.size.y = 4;
    mytui.windows.nodes.size.x = 1;
    wsetsize(mytui.windows.nodes.win, mytui.windows.nodes.size);
    wborder_frame(mytui.windows.nodes.win, frame_normal);
    mvwprintw(mytui.windows.nodes.win, 0, 3, " Network ");
    
    wdraw_label(
        mytui.windows.nodes.win, "#1234\njoao-peterson\n192.168.1.255:4092", 
        1, 1, 3, 3, 28, 28,
        strfmt_align_center | strfmt_lines_cut | strfmt_linebreak_no_wrap_dot_dot_dot,
        frame_dotted, ' ',
        NULL
    );

    // talk / chat
    wclear(mytui.windows.talk.win);
    mytui.windows.talk.size.h = height - 5 - mytui.windows.input.size.h;
    mytui.windows.talk.size.w = mytui.windows.input.size.w;
    mytui.windows.talk.size.y = 4;
    mytui.windows.talk.size.x = mytui.windows.nodes.size.w + 1;
    wsetsize(mytui.windows.talk.win, mytui.windows.talk.size);
    wborder_frame(mytui.windows.talk.win, frame_normal);
    mvwprintw(mytui.windows.talk.win, 0, 3, " Chat ");

    // input
    wclear(mytui.windows.input.win);
    mytui.windows.input.pan = new_panel(mytui.windows.input.win);
    mytui.windows.input.size.h = 7;
    mytui.windows.input.size.w = width - 2 - mytui.windows.nodes.size.w;
    mytui.windows.input.size.y = height - 7 - 1;
    mytui.windows.input.size.x = mytui.windows.nodes.size.w + 1;
    wsetsize(mytui.windows.input.win, mytui.windows.input.size);
    wborder_frame(mytui.windows.input.win, frame_normal);
    mvwprintw(mytui.windows.input.win, 0, 3, " Input ");

    // wrefresh(nav_win);
    // wrefresh(main_win);
    // wrefresh(nodes_win);
    // wrefresh(talk_win);
    // wrefresh(input_win);
}

#endif