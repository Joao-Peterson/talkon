#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "c_doc/doc.h"
#include "c_doc/doc_json.h"
#include "inc/config.h"
#include "inc/strfmt.h"
#include "inc/curses_extra.h"
#include "inc/tui.h"
#include "inc/simple_tcp_msg.h"
#include "stcp/stcp.h"

void stcp_error_cb(stcp_error err, void *data){
    const char *msg = NULL; 
    
    switch(err){
        case STCP_UNKNOWN_ERROR:
            msg = "STCP_UNKNOWN_ERROR";
            break;                
                        
        case STCP_FUNCTION_INTERRUPTED:
            msg = "STCP_FUNCTION_INTERRUPTED";
            break;         
                
        case STCP_INVALID_FILE_HANDLE:
            msg = "STCP_INVALID_FILE_HANDLE";
            break;          
                
        case STCP_PERMISSION_DENIED:
            msg = "STCP_PERMISSION_DENIED";
            break;            
                    
        case STCP_INVALID_POINTER:
            msg = "STCP_INVALID_POINTER";
            break;              
                    
        case STCP_INVALID_ARGUMENT:
            msg = "STCP_INVALID_ARGUMENT";
            break;             
                    
        case STCP_TOO_MANY_SOCKETS:
            msg = "STCP_TOO_MANY_SOCKETS";
            break;             
                    
        case STCP_RESOURCE_UNAVAILABLE:
            msg = "STCP_RESOURCE_UNAVAILABLE";
            break;         
                
        case STCP_BLOCKED_BY_CALLER:
            msg = "STCP_BLOCKED_BY_CALLER";
            break;            
                    
        case STCP_BLOCKED_BY_CALLEE:
            msg = "STCP_BLOCKED_BY_CALLEE";
            break;            
                    
        case STCP_INVALID_SOCKET:
            msg = "STCP_INVALID_SOCKET";
            break;               
                    
        case STCP_INVALID_DESTINATION:
            msg = "STCP_INVALID_DESTINATION";
            break;          
                
        case STCP_MESSAGE_TOO_LONG:
            msg = "STCP_MESSAGE_TOO_LONG";
            break;             
                    
        case STCP_INVALID_PROTOCOL:
            msg = "STCP_INVALID_PROTOCOL";
            break;             
                    
        case STCP_INVALID_PROTOCOL_OPTION:
            msg = "STCP_INVALID_PROTOCOL_OPTION";
            break;      
            
        case STCP_PROTOCOL_NOT_SUPPORTED:
            msg = "STCP_PROTOCOL_NOT_SUPPORTED";
            break;       
            
        case STCP_SOCKET_NOT_SUPPORTED:
            msg = "STCP_SOCKET_NOT_SUPPORTED";
            break;         
                
        case STCP_OPERATION_NOT_SUPPORTED:
            msg = "STCP_OPERATION_NOT_SUPPORTED";
            break;      
            
        case STCP_PROTOCOL_FAMILY_NOT_SUPPORTED:
            msg = "STCP_PROTOCOL_FAMILY_NOT_SUPPORTED";
            break;
        
        case STCP_ADDRESS_FAMILY_NOT_SUPPORTED:
            msg = "STCP_ADDRESS_FAMILY_NOT_SUPPORTED";
            break; 
        
        case STCP_ADDRESS_IN_USE:
            msg = "STCP_ADDRESS_IN_USE";
            break;               
                    
        case STCP_ADDRESS_NOT_AVAILABLE:
            msg = "STCP_ADDRESS_NOT_AVAILABLE";
            break;        
                
        case STCP_NETWORK_DOWN:
            msg = "STCP_NETWORK_DOWN";
            break;                 
                        
        case STCP_NETWORK_UNREACHABLE:
            msg = "STCP_NETWORK_UNREACHABLE";
            break;          
                
        case STCP_NETWORK_RESET:
            msg = "STCP_NETWORK_RESET";
            break;                
                        
        case STCP_CONNECTION_ABORTED:
            msg = "STCP_CONNECTION_ABORTED";
            break;           
                
        case STCP_CONNECTION_RESET:
            msg = "STCP_CONNECTION_RESET";
            break;             
                    
        case STCP_NO_BUFFER_SPACE:
            msg = "STCP_NO_BUFFER_SPACE";
            break;              
                    
        case STCP_SOCKET_ALREADY_CONNECTED:
            msg = "STCP_SOCKET_ALREADY_CONNECTED";
            break;     
            
        case STCP_SOCKET_NOT_CONNECTED:
            msg = "STCP_SOCKET_NOT_CONNECTED";
            break;         
                
        case STCP_SOCKET_SHUTDOWN:
            msg = "STCP_SOCKET_SHUTDOWN";
            break;              
                    
        case STCP_TOO_MANY_REFERENCES:
            msg = "STCP_TOO_MANY_REFERENCES";
            break;          
                
        case STCP_CONNECTION_TIMED_OUT:
            // msg = "STCP_CONNECTION_TIMED_OUT";
            break;         
                
        case STCP_CONNECTION_REFUSED:
            msg = "STCP_CONNECTION_REFUSED";
            break;           
                
        case STCP_INVALID_NAME:
            msg = "STCP_INVALID_NAME";
            break;                 
                        
        case STCP_NAME_TOO_LONG:
            msg = "STCP_NAME_TOO_LONG";
            break;                
                        
        case STCP_HOST_DOWN:
            msg = "STCP_HOST_DOWN";
            break;                    
                            
        case STCP_HOST_UNREACHABLE:
            msg = "STCP_HOST_UNREACHABLE";
            break;             
                    
        case STCP_DIRECTORY_NOT_EMPTY:
            msg = "STCP_DIRECTORY_NOT_EMPTY";
            break;          
                
        case STCP_TOO_MANY_PROCESSES:
            msg = "STCP_TOO_MANY_PROCESSES";
            break;           
                
        case STCP_TOO_MANY_USERS:
            msg = "STCP_TOO_MANY_USERS";
            break;               
                    
        case STCP_OUT_OF_DISK_SPACE:
            msg = "STCP_OUT_OF_DISK_SPACE";
            break;            
                    
        case STCP_OLD_HANDLE:
            msg = "STCP_OLD_HANDLE";
            break;                   
                        
        case STCP_RESOURCE_NOT_LOCAL:
            msg = "STCP_RESOURCE_NOT_LOCAL";
            break;  

        default:
            msg = "Unknow STCP error!";         
    }

    if(msg != NULL)
        printf("%s\n", msg);        
}

int main(int argc, char **argv){

    // if(!stcp_initialize()){
    //     printf("Error init stcp\n");
    //     exit(-1);
    // }

    // stcp_set_error_callback(&stcp_error_cb, NULL);

    // stcp_address *server_addr = stcp_create_address_ipv4("192.168.0.150", 5003);
    // stcp_server *server = stcp_create_server(server_addr, 10);

    // if(server == NULL){
    //     printf("Error in server init\n");
    //     exit(-1);
    // }

    // // 
    // while(1){

    //     stcp_channel *client_in = stcp_accept_channel(server, 100);

    //     // respond to client
    //     if(client_in != NULL){

    //         char buffer[10000];

    //         while(1){
    //             stcp_receive(client_in, buffer, 10000, 100);

    //             if(buffer[0] != '{'){

    //                 continue;        
    //             }
    //             else{
                    
    //                 doc *packet_in = doc_json_parse(buffer);

    //                 int msg_type = doc_get(packet_in, "type", int);

    //                 switch(msg_type){
    //                     case msg_type_ping:
    //                         doc *res = doc_new(
    //                         "res", dt_obj,
    //                             "type", dt_int32, msg_type_ping,
    //                             "info", dt_obj, 
    //                                 "name", dt_const_string, "talkon_client_#1", 18ULL,
    //                             ";", 
    //                         ";");

    //                         char *res_str = doc_json_stringify(res);

    //                         stcp_send(client_in, res_str, strlen(res_str), 100);

    //                         free(res_str);
    //                         doc_delete(res, ".");
    //                         break;

    //                     // error type
    //                     case msg_type_message:
    //                     case msg_type_status:
    //                     case 0:
    //                     default:
    //                         break;
    //                 }  

    //                 doc_delete(packet_in, ".");
                    
    //                 break;
    //             }
    //         }

    //         stcp_free_channel(client_in);
    //     }
    // }

    // stcp_free_address(server_addr);
    // stcp_free_server(server);
	// stcp_terminate();

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

    config_init();
    config_save();
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