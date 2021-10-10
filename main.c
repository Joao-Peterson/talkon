#include <stdio.h>
#include <stdlib.h>
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

typedef struct{
    int h;
    int w;
    int y;
    int x;
}win_size_t;

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

void wsetsize(WINDOW *window, win_size_t winsize){
    mvwin(window, winsize.y, winsize.x);
    wresize(window, winsize.h, winsize.w);
}

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

int main(int argc, char **argv){

    initscr();
    cbreak();
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
        clear();

        // main window
        wclear(main_win);
        wresize(main_win, LINES, COLS);
        wborder_frame(main_win, frame_normal);
        mvwprintw(main_win, 0, 3, "[Talkon - TCP LAN Messenger v1.0]");

        // navbar / menu
        wclear(nav_win);
        win_size_t nav_win_size = {
            .h = 3, 
            .w = COLS - 2, 
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
            .h = LINES - 5, 
            .w = COLS * 0.4, 
            .y = 4, 
            .x = 1
        };

        wsetsize(nodes_win, nodes_win_size);
        wborder_frame(nodes_win, frame_normal);
        mvwprintw(nodes_win, 0, 3, " Network ");
        wdraw_rect(nodes_win, 7, nodes_win_size.w - 2, 1, 1, frame_dotted, ' ');

        // input
        wclear(input_win);
        win_size_t input_win_size = {
            .h = 7, 
            .w = COLS - 2 - nodes_win_size.w, 
            .y = LINES - 7 - 1, 
            .x = nodes_win_size.w + 1
        };

        wsetsize(input_win, input_win_size);
        wborder_frame(input_win, frame_normal);
        mvwprintw(input_win, 0, 3, " Input ");
        

        // talk / chat
        wclear(talk_win);
        win_size_t talk_win_size = {
            .h = LINES - 5 - input_win_size.h, 
            .w = input_win_size.w, 
            .y = 4, 
            .x = nodes_win_size.w + 1
        };

        wsetsize(talk_win, talk_win_size);
        wborder_frame(talk_win, frame_normal);
        mvwprintw(talk_win, 0, 3, " Chat ");

        // render
        wrefresh(nav_win);
        wrefresh(main_win);
        wrefresh(nodes_win);
        wrefresh(talk_win);
        wrefresh(input_win);
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