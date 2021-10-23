#include "receiver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "stcp.h"
#include "simple_tcp_msg.h"
#include "log.h"
#include "doc.h"
#include "doc_json.h"

/* ----------------------------------------- Defines ------------------------------------------ */

#define buffer_read_size 1024
#define timeout 100

/* ----------------------------------------- Functions ---------------------------------------- */

// callback for stcp to call when a error occurs
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
        log("STCP error: \"%s\"\n", msg);        
}

// initialize receiver
receiver_t *receiver_init(void){

    if(!stcp_initialize()){
        log_error("Error initializing stcp\n");
        return NULL;
    }

    stcp_set_error_callback(&stcp_error_cb, NULL);

    receiver_t *receiver = calloc(1, sizeof(*receiver));

    // stcp_address *server_addr = stcp_create_address_ipv4("192.168.0.150", 5003);
    receiver->server_addr = stcp_create_address_hostname("localhost", config_get("port", int));

    if(receiver->server_addr == NULL){
        log_error("Error creating address for localhost\n");
        return NULL;
    }

    receiver->server = stcp_create_server(receiver->server_addr, 10);

    if(receiver->server == NULL){
        log_error("Error in server initialization on port [%d]\n", config_get("port", int));
        return NULL;
    }

    return receiver;
}

// ends the receiver, cleaning up memory
void receiver_delete(receiver_t *receiver){
    stcp_free_address(receiver->server_addr);
    stcp_free_server(receiver->server);
    stcp_terminate();
}

// responds to the message received  
void receiver_respond(stcp_channel *client, char *data){
    doc *packet_in = doc_json_parse(data);

    if(packet_in == NULL){
        log_error("Data received is not a json format\n");
        log_error("\"%s\"\n", data);
        return;
    }

    int msg_type = doc_get(packet_in, "type", int);

    if(doc_error_code != errno_doc_ok){
        log_error("error on doc json, received json is invalid or malformed: %s", doc_get_error_msg());
        return;
    }

    switch(msg_type){
        case msg_type_ping:
            {
                char *name = config_get("name", char*);
                
                doc *res = doc_new(
                "res", dt_obj,
                    "type", dt_int32, msg_type_info,
                    "info", dt_obj, 
                        "name", dt_const_string, name, (size_t)(strlen(name) + 1),
                    ";", 
                ";");

                char *res_str = doc_json_stringify(res);

                if(res_str == NULL){
                    res_str = "{\"type\": 0}";
                    stcp_send(client, res_str, strlen(res_str), timeout);
                }
                else{
                    stcp_send(client, res_str, strlen(res_str), timeout);
                    free(res_str);
                }

                doc_delete(res, ".");
            }
            break;

        // error type
        case msg_type_message:
        case msg_type_status:
        case 0:
        default:
            break;
    }  

    doc_delete(packet_in, ".");
}

// receiver function that listens on network and respond accordingly
void *receiver_listen(void *data){
    
    receiver_t *receiver = receiver_init();
    if(receiver == NULL){
        log_error("Receiver init failed\n");
        return NULL;
    }

    while(1){

        stcp_channel *client_in = stcp_accept_channel(receiver->server, timeout);

        // respond to client
        if(client_in != NULL){

            int received_msg_size = 1;
            char *received_msg = calloc(received_msg_size, sizeof(char));

            char buffer[buffer_read_size];
            int received_bytes;

            // receive until receive d bytes are less than buffer size
            do{
                received_bytes = stcp_receive(client_in, buffer, buffer_read_size, timeout);
                
                if(received_bytes == 0){
                    // log_error("stcp_receive() error\n");
                    // free(received_msg);
                    // received_msg == NULL;
                    continue;
                }

                received_msg_size += received_bytes;
                received_msg = realloc(received_msg, received_msg_size);
                strcat(received_msg, buffer);

            }while(received_bytes == buffer_read_size);

            if(received_msg != NULL){
                receiver_respond(client_in, received_msg);
                free(received_msg);
            }

            stcp_free_channel(client_in);
        }
    }
}
