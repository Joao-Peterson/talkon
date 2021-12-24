#include "net_discovery.h"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <doc.h>
#include <doc_json.h>

#include "config.h"
#include "simple_tcp_msg.h"
#include "log.h"
#include "inet_extra.h"

#include <plibsys.h>
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

// Multi cast example
// https://github.com/NakedSolidSnake/Raspberry_IPC_Socket_UDP_Multicast/blob/master/lib/udp_multicast_receiver.c
// https://github.com/NakedSolidSnake/Raspberry_IPC_Socket_UDP_Multicast/blob/master/lib/udp_multicast_sender.c

/* ----------------------------------------- Defines ------------------------------------------ */

#define buffer_read_size 1024

/* ----------------------------------------- Functions ---------------------------------------- */

// initialize discovery_receiver
discovery_receiver_t *discovery_receiver_init(void){

    discovery_receiver_t *discovery_receiver = calloc(1, sizeof(*discovery_receiver));

    int udp_port = config_get("discovery.port_udp_listen", int);
    int port_retry_num = config_get("discovery.port_retry_num", int);

    PError *err;

    for(int retry = 0; retry < port_retry_num; retry++){

        // make sure to free if faile don retry
        if(discovery_receiver->address != NULL)
            p_socket_address_free(discovery_receiver->address);

        if(discovery_receiver->socket != NULL)
            p_socket_free(discovery_receiver->socket);

        // create addres struct
        if((discovery_receiver->address = p_socket_address_new_any(P_SOCKET_FAMILY_INET, retry+udp_port)) == NULL){
            if(retry == port_retry_num){
                log_error("Error creating address for localhost on discovery_receiver\n");
                return NULL;
            }
            else{
                log_error("Error creating address for localhost on discovery_receiver in port [%i] - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }

        // create socket on ip and port
        discovery_receiver->socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP, &err);
        if(discovery_receiver->socket == NULL || err != NULL){
            if(err != NULL){
                log_error("[plibsys] [%i] %s\n", p_error_get_code(err), p_error_get_message(err));
                p_error_free(err);
                err = NULL;
            }
            
            if(retry == port_retry_num){
                log_error("Error in creating socket in discovery_receiver on udp port [%i]\n", retry+udp_port);
                return NULL;
            }
            else{
                log_error("Error in creating socket in discovery_receiver on udp port [%i] - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }

        // bind
        pboolean res = p_socket_bind(discovery_receiver->socket, discovery_receiver->address, false, &err);
        if(res == false || err != NULL){
            if(err != NULL){
                log_error("[plibsys] [%i] %s\n", p_error_get_code(err), p_error_get_message(err));
                p_error_free(err);
                err = NULL;
            }

            if(retry == port_retry_num){
                log_error("Error binding socket in discovery_receiver on udp port [%d] to localhost\n", retry+udp_port);
                return NULL;
            }
            else{
                log_error("Error binding socket in discovery_receiver on udp port [%d] to localhost - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }
        
        break;
    }

    // register on multicast group
    struct ip_mreq multicast_req;
    char *udp_group = config_get("discovery.address_udp_multicast_group", char*);

    // make multicast req to the kernel asking to join the multicast group 
    inet_aton(udp_group, &multicast_req.imr_multiaddr);
    multicast_req.imr_interface.s_addr = htonl(INADDR_ANY);

    // register the request
    if((setsockopt(p_socket_get_fd(discovery_receiver->socket), IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_req , sizeof(multicast_req))) < 0){
        log_error("Error setting up the udp multicast group join request in discovery_receiver\n");
        p_socket_address_free(discovery_receiver->address);
        p_socket_free(discovery_receiver->socket);
        return NULL;
    }

    // enable loopback
    int loopback = 1;
    if((setsockopt(p_socket_get_fd(discovery_receiver->socket), IPPROTO_IP, IP_MULTICAST_LOOP, &loopback , sizeof(loopback))) < 0){
        log_error("Error setting up the udp multicast loopback option in discovery_receiver\n");
        p_socket_address_free(discovery_receiver->address);
        p_socket_free(discovery_receiver->socket);
        return NULL;
    }

    log("UDP multicast listener opened in discovery_receiver on interface %s:%i\n", p_socket_address_get_address(discovery_receiver->address), p_socket_address_get_port(discovery_receiver->address));
    
    return discovery_receiver;
}

// ends the discovery_receiver, cleaning up memory
void discovery_receiver_delete(discovery_receiver_t *discovery_receiver){
    p_socket_address_free(discovery_receiver->address);
    p_socket_free(discovery_receiver->socket);
}

// responds to the message received  
void discovery_receiver_respond(PSocket *client, PSocketAddress *remote_address, char *data){
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
                doc *info = doc_copy(profile_doc, ".");
                doc_rename(info, ".", "info");
                
                doc *res = doc_new(
                "res", dt_obj,
                    "type", dt_int32, msg_type_info,
                ";");

                doc_append(res, ".", info);

                char *res_str = doc_json_stringify(res);

                if(res_str == NULL){
                    res_str = "{\"type\": 0}";
                    p_socket_send_to(client, remote_address, res_str, strlen(res_str) + 1, NULL);
                }
                else{
                    p_socket_send_to(client, remote_address, res_str, strlen(res_str) + 1, NULL);
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

// discovery_receiver function that listens on network and respond accordingly
void discovery_receiver_listen(discovery_receiver_t *discovery_receiver){
    
    int received_msg_size = 1;
    char *received_msg = calloc(received_msg_size, sizeof(char));

    char buffer[buffer_read_size];
    int received_bytes;

    PSocketAddress *remote_address = NULL;

    // get network interface info
    inet_addresses_t *interfaces = inet_get_if_addresses();

    // receive until received bytes are less than buffer size
    do{
        if(remote_address != NULL)
            p_socket_address_free(remote_address);

        received_bytes = p_socket_receive_from(discovery_receiver->socket, &remote_address, buffer, buffer_read_size, NULL);

        /*
        * filter out the response to itself, this occurs when the host interface address is the same
        * and the port for the remote is the same as our discovery receiver.
        * to simplify things, instead of sharing memory with the receiver thread to see
        * what port is being used, since it can be different from the config file,
        * we can assume the lower digit of the receiver and transmitter are equal because
        * they are initialized at the same time, and by substracting one from another
        * that last digit must be 0, module 10 should mask the last digit out before comparing
        */ 
        if(
            (inet_address_cmp_str(p_socket_address_get_address(remote_address), interfaces) == true) &&
            ((p_socket_address_get_port(remote_address) - p_socket_address_get_port(discovery_receiver->address)) % 10) == 0
        ){
            log("discovery receiver got a multicast to self, ignoring ...\n");
            free(received_msg);
            received_msg = NULL;
            break;
        }

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
        discovery_receiver_respond(discovery_receiver->socket, remote_address, received_msg);
        
        if(remote_address != NULL)
            p_socket_address_free(remote_address);

        free(received_msg);
    }

    // cleaning
    inet_addresses_delete(interfaces);

    return;
}

// initialize discovery_transmitter
discovery_transmitter_t *discovery_transmitter_init(void){
    discovery_transmitter_t *discovery_transmitter = calloc(1, sizeof(*discovery_transmitter));

    int udp_port = config_get("discovery.port_udp_send", int);
    int port_retry_num = config_get("discovery.port_retry_num", int);

    PError *err;

    for(int retry = 0; retry < port_retry_num; retry++){

        // make sure to free if faile don retry
        if(discovery_transmitter->address != NULL)
            p_socket_address_free(discovery_transmitter->address);

        if(discovery_transmitter->socket != NULL)
            p_socket_free(discovery_transmitter->socket);

        // create addres struct
        if((discovery_transmitter->address = p_socket_address_new_any(P_SOCKET_FAMILY_INET, retry+udp_port)) == NULL){
            if(retry == port_retry_num){
                log_error("Error creating address for localhost in discovery_transmitter\n");
                return NULL;
            }
            else{
                log_error("Error creating address for localhost in discovery_transmitter in port [%i] - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }

        // create socket on ip and port
        discovery_transmitter->socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP, &err);
        if(discovery_transmitter->socket == NULL || err != NULL){
            if(err != NULL){
                log_error("[plibsys] [%i] %s\n", p_error_get_code(err), p_error_get_message(err));
                p_error_free(err);
                err = NULL;
            }
            
            if(retry == port_retry_num){
                log_error("Error in creating socket in discovery_transmitter on udp port [%i]\n", retry+udp_port);
                return NULL;
            }
            else{
                log_error("Error in creating socket in discovery_transmitter on udp port [%i] - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }

        // bind
        pboolean res = p_socket_bind(discovery_transmitter->socket, discovery_transmitter->address, false, &err);
        if(res == false || err != NULL){
            if(err != NULL){
                log_error("[plibsys] [%i] %s\n", p_error_get_code(err), p_error_get_message(err));
                p_error_free(err);
                err = NULL;
            }

            if(retry == port_retry_num){
                log_error("Error binding socket on udp port [%d] to localhost in discovery_transmitter\n", retry+udp_port);
                return NULL;
            }
            else{
                log_error("Error binding socket on udp port [%d] to localhost in discovery_transmitter - retry:[%i]\n", retry+udp_port, retry);
                continue;
            }
        }
        
        break;
    }

    // enable loopback
    int loopback = 1;
    if((setsockopt(p_socket_get_fd(discovery_transmitter->socket), IPPROTO_IP, IP_MULTICAST_LOOP, &loopback , sizeof(loopback))) < 0){
        log_error("Error setting up the udp multicast loopback option in discovery_transmitter\n");
        p_socket_address_free(discovery_transmitter->address);
        p_socket_free(discovery_transmitter->socket);
        return NULL;
    }

    // set timeout
    int timeout_ms = config_get("discovery.timeout_ms", int);
    p_socket_set_timeout(discovery_transmitter->socket, timeout_ms);

    log("UDP multicast sender opened in discovery_transmitter on interface %s:%i\n", p_socket_address_get_address(discovery_transmitter->address), p_socket_address_get_port(discovery_transmitter->address));
    
    return discovery_transmitter;
}

// ends the discovery_transmitter, cleaning up memory
void discovery_transmitter_delete(discovery_transmitter_t *discovery_transmitter){
    p_socket_address_free(discovery_transmitter->address);
    p_socket_free(discovery_transmitter->socket);
}

void discovery_transmitter_ping(discovery_transmitter_t *discovery_transmitter){

    // config get
    int port_remote_min = config_get("discovery.port_udp_discovery_range[0]", int);
    int port_remote_max = config_get("discovery.port_udp_discovery_range[1]", int);
    char *multicast_group = config_get("discovery.address_udp_multicast_group", char*);

    PError *err = NULL;

    // ping trought all listening udp ports on multicast group 
    for(int port = port_remote_min; port <= port_remote_max; port++){

        // create remote address
        PSocketAddress *multicast = p_socket_address_new(multicast_group, port);

        // send ping
        psize res = p_socket_send_to(discovery_transmitter->socket, multicast, "{\"type\": 1}", 14ULL, &err);
        if(res == -1 || err != NULL){
            log_error("Error sending multicast in discovery_transmitter to udp group (%s:%i)",
                p_socket_address_get_address(multicast),
                p_socket_address_get_port(multicast)
            );
            
            if(err != NULL){
                log_error("[plibsys] [%i] %s\n", p_error_get_code(err), p_error_get_message(err));
                p_error_free(err);
                err = NULL;
            }
        }

        // the code below wait for responses of the ping
        int received_msg_size = 1;
        char *received_msg = calloc(received_msg_size, sizeof(char));

        char buffer[buffer_read_size];
        int received_bytes;

        PSocketAddress *remote_address = NULL;

        // receive until receive d bytes are less than buffer size
        do{
            if(remote_address != NULL)
                p_socket_address_free(remote_address);
            
            // send ping
            received_bytes = p_socket_receive_from(discovery_transmitter->socket, &remote_address, buffer, buffer_read_size, &err);

            // error on timeout
            if(err != NULL || p_error_get_code(err) == P_ERROR_IO_TIMED_OUT){
                log("discovery_transmitter timed out on (%s:%i)\n", multicast_group, port);
                
                p_error_free(err);
                err = NULL;

                free(received_msg);
                received_msg = NULL;
                
                break;
            }

            // if no bytes received, loop again to receive 
            if(received_bytes == 0){
                continue;
            }

            received_msg_size += received_bytes;
            received_msg = realloc(received_msg, received_msg_size);
            strcat(received_msg, buffer);

        }while(received_bytes == buffer_read_size);

        // process the message
        if(received_msg != NULL){
            log("Received from (%s:%i): %s.\n", 
                p_socket_address_get_address(remote_address), 
                p_socket_address_get_port(remote_address), 
                received_msg
            );

            

            free(received_msg);
        }

        // cleaning
        p_socket_address_free(remote_address);
        p_socket_address_free(multicast);
    }

    return;
}