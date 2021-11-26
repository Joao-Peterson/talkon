#include "net_messaging.h"
#include "config.h"
#include "log.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <plibsys.h>

/* ----------------------------------------- Globals ------------------------------------------ */

// main list of nodes 
doc *transmitter = NULL;

/* ----------------------------------------- Private functions -------------------------------- */

uint_t ip_strto_uint(char *ip_string){
    if(ip_string == NULL){
        log_error("error parsing ip string from configuration.\nIP string: \"%s\".\n", ip_string);
        return 0;
    }
    
    int ip = 0;

    for(int i = 0; i < 4; i++){
        ip |= (((uint8_t)atoi(strtok(NULL, "."))) << 8);

        log_debug("IP = %X\n", ip);
    }

    return ip;
}

/* ----------------------------------------- Functions ---------------------------------------- */

// get pointer to node list doc*
doc *transmitter_get_nodes(void){
    PSocketAddress *addr = p_socket_address_new_any(P_SOCKET_FAMILY_INET, 5005);                    
    PSocket *socket = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_DATAGRAM, P_SOCKET_PROTOCOL_UDP, NULL);
    p_socket_bind(socket, addr, 0, NULL);

    int flag = 1;
    setsockopt(p_socket_get_fd(socket), IPPROTO_IP, IP_MULTICAST_LOOP, (void*)(&flag), sizeof(flag));

    PSocketAddress *multicast = p_socket_address_new(config_get("udp_group", char*), 5001);
    psize res = p_socket_send_to(socket, multicast, "{\"type\": 1}", 14ULL, NULL);
    // psize res = p_socket_send_to(socket, multicast, "MULTICAST!!!!", 14ULL, NULL);
    log_debug("sended: [%i]\n", res);

    int received_msg_size = 1;
    char *received_msg = calloc(received_msg_size, sizeof(char));

    char buffer[1024];
    int received_bytes;

    PSocketAddress *remote_address = NULL;
    // receive until receive d bytes are less than buffer size
    do{
        if(remote_address != NULL)
            p_socket_address_free(remote_address);
        
        received_bytes = p_socket_receive_from(socket, &remote_address, buffer, 1024, NULL);
        if(received_bytes == 0){
            continue;
        }

        received_msg_size += received_bytes;
        received_msg = realloc(received_msg, received_msg_size);
        strcat(received_msg, buffer);

    }while(received_bytes == 1024);

    if(received_msg != NULL){
        log("Received from (%s:%i): %s.\n", 
            p_socket_address_get_address(remote_address), 
            p_socket_address_get_port(remote_address), 
            received_msg
        );

        free(received_msg);
    }

    p_socket_free(socket);
    p_socket_address_free(remote_address);
    p_socket_address_free(addr);
    p_socket_address_free(multicast);
}

// clean the list of nodes
void transmitter_delete_nodes(void){

}