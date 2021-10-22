#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "doc.h"
#include "doc_json.h"
#include "config.h"
#include "strfmt.h"
#include "curses_extra.h"
#include "tui.h"
#include "simple_tcp_msg.h"
#include "receiver.h"
#include "log.h"

#include "c_doc/doc.h"
#include "c_doc/doc_json.h"
#include "inc/config.h"
#include "inc/strfmt.h"
#include "inc/curses_extra.h"
#include "inc/tui.h"
#include "inc/simple_tcp_msg.h"
#include "stcp/stcp.h"

int main(int argc, char **argv){

    config_init();
    
    doc_set(config_doc, "port", int, atoi(argv[1]));

    pthread_t receiver_thread;
    if(pthread_create(&receiver_thread, NULL, &receiver_listen, NULL) < 0){
        log_error("Thread creation failed\n");
        return -1;
    }

    pthread_join(receiver_thread, NULL);
    
    config_save();


    // initscr();
    // // cbreak();
    // timeout(-1);
    // noecho();
    // clear();
    // curs_set(0);

    // // check colors
    // if(!has_colors()){
    //     printw("This terminal doesn't support colors!\nPlease use another terminal");
    //     getch();
    //     exit(-1);
    // }
    // if(!can_change_color()){
    //     printw("This terminal doesn't support custom colors!\nPlease use another terminal");
    //     getch();
    //     exit(-1);
    // }

    // start_color();

    // curses_extra_init();
    // tui_init();

    // // first render
    // update_panels();
    // tui_draw();
    // tui_draw();
    // refresh();

    // // main loop
    // while(1){
        
    //     // ----------------------- draw ------------------------
    //     tui_draw();
    //     update_panels();
    //     refresh();

    //     // ----------------------- input -----------------------
    //     int input = getch();

    //     tui_logic(input);

    //     switch(input){
    //         case 'q':
    //             endwin();
    //             exit(0);
    //             break;

    //         default:
    //             break;
    //     }

    // }

    // endwin();

    return 0;
}