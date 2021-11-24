#include "transceiver.h"
#include "config.h"
#include "log.h"
#include "stcp.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------- Globals ------------------------------------------ */

// main list of nodes 
doc *transceiver = NULL;

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

// ping ip range for other talkon application
void transceiver_discover(void){
    uint_t ip_min = ip_strto_uint(config_get("discovery.addr_range[0]", char*));
    uint_t ip_max = ip_strto_uint(config_get("discovery.addr_range[1]", char*));
    uint_t port_min = config_get("discovery.port_range[0]", uint_t);
    uint_t port_max = config_get("discovery.port_range[1]", uint_t);

    if(ip_min == 0 || ip_max == 0){
        log_error("error while discovering nodes on network");
        return;
    }

    for(uint_t ip = ip_min; ip <= ip_max; ip++){
        log_debug("Current IP: %X\n", ip);
        
        if(
            ((ip & 0x000000FF) == 0x00) || 
            ((ip & 0x000000FF) == 0x01) || 
            ((ip & 0x000000FF) == 0xFF) ||
            ((ip & 0xFF000000) == 0x00) ||
            ((ip & 0xFF000000) == 0x00)
        )
            continue;

        char ip_str[16] = {0};
        snprintf(ip_str, 16, "%03d.%03d.%03d.%03d", ip & 0xFF000000, ip & 0x00FF0000, ip & 0x0000FF00, ip & 0x000000FF);

        for(uint_t port = port_min; port <= port_max; port++){
            stcp_address *addr = stcp_create_address_ipv4(ip_str, port);
            
            if(addr == NULL)
                continue;

            stcp_channel *node = stcp_create_channel(addr);

            if(node == NULL)
                continue;            
        }
    }
}

// get pointer to node list doc*
doc *transceiver_get_nodes(void){

}

// clean the list of nodes
void transceiver_delete_nodes(void){

}