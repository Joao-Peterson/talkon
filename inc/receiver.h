#ifndef _RECEIVER_HEADER_
#define _RECEIVER_HEADER_

#include "stcp.h"

// receiver struct
typedef struct{
    stcp_address *server_addr;
    stcp_server *server;
}receiver_t;

// initialize receiver
receiver_t *receiver_init(void);

// ends the receiver, cleaning up memory
void receiver_delete(receiver_t *receiver);

// receiver function that listens on network and respond accordingly
void *receiver_listen(void *data);

#endif