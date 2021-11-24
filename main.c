#include <netinet/in.h>
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
#include <sys/socket.h>

/* ----------------------------------------- Struct's ----------------------------------------- */

// data type for interfacing with the receiver thread
typedef struct{
    int run_flag;
}receiver_thread_interface_t;

/* ----------------------------------------- Globals ------------------------------------------ */

receiver_thread_interface_t receiver_thread_interface = {
    .run_flag = 1
};

pthread_t receiver_thread;

/* ----------------------------------------- Functions ---------------------------------------- */

// receiver thread daemon routine
void *receiver_thread_routine(void *data){
    receiver_thread_interface_t *interface = &receiver_thread_interface;

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

    return NULL;
}

// err and die
void static inline err_and_die(void){
    endwin();
    receiver_thread_interface.run_flag = 0;
    pthread_join(receiver_thread, NULL);
    config_save();
    p_libsys_shutdown();
}

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

    // create threads for messaging
    if(pthread_create(&receiver_thread, NULL, &receiver_thread_routine, NULL) < 0){
        log_error("Receiver thread creation failed\n");
        return -1;
    }
    
    // initialize curses
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
        
        // ----------------------- daemons ---------------------

        if(receiver_thread_interface.run_flag == 0){
            log_error("An error was occured while starting the TCP comunication.\n");
            log_error("Please open your log file in: \"%s\"\nfor more details.\n", config_get_config_folder_path());
            err_and_die();
            return 0;
        }
        
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
                    PSocketAddress *addr = p_socket_address_new_any(P_SOCKET_FAMILY_INET, 5005);                    
                    PSocket *socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP, NULL);
                    p_socket_bind(socket, addr, 0, NULL);

                    // int flag = 1;
                    // setsockopt(((PSocket_receiver_t*)socket)->fd, IPPROTO_IP, IP_MULTICAST_TTL, (void*)(&flag), sizeof(flag));

                    // PSocketAddress *multicast = p_socket_address_new(config_get("udp_group", char*), 5001);
                    PSocketAddress *multicast = p_socket_address_new("127.0.0.1", 5001);
                    psize res = p_socket_send_to(socket, multicast, "MULTICAST WORKSSSSS!!", 22ULL, NULL);
                    log_debug("sended: [%i]\n", res);
                    p_socket_free(socket);
                    p_socket_address_free(addr);
                    p_socket_address_free(multicast);
                }
                break;

            default:
                break;
        }

    }

    err_and_die();
    return 0;
}