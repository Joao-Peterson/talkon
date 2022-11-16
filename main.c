#include <curses.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>
#include <plibsys.h>
#include <doc.h>
#include <doc_print.h>
#include <doc_json.h>

#include "db.h"
#include "config.h"
#include "strfmt.h"
#include "curses_extra.h"
#include "tui.h"
#include "simple_tcp_msg.h"
#include "net_discovery.h"
#include "log.h"

/* ----------------------------------------- Globals ------------------------------------------ */

PRWLock *db_lock;

db_t *db;

/* ----------------------------------------- Functions ---------------------------------------- */

// err and die
void static inline err_and_die(void);

/* ----------------------------------------- Main --------------------------------------------- */

// entry point main
int main(int argq, char **argv){

    // ! TEMP
    if(argq < 2){
        printf("Pass the profile number.\n");
        return -1;
    }

    uint32_t profile_id = atoi(argv[1]);
    // ! TEMP

    // init routines
    p_libsys_init();
    config_init(profile_id);

    // log file
    char buffer[500];
    snprintf(buffer, 500, "%s/%s.%s", config_get_config_folder_path(), profile_get("uuid", char*), "log");
    log_set_output_file(buffer, "w+");

    db = db_init();
    if(db == NULL){
        log_error("error initializing sqlite database.\n");
        return -1;
    }

    // greetings!
    // log("logged as \"%s\" uuid: \"%s\", welcome!.\n", profile_get("name", char*), profile_get("uuid", char*));

    // variable for plibsys errors
    PError *err = NULL;

    // initalize lock for database read/write
    db_lock = p_rwlock_new();
    if(db_lock == NULL){
        log_error("error initializing sqlite database rwlock.\n");
        return -1;
    }

    // discovery receiver
    uint32_t receiver_udp_port = (uint32_t)config_get("discovery.port_udp_listen", int);
    uint32_t receiver_port_retry_num = (uint32_t)config_get("discovery.port_retry_num", int);

    discovery_receiver_t *receiver = discovery_receiver_init(
        receiver_udp_port, receiver_port_retry_num
    );

    // discovery trasmitter
    uint32_t transmitter_udp_port = config_get("discovery.port_udp_send", int);
    uint32_t transmitter_port_retry_num = config_get("discovery.port_retry_num", int);
    uint32_t transmitter_timeout_ms = config_get("discovery.timeout_ms", int);
    uint32_t transmitter_port_remote_min = config_get("discovery.port_udp_discovery_range[0]", int);
    uint32_t transmitter_port_remote_max = config_get("discovery.port_udp_discovery_range[1]", int);
    char *transmitter_multicast_group = config_get("discovery.address_udp_multicast_group", char*);

    discovery_transmitter_t *transmitter = discovery_transmitter_init(
        transmitter_udp_port, transmitter_port_retry_num, transmitter_timeout_ms,
        transmitter_port_remote_max, transmitter_port_remote_min,
        transmitter_multicast_group 
    );

    // initialize curses
    initscr();
    keypad(stdscr, true);
    // cbreak();
    timeout(0);
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

    // fetch nodes on startup
    if(tui.nodes != NULL){
        doc_delete(tui.nodes, ".");
        tui.nodes = NULL;
    }    
    if(db_select_nodes(db, &tui.nodes)){
        log_error("couldn't fetch nodes from sqlite database.\n");
    }

    // first render, dit it three times to update all things correctly
    for(int i = 0; i < 3; i++){
        tui_draw(tui_layer_base);
        update_panels();
        refresh();
    }

    // main loop
    while(1){
        
        // ----------------------- treads com. -----------------

        int ping_done = false;
        if(p_rwlock_reader_trylock(discovery_thread_flags_lock)){
            ping_done = discovery_thread_flags.done;
            discovery_thread_flags.done = false;
            p_rwlock_reader_unlock(discovery_thread_flags_lock);
        }
        
        if(ping_done){
            log_debug("ping done.\n");

            if(tui.nodes != NULL){
                doc_delete(tui.nodes, ".");
                tui.nodes = NULL;
            }

            tui.ping_icon_show = false;

            tui_draw(tui_layer_base);
            update_panels();
            refresh();

            if(db_select_nodes(db, &tui.nodes)){
                log_error("couldn't fetch nodes from sqlite database.\n");
            }
            else{
                // doc_print(tui.nodes);
            }
        }

        // ----------------------- input -----------------------
        int input = getch();
        // fflush(stdin);
        // int input = getchar();
        // fflush(stdin);

        switch(tui.cur_sel_win){
            case window_id_main:
                switch(input){
                    case 'q':
                            err_and_die();
                            log_close_out(); // close rfopen() of stderr by log.c
                            return 0;
                    break;
                }
            break;
            
            case window_id_nav:
                switch(input){
                    case 'q':
                            err_and_die();
                            log_close_out(); // close rfopen() of stderr by log.c
                            return 0;
                    break;
                }

            break;
            
            case window_id_nodes:
                switch(input){
                    case '\t':
                        tui.cur_sel_win = window_id_talk;
                        break;
                        
                    case 'q':
                            err_and_die();
                            log_close_out(); // close rfopen() of stderr by log.c
                            return 0;
                    break;

                    case 'p':
                    case 'r':
                        log_debug("sending ping signal.\n");
                        tui.ping_icon_show = true;
                        p_rwlock_writer_lock(discovery_thread_flags_lock);
                        discovery_thread_flags.ping = true;
                        p_rwlock_writer_unlock(discovery_thread_flags_lock);
                    break;

                    case KEY_DOWN:
                        tui.cur_sel_node++; // scroll overflow is handled on window draw, because of dynamic dimensions
                    break;

                    case KEY_UP:
                        tui.cur_sel_node--; // scroll underflow is handled on window draw, because of dynamic dimensions
                    break;
                }

            break;
            
            case window_id_talk:
                switch(input){
                    case '\t':
                        tui.cur_sel_win = window_id_input;
                        break;

                    case 'q':
                            err_and_die();
                            log_close_out(); // close rfopen() of stderr by log.c
                            return 0;
                    break;
                }

            break;
            
            case window_id_input:
                switch(input){
                    // change window
                    case '\t':
                        tui.cur_sel_win = window_id_nodes;
                        break;

                    // enter to send message
                    case KEY_ENTER:
                        
                        break;

                    // clear message
                    case KEY_BACKSPACE:
                        {
                            if(tui.input_buffer_size == 1){
                                tui.input_buffer[tui.input_buffer_size] = '\0';
                                tui.input_buffer[tui.input_buffer_size - 1] = '\0';
                                tui.input_buffer_size--;
                            }
                            else if(tui.input_buffer_size > 0){
                                tui.input_buffer[tui.input_buffer_size - 1] = input_cursor_char;
                                tui.input_buffer[tui.input_buffer_size] = '\0';
                                tui.input_buffer_size--;
                            }
                        }
                        break;

                    // input characters
                    default:
                        {
                            if(isprint(input) && (tui.input_buffer_size < input_max_len)){
                                char input_str[3];
                                input_str[0] = input;
                                input_str[1] = input_cursor_char;
                                input_str[2] = '\0';

                                if(tui.input_buffer_size > 0)
                                    tui.input_buffer[tui.input_buffer_size] = '\0';

                                strncat(tui.input_buffer, input_str, input_max_len);

                                tui.input_buffer_size++;
                            }
                        }
                        break;

                    // case 'q':
                    //         err_and_die();
                    //         log_close_out(); // close rfopen() of stderr by log.c
                    //         return 0;
                    // break;
                }

            break;
            
            default:
                switch(input){
                    case 'q':
                            err_and_die();
                            log_close_out(); // close rfopen() of stderr by log.c
                            return 0;
                    break;
                }
            break;
        }

        // ----------------------- draw ------------------------
        
        switch(input){
            // update animations every time, on getchr timeout 
            case ERR:
                tui_draw(tui_layer_animations);
                tui_draw(tui_layer_text);

                // when an animation or popup disappers, redraw all windows
                if(tui.ping_icon_show == 0 && last_ping_state == 1)
                    tui_draw(tui_layer_base);
                    
                last_ping_state = tui.ping_icon_show;

                update_panels();
                refresh();
                
                break;
            
            // update all layers in case of input commands, exluding text input
            default:
                {
                    static window_id_t last_window = first_sel_win;

                    /**
                    * render all windows normally (everytime) regardless of current selected window, 
                    * except when the input window is selected, we render all windows when it's first selected.
                    * This is done to minimize rendering on every keystroke when typing on the input window. 
                    */
                    if( 
                        tui.cur_sel_win != window_id_input ||
                        (tui.cur_sel_win != last_window && tui.cur_sel_win == window_id_input)
                    ){
                        tui_draw(tui_layer_base);
                        update_panels();
                        refresh();
                    }

                    last_window = tui.cur_sel_win;
                }
                break;
        }
        
    }

    err_and_die();
    return 0;
}

/* ----------------------------------------- Functions ---------------------------------------- */

// err and die
void static inline err_and_die(void){
    log("shutting down program normally.\n");

    endwin();
    
    discovery_transmitter_delete(transmitter);
    discovery_receiver_delete(receiver);
    db_delete(db);

    tui_end();

    config_save();
    config_end();
    p_libsys_shutdown();
}
