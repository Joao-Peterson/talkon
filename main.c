#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include <doc.h>
#include <doc_json.h>
#include "config.h"
#include "strfmt.h"
#include "curses_extra.h"
#include "tui.h"
#include "simple_tcp_msg.h"
#include "net_discovery.h"
#include "log.h"

#include <doc.h>
#include <doc_json.h>

#include "config.h"
#include "strfmt.h"
#include "curses_extra.h"
#include "tui.h"
#include "simple_tcp_msg.h"

#include <plibsys.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* ----------------------------------------- Globals ------------------------------------------ */

pthread_t discovery_receiver_thread;

pthread_t discovery_transmitter_thread;

PSemaphore *discovery_ping_signal;

/* ----------------------------------------- Functions ---------------------------------------- */

// err and die
void static inline err_and_die(void);

// discovery_receiver thread daemon routine
void *discovery_receiver_thread_routine(void *data);

// discovery_transmitter thread daemon routine
void *discovery_transmitter_thread_routine(void *data);

/* ----------------------------------------- Main --------------------------------------------- */

// entry point main
int main(int argc, char **argv){


    // init routines
    p_libsys_init();
    config_init();

    // log file
    char buffer[500];
    snprintf(buffer, 500, "%s/%s", config_get_config_folder_path(), "log.txt");
    // log_set_output_file(buffer, "w+");

    // variable for plibsys errors
    PError *err = NULL;
    
    // unique name for semaphore
    srand((unsigned int)time(NULL));
    char sem_name[50] = {0};
    snprintf(sem_name, 50, "discovery_ping_signal_%i", rand());
    
    // create semaphore for discovery transmitter communication
    discovery_ping_signal = p_semaphore_new(sem_name, 0, P_SEM_ACCESS_OPEN, &err);
    if(discovery_ping_signal == NULL || err != NULL){
        log_error("Error while creating psemaphore\n");
        if(err != NULL){
            log_error("[plibsys] [%i] %s\n", p_error_get_code(err), p_error_get_message(err));
            p_error_free(err);
            err = NULL;
        }
        return -1;
    }

    // create thread for discovery listening
    if(pthread_create(&discovery_receiver_thread, NULL, &discovery_receiver_thread_routine, NULL) < 0){
        log_error("Discovery receiver thread creation failed\n");
        return -1;
    }

    // create thread for discovery sending
    if(pthread_create(&discovery_transmitter_thread, NULL, &discovery_transmitter_thread_routine, NULL) < 0){
        log_error("Discovery transmitter thread creation failed\n");
        return -1;
    }
    
    // // initialize curses
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

    // main loop
    while(1){
        
        // ----------------------- draw ------------------------
        // tui_draw();
        // update_panels();
        // refresh();

        // ----------------------- input -----------------------
        // int input = getch();
        fflush(stdin);
        int input = getchar();
        fflush(stdin);

        // tui_logic(input);

        switch(input){
            case 'q':
                {
                    err_and_die();

                    // close rfopen() of stderr by log.c
                    // works without it, lets see how long 
                    // fclose(stderr);
                    return 0;
                }
                break;

            case 'p':
                {
                    p_semaphore_release(discovery_ping_signal, NULL);
                }
                break;

            default:
                break;
        }

    }

    err_and_die();
    return 0;
}

/* ----------------------------------------- Functions ---------------------------------------- */

// err and die
void static inline err_and_die(void){
    endwin();
    // pthread_join(discovery_receiver_thread, NULL);
    pthread_cancel(discovery_receiver_thread);
    pthread_cancel(discovery_transmitter_thread);
    p_semaphore_take_ownership(discovery_ping_signal);
    p_semaphore_release(discovery_ping_signal, NULL);
    config_save();
    p_libsys_shutdown();
}

// discovery_receiver thread daemon routine
void *discovery_receiver_thread_routine(void *data){

    discovery_receiver_t *discovery_receiver = discovery_receiver_init();
    
    if(discovery_receiver == NULL){
        log_error("discovery_Receiver init failed\n");
        return NULL;
    }

    while(1){
        discovery_receiver_listen(discovery_receiver);
    }

    discovery_receiver_delete(discovery_receiver);

    return NULL;
}

// discovery_transmitter thread routine
void *discovery_transmitter_thread_routine(void *data){

    discovery_transmitter_t *transmitter = discovery_transmitter_init();

    while(1){
        p_semaphore_acquire(discovery_ping_signal, NULL);
        discovery_transmitter_ping(transmitter);
    }

    discovery_transmitter_delete(transmitter);

    return NULL;
}
