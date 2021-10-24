#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <doc.h>
#include <doc_json.h>
#include "config.h"
#include "strfmt.h"
#include "curses_extra.h"
#include "tui.h"
#include "simple_tcp_msg.h"
#include "receiver.h"
#include "log.h"

#include <doc.h>
#include <doc_json.h>

#include "config.h"
#include "strfmt.h"
#include "curses_extra.h"
#include "tui.h"
#include "simple_tcp_msg.h"
#include "stcp.h"

#include <plibsys.h>

// data type for interfacing with the receiver thread
typedef struct{
    int run_flag;
}receiver_thread_interface_t;

// receiver thread daemon routine
void *receiver_thread_routine(void *data){
    receiver_thread_interface_t *interface = (receiver_thread_interface_t*)data;

    receiver_t *receiver = receiver_init();
    
    if(receiver == NULL){
        log_error("Receiver init failed\n");
        interface->run_flag = 0;
        return NULL;
    }

    while(interface->run_flag){
        receiver_listen(receiver);
    }

    receiver_delete(receiver);
}




int main(int argc, char **argv){

    p_libsys_init();
    p_libsys_shutdown();

    // main variables
    receiver_thread_interface_t receiver_thread_interface = {
        .run_flag = 1
    };

    // log file
    char buffer[500];
    snprintf(buffer, 500, "%s/%s", config_get_config_folder_path(), "log.txt");
    log_set_output_file(buffer, "w+");

    // read config file
    config_init();
    
    doc_set(config_doc, "port", int, atoi(argv[1]));

    // create threads for messaging
    pthread_t receiver_thread;
    if(pthread_create(&receiver_thread, NULL, &receiver_thread_routine, &receiver_thread_interface) < 0){
        log_error("Thread creation failed\n");
        return -1;
    }
    
    // initialize curses
    initscr();
    // cbreak();
    timeout(-1);
    noecho();
    clear();
    curs_set(0);

    // check colors
    if(!has_colors()){
        printw("This terminal doesn't support colors!\nPlease use another terminal");
        getch();
        exit(-1);
    }
    if(!can_change_color()){
        printw("This terminal doesn't support custom colors!\nPlease use another terminal");
        getch();
        exit(-1);
    }

    start_color();

    curses_extra_init();
    tui_init();

    // first render
    update_panels();
    tui_draw();
    tui_draw();
    refresh();

    // main loop
    while(1){
        
        // ----------------------- daemons ---------------------

        if(receiver_thread_interface.run_flag == 0){
            endwin();
            pthread_join(receiver_thread, NULL);
            config_save();

            printf("An error was occured while starting the TCP comunication.\n");
            printf("Please open your log file in: \"%s\"\nfor more details.\n", config_get_config_folder_path());
            return 0;
        }
        
        // ----------------------- draw ------------------------
        tui_draw();
        update_panels();
        refresh();

        // ----------------------- input -----------------------
        int input = getch();

        tui_logic(input);

        switch(input){
            case 'q':
                {
                    endwin();
                    receiver_thread_interface.run_flag = 0;
                    pthread_join(receiver_thread, NULL);
                    config_save();
                
                    // close rfopen() of stderr by log.c
                    // works without it, lets see how long 
                    // fclose(stderr);
                    return 0;
                }
                break;

            default:
                break;
        }

    }

    return 0;
}