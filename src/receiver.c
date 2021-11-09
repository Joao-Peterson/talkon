#include "receiver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <doc.h>
#include <doc_json.h>

#include "config.h"
#include "simple_tcp_msg.h"
#include "log.h"

/* ----------------------------------------- Defines ------------------------------------------ */

#define buffer_read_size 1024

typedef struct{
	PSocketFamily	family;
	PSocketProtocol	protocol;
	PSocketType	type;
	pint		fd;
	pint		listen_backlog;
	pint		timeout;
	puint		blocking	: 1;
	puint		keepalive	: 1;
	puint		closed		: 1;
	puint		connected	: 1;
	puint		listening	: 1;
#ifdef P_OS_WIN
	WSAEVENT	events;
#endif
#ifdef P_OS_SCO
	PTimeProfiler	*timer;
#endif
}PSocket_receiver_t;

/* ----------------------------------------- Private functions -------------------------------- */

P_LIB_API void p_socket_set_broadcast(PSocket *socket, pboolean set_broadcast){

    PSocket_receiver_t *PSocket_receiver = (PSocket_receiver_t*)socket;
    
#ifdef P_OS_WIN
	pchar value;
#else
	pint value;
#endif

	if (P_UNLIKELY (socket == NULL))
		return;

#ifdef P_OS_WIN
	value = !! (pchar) set_broadcast;
#else
	value = !! (pint) set_broadcast;
#endif
	if (setsockopt (PSocket_receiver->fd, SOL_SOCKET, SO_BROADCAST, &value, sizeof (value)) < 0) {
		log_error("PSocket::p_socket_set_broadcast: setsockopt() with SO_BROADCAST failed");
		return;
	}
}

/* ----------------------------------------- Functions ---------------------------------------- */

// initialize receiver
receiver_t *receiver_init(void){

    receiver_t *receiver = calloc(1, sizeof(*receiver));

    if((receiver->server_addr = p_socket_address_new("127.0.0.1", config_get("udp-port", int))) == NULL){
        log_error("Error creating address for localhost\n");
        return NULL;
    }

    if((receiver->server = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP, NULL)) == NULL){
        log_error("error in creating socket on udp port [%d]\n", config_get("udp-port", int));
        return NULL;
    }

    if((p_socket_bind(receiver->server, receiver->server_addr, true, NULL)) == false){
        log_error("error binding socket on udp port [%d] to localhost\n", config_get("udp-port", int));
        return NULL;
    }

    p_socket_set_broadcast(receiver->server, true);
    
    return receiver;
}

// ends the receiver, cleaning up memory
void receiver_delete(receiver_t *receiver){
    p_socket_address_free(receiver->server_addr);
    p_socket_free(receiver->server);
}

// responds to the message received  
void receiver_respond(PSocket *client, char *data){
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
                    // psocket(client, res_str, strlen(res_str), timeout);
                }
                else{
                    // stcp_send(client, res_str, strlen(res_str), timeout);
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
            {
                log_error("type received (%i) is not a defined type\n", msg_type);

                char buffer[500];
                snprintf(buffer, 500, "type received (%i) is not a defined type\n", msg_type);
                
                // stcp_send(client, buffer, strlen(buffer), timeout);
            }
            break;
    }  

    doc_delete(packet_in, ".");
}

// receiver function that listens on network and respond accordingly
void *receiver_listen(void *data){
    
    receiver_t *receiver = (receiver_t*)data;

    // respond to client
    int received_msg_size = 1;
    char *received_msg = calloc(received_msg_size, sizeof(char));

    char buffer[buffer_read_size];
    int received_bytes;

    // receive until receive d bytes are less than buffer size
    do{
        received_bytes = p_socket_receive(receiver->server, buffer, buffer_read_size, NULL);
        
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
        log("Broadcast: %s.\n", received_msg);
        // receiver_respond(client_in, received_msg);
        free(received_msg);
    }
}
