#include "inet_extra.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdint.h>

/* ----------------------------------------- Functions ---------------------------------------- */

// allocate memory for a new node
inet_addresses_t *inet_addresses_new(void){
    inet_addresses_t *ll = (inet_addresses_t*)calloc(1, sizeof(inet_addresses_t));
    return ll;
}

// push a addres value onto the linked list
void inet_addresses_push(inet_addresses_t **addresses, uint32_t address){
    if(*addresses == NULL){
        *addresses = inet_addresses_new();
        (*addresses)->address = address;
    }
    else{
        inet_addresses_t *new = inet_addresses_new();
        new->address = address;
        new->next = NULL;

        for(inet_addresses_loop(cursor, (*addresses))){
            if(cursor->next == NULL){

                cursor->next = new;

                break;
            }
        }
    }

    return;
}

// delete the linked list recusevly
void inet_addresses_delete(inet_addresses_t *addresses){
    inet_addresses_t *buff = NULL;
    for(inet_addresses_t *cursor = addresses; cursor != NULL; ){
        buff = cursor->next;

        free(cursor);

        cursor = buff;
    }
}

// get the network interfaces ipv4 addresses, populating the linked list
inet_addresses_t *inet_get_if_addresses(void){
    inet_addresses_t *ll = NULL;
    struct ifaddrs *interfaces;
    
    // get linked list for network interfaces
    // https://linux.die.net/man/3/getifaddrs
    getifaddrs(&interfaces);

    // loop through them
    for(struct ifaddrs *interface = interfaces; interface != NULL; interface = interface->ifa_next){

        // if it's an ipv4 interface
        if(interface->ifa_addr->sa_family == AF_INET){
            // get host address (ip)
            // https://linux.die.net/man/3/getnameinfo

            // char host[NI_MAXHOST] = {0};
            // size_t sa_size = (interface->ifa_addr->sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
            // getnameinfo(interface->ifa_addr, sa_size, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

            // store onto linked list
            uint32_t address = ntohl(((struct sockaddr_in*)(interface->ifa_addr))->sin_addr.s_addr);
            inet_addresses_push(&ll, address);
        }
    }

    freeifaddrs(interfaces);

    return ll;
}

// compare a address value to see if it is present into the list
// the address value is must be in host byte order
int inet_address_cmp(uint32_t address, inet_addresses_t *addresses){
    for(inet_addresses_loop(cursor, addresses)){
        if(address == cursor->address)
            return 1;
    }

    return 0;
}

// compare a address string to see if it is present into the list
int inet_address_cmp_str(char *address, inet_addresses_t *addresses){
    if(address == NULL || addresses == NULL) return 0; 
    uint32_t addr_num = ntohl(inet_addr(address));
    if(addr_num == -1) return 0;
    return inet_address_cmp(addr_num, addresses);
}
