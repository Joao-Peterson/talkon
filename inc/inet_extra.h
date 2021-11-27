#ifndef _INET_EXTRA_HEADER_
#define _INET_EXTRA_HEADER_

#include <stdint.h>

/* ----------------------------------------- Typedef's ---------------------------------------- */

// struct inet_addresses typedef
typedef struct inet_addresses_t inet_addresses_t;

/* ----------------------------------------- Defines ------------------------------------------ */

// macro to use on for loops
#define inet_addresses_loop(iterator, list) inet_addresses_t *iterator = list; iterator != NULL; iterator = iterator->next

/* ----------------------------------------- Struct's ----------------------------------------- */

struct inet_addresses_t{
    inet_addresses_t *next;
    uint32_t address;
};

/* ----------------------------------------- Functions ---------------------------------------- */

// push a addres value onto the linked list
void inet_addresses_push(inet_addresses_t **addresses, uint32_t address);

// delete the linked list recusevly
void inet_addresses_delete(inet_addresses_t *addresses);

// get the network interfaces ipv4 addresses, populating the linked list
inet_addresses_t *inet_get_if_addresses(void);

// compare a address value to see if it is present into the list
// the address value is must be in host byte order
int inet_address_cmp(uint32_t address, inet_addresses_t *addresses);

// compare a address string to see if it is present into the list
int inet_address_cmp_str(char *address, inet_addresses_t *addresses);

#endif