#include "receiver.h"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <doc.h>
#include <doc_json.h>

#include "config.h"
#include "simple_tcp_msg.h"
#include "log.h"

#include <psocket.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// IP_MULTICAST_TTL 
// https://docs.oracle.com/cd/E23824_01/html/821-1602/sockets-137.html
// Multicast options
// https://tldp.org/HOWTO/Multicast-HOWTO-6.html
// IP optioons
// https://www.man7.org/linux/man-pages/man7/ip.7.html
// plibsys sockets
// https://saprykin.github.io/plibsys-docs/psocket_8h.html

/* ----------------------------------------- Defines ------------------------------------------ */

#define buffer_read_size 1024

/* ----------------------------------------- Functions ---------------------------------------- */

// initialize receiver
receiver_t *receiver_init(void){

    receiver_t *receiver = calloc(1, sizeof(*receiver));

    int udp_port = config_get("udp_port", int);
    int port_retry_num = config_get("port_retry_num", int);

    PError *err;

    for(int retry = 0; retry < port_retry_num; retry++){

        // make sure to free if faile don retry
        if(receiver->address != NULL)
            p_socket_address_free(receiver->address);

        if(receiver->socket != NULL)
            p_socket_free(receiver->socket);

        // create addres struct
        if((receiver->address = p_socket_address_new_any(P_SOCKET_FAMILY_INET, retry+udp_port)) == NULL){
            if(retry == port_retry_num){
                log_error("Error creating address for localhost\n");
                return NULL;
            }
            else{
                log_error("Error creating address for localhost in port [%i] - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }

        // create socket on ip and port
        receiver->socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP, &err);
        if(receiver->socket == NULL || err != NULL){
            if(err != NULL){
                log_error("[plibsys] [%i] %s\n", p_error_get_code(err), p_error_get_message(err));
                p_error_free(err);
                err = NULL;
            }
            
            if(retry == port_retry_num){
                log_error("Error in creating socket on udp port [%i]\n", retry+udp_port);
                return NULL;
            }
            else{
                log_error("Error in creating socket on udp port [%i] - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }

        // bind
        pboolean res = p_socket_bind(receiver->socket, receiver->address, false, &err);
        if(res == false || err != NULL){
            if(err != NULL){
                log_error("[plibsys] [%i] %s\n", p_error_get_code(err), p_error_get_message(err));
                p_error_free(err);
                err = NULL;
            }

            if(retry == port_retry_num){
                log_error("Error binding socket on udp port [%d] to localhost\n", retry+udp_port);
                return NULL;
            }
            else{
                log_error("Error binding socket on udp port [%d] to localhost - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }
        
        break;
    }

    // register on multicast group
    struct ip_mreq multicast_req;
    char *udp_group = config_get("udp_group", char*);

    // make multicast req to the kernel asking to join the multicast group 
    inet_aton(udp_group, &multicast_req.imr_multiaddr);
    multicast_req.imr_interface.s_addr = htonl(INADDR_ANY);

    // register the request
    if((setsockopt(p_socket_get_fd(receiver->socket), IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_req , sizeof(multicast_req))) < 0){
        log_error("Error setting up the udp multicast group join request\n");
        p_socket_address_free(receiver->address);
        p_socket_free(receiver->socket);
        return NULL;
    }

    // enable loopback
    int loopback = 1;
    if((setsockopt(p_socket_get_fd(receiver->socket), IPPROTO_IP, IP_MULTICAST_LOOP, &loopback , sizeof(loopback))) < 0){
        log_error("Error setting up the udp multicast loopback option\n");
        p_socket_address_free(receiver->address);
        p_socket_free(receiver->socket);
        return NULL;
    }

    log("UDP multicast listener opened on interface %s:%i\n", p_socket_address_get_address(receiver->address), p_socket_address_get_port(receiver->address));
    
    return receiver;
}

// ends the receiver, cleaning up memory
void receiver_delete(receiver_t *receiver){
    p_socket_address_free(receiver->address);
    p_socket_free(receiver->socket);
}

// responds to the message received  
void receiver_respond(PSocket *client, PSocketAddress *remote_address, char *data){
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
                    p_socket_send_to(client, remote_address, res_str, strlen(res_str), NULL);
                }
                else{
                    p_socket_send_to(client, remote_address, res_str, strlen(res_str), NULL);
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

                p_socket_send_to(client, remote_address, buffer, strlen(buffer), NULL);
            }
            break;
    }  

    doc_delete(packet_in, ".");
}

// receiver function that listens on network and respond accordingly
void *receiver_listen(void *data){
    
    receiver_t *receiver = (receiver_t*)data;

    int received_msg_size = 1;
    char *received_msg = calloc(received_msg_size, sizeof(char));

    char buffer[buffer_read_size];
    int received_bytes;

    PSocketAddress *remote_address = NULL;

    // receive until receive d bytes are less than buffer size
    do{
        if(remote_address != NULL)
            p_socket_address_free(remote_address);

        received_bytes = p_socket_receive_from(receiver->socket, &remote_address, buffer, buffer_read_size, NULL);

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
        log("Multicast from (%s:%i): %s.\n", 
            p_socket_address_get_address(remote_address), 
            p_socket_address_get_port(remote_address), 
            received_msg
        );
        receiver_respond(receiver->socket, remote_address, received_msg);
        
        if(remote_address != NULL)
            p_socket_address_free(remote_address);

        free(received_msg);
    }

    return NULL;
}
