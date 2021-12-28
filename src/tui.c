#include "curses_extra.h"
#include "db.h"
#include "strfmt.h"
#include "tui.h"
#include "config.h"
#include "simple_tcp_msg.h"
#include <stdio.h>
#include <curses.h>
#include <doc.h>
#include <math.h>
#include "log.h"

/* ----------------------------------------- Structs ---------------------------------------- */

// global tui object
tui_t tui;

/* ----------------------------------------- Functions -------------------------------------- */

// init windows and main global state
void tui_init(void){    

    int height, width;
    getmaxyx(stdscr, height, width);
    // colors
    init_color(color_text, 800, 900, 1000);
    init_color(color_text_highlight, 975, 975, 1000);
    
    init_color(color_border, 800, 900, 1000);
    init_color(color_border_highlight, 300, 500, 700);
    // init_color(color_border_highlight, 600, 700, 900);
    
    init_color(color_background, 20, 60, 80);

    init_pair(color_pair_normal, color_text, color_background);
    init_pair(color_pair_border, color_border, color_background);
    init_pair(color_pair_border_highlight, color_border_highlight, color_background);

    tui.cur_sel_win = window_id_nodes;
    tui.window_frame_normal = frame_normal;
    tui.window_frame_selected = frame_normal;

    // setting up windows
    tui.windows.main.win = newwin(0, 0, 0, 0);
    tui.windows.main.pan = new_panel(tui.windows.main.win);

    tui.windows.nav.win = newwin(0, 0, 0, 0);
    tui.windows.nav.pan = new_panel(tui.windows.nav.win);

    tui.windows.nodes.win = newwin(0, 0, 0, 0);
    tui.windows.nodes.pan = new_panel(tui.windows.nodes.win);

    tui.windows.talk.win = newwin(0, 0, 0, 0);
    tui.windows.talk.pan = new_panel(tui.windows.talk.win);

    tui.windows.input.win = newwin(0, 0, 0, 0);
    tui.windows.input.pan = new_panel(tui.windows.input.win);

    tui.nodes = NULL;

    tui.window_nodes_scroll = 0;
    tui.window_input_scroll = 0;
    tui.window_talk_scroll = 0;

    // refresh curses
    refresh();
}

// main window draw function
void main_window(void){
    wclear(tui.windows.main.win);
    wresize(tui.windows.main.win, tui.terminal_h, tui.terminal_w);

    wattron(tui.windows.main.win, COLOR_PAIR(color_pair_border));
    
    {
        wdraw_rect(tui.windows.main.win, tui.terminal_h, tui.terminal_w, 0, 0, frame_normal, ' ');
        // wborder_frame(tui.windows.main.win, frame_normal);
        mvwprintw(tui.windows.main.win, 0, 3, "[Talkon - TCP LAN Messenger v1.0]");
    }

    wattroff(tui.windows.main.win, COLOR_PAIR(color_pair_border));
}

// nav window draw function
void nav_window(void){
    wclear(tui.windows.nav.win);
    tui.windows.nav.size.h = 3;
    tui.windows.nav.size.w = tui.terminal_w - 2;
    tui.windows.nav.size.y = 1;
    tui.windows.nav.size.x = 1;
    wsetsize(tui.windows.nav.win, tui.windows.nav.size);

    wattron(tui.windows.nav.win, COLOR_PAIR(color_pair_border));

    {
        wdraw_rect(tui.windows.nav.win, tui.windows.nav.size.h, tui.windows.nav.size.w, 0, 0, frame_normal, ' ');
        // wborder_frame(tui.windows.nav.win, frame_normal);
        wdraw_rect(tui.windows.nav.win, 1, tui.windows.nav.size.w - 2, 1, 1, frame_grain_thin, ' ');
        mvwprintw(tui.windows.nav.win, 1, 3, profile_get("name", char*));
    }
    
    wattroff(tui.windows.nav.win, COLOR_PAIR(color_pair_border));
}

// nodes window draw function
void nodes_window(void){
    wclear(tui.windows.nodes.win);
    tui.windows.nodes.size.h = tui.terminal_h - 5;
    tui.windows.nodes.size.w = tui.terminal_w * 0.4;
    tui.windows.nodes.size.y = 4;
    tui.windows.nodes.size.x = 1;
    wsetsize(tui.windows.nodes.win, tui.windows.nodes.size);

    if(tui.cur_sel_win == window_id_nodes)
        wattron(tui.windows.nodes.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattron(tui.windows.nodes.win, COLOR_PAIR(color_pair_border));

    // draw commands
    {
        wdraw_rect(tui.windows.nodes.win, tui.windows.nodes.size.h, tui.windows.nodes.size.w, 0, 0, frame_normal, ' ');
        mvwprintw(tui.windows.nodes.win, 0, 3, " Network ");

        size_t nodes_max =  (size_t)floor((double)(tui.windows.nodes.size.h-2) / (double)(profile_pic_height+2));
        log_debug("node_max: %i.\n", nodes_max);
        size_t nodes_q = doc_get_size(tui.nodes, ".");
        size_t scroll_size = (size_t)((double)(tui.windows.nodes.size.h - 4) / (double)nodes_max);
        log_debug("scroll_size: %i.\n", scroll_size);

        log_debug("window_nodes_scroll before: %i.\n", tui.window_nodes_scroll);
        if((tui.window_nodes_scroll * nodes_max) > nodes_q)
            tui.window_nodes_scroll = nodes_q / nodes_max;

        log_debug("window_nodes_scroll after: %i.\n", tui.window_nodes_scroll);

        // scroll bar
        size_t max_w = tui.windows.nodes.size.h - 2;
        for(size_t i = 1; i <= max_w; i++){
            if(i == 1)
                mvwaddch(tui.windows.nodes.win, i, tui.windows.nodes.size.w - 2, '+');

            else if(i == max_w)
                mvwaddch(tui.windows.nodes.win, i, tui.windows.nodes.size.w - 2, '-');
                
            // scroll handle
            else{                                   
                size_t start = 2 + tui.window_nodes_scroll*scroll_size;                 
                size_t end = start + scroll_size;                 
                if(i >= start && i < end)
                    mvwaddch(tui.windows.nodes.win, i, tui.windows.nodes.size.w - 2, ACS_CKBOARD);

                else
                    mvwaddch(tui.windows.nodes.win, i, tui.windows.nodes.size.w - 2, ACS_VLINE);
            }
        }

        // draw nodes
        if(tui.nodes != NULL && nodes_q > 0){

            size_t i = 0;
            for(doc_loop(node, tui.nodes)){

                if(i >= tui.window_nodes_scroll){
                    
                    if(i >= (nodes_max + tui.window_nodes_scroll))
                        break;

                    log_debug("printing node[%i].\n", i);

                    // profile pic + border
                    wdraw_label(
                        tui.windows.nodes.win, doc_get(node, "pic", char*), 
                        (profile_pic_height+2) * (i-tui.window_nodes_scroll) + 1, 1, 7, 7, 
                        tui.windows.nodes.size.w - 3, tui.windows.nodes.size.w - 3,
                        (strfmt_t)(strfmt_align_left | strfmt_lines_cut | strfmt_linebreak_no_wrap_dot_dot_dot),
                        frame_dotted, ' ',
                        NULL
                    );

                    // name + id
                    char buffer[500];
                    snprintf(buffer, 500, "%s\n%s", doc_get(node, "name", char*), doc_get(node, "uuid", char*));
                    wdraw_label(
                        tui.windows.nodes.win, buffer, 
                        (profile_pic_height+2) * (i-tui.window_nodes_scroll) + 2,
                        profile_pic_width, 5, 5, 
                        3, tui.windows.nodes.size.w - 3 - profile_pic_width,
                        (strfmt_t)(strfmt_align_center | strfmt_lines_cut | strfmt_linebreak_no_wrap_dot_dot_dot),
                        frame_noframe, ' ',
                        NULL
                    );

                }

                i++;
            }
            
        }
        else{
            wdraw_label(
                tui.windows.nodes.win, "No known nodes available!", 
                1, 1, 3, 4, tui.windows.nodes.size.w - 2, tui.windows.nodes.size.w - 2,
                (strfmt_t)(strfmt_align_left | strfmt_lines_cut | strfmt_linebreak_no_wrap_dot_dot_dot),
                frame_dotted, ' ',
                NULL
            );
        }

    }

    if(tui.cur_sel_win == window_id_nodes)
        wattroff(tui.windows.nodes.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattroff(tui.windows.nodes.win, COLOR_PAIR(color_pair_border));
}

// talk window draw function
void talk_window(void){
    wclear(tui.windows.talk.win);
    tui.windows.talk.size.h = tui.terminal_h - 5 - tui.windows.input.size.h;
    tui.windows.talk.size.w = tui.windows.input.size.w;
    tui.windows.talk.size.y = 4;
    tui.windows.talk.size.x = tui.windows.nodes.size.w + 1;
    wsetsize(tui.windows.talk.win, tui.windows.talk.size);
    
    if(tui.cur_sel_win == window_id_talk)
        wattron(tui.windows.talk.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattron(tui.windows.talk.win, COLOR_PAIR(color_pair_border));

    // draw commands
    {
        wdraw_rect(tui.windows.talk.win, tui.windows.talk.size.h, tui.windows.talk.size.w, 0, 0, frame_normal, ' ');
        mvwprintw(tui.windows.talk.win, 0, 3, " Chat ");
    }

    if(tui.cur_sel_win == window_id_talk)
        wattroff(tui.windows.talk.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattroff(tui.windows.talk.win, COLOR_PAIR(color_pair_border));
}

// input window draw function
void input_window(void){
    wclear(tui.windows.input.win);
    tui.windows.input.pan = new_panel(tui.windows.input.win);
    tui.windows.input.size.h = 7;
    tui.windows.input.size.w = tui.terminal_w - 2 - tui.windows.nodes.size.w;
    tui.windows.input.size.y = tui.terminal_h - 7 - 1;
    tui.windows.input.size.x = tui.windows.nodes.size.w + 1;
    wsetsize(tui.windows.input.win, tui.windows.input.size);
    
    if(tui.cur_sel_win == window_id_input)
        wattron(tui.windows.input.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattron(tui.windows.input.win, COLOR_PAIR(color_pair_border));

    // draw commands
    {
        wdraw_rect(tui.windows.input.win, tui.windows.input.size.h, tui.windows.input.size.w, 0, 0, frame_normal, ' ');
        mvwprintw(tui.windows.input.win, 0, 3, " Input ");
    }

    if(tui.cur_sel_win == window_id_input)
        wattroff(tui.windows.input.win, COLOR_PAIR(color_pair_border_highlight));
    else
        wattroff(tui.windows.input.win, COLOR_PAIR(color_pair_border));
}

// draw to curses
void tui_draw(void){

    getmaxyx(stdscr, tui.terminal_h, tui.terminal_w);

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
void tui_logic(int input){
    switch(input){
        case '\t':
            {
                switch(tui.cur_sel_win){
                    case window_id_nodes:
                        tui.cur_sel_win = window_id_talk;
                        break;
                        
                    case window_id_talk:
                        tui.cur_sel_win = window_id_input;
                        break;
                        
                    default:
                    case window_id_input:
                        tui.cur_sel_win = window_id_nodes;
                        break;
                }
            }
            break;
    }
}
