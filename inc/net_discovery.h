#ifndef _NET_DISCOVER_HEADER_
#define _NET_DISCOVER_HEADER_

#include <plibsys.h>
#include <doc.h>

/* ----------------------------------------- Struct's ----------------------------------------- */

// discovery_receiver struct
typedef struct{
    PSocketAddress *address;
    PSocket *socket;
}discovery_receiver_t;

// discovery_transmitter struct
typedef struct{
    PSocketAddress *address;
    PSocket *socket;
}discovery_transmitter_t;

/* ----------------------------------------- Functions ---------------------------------------- */

// initialize discovery_receiver
discovery_receiver_t *discovery_receiver_init(void);

// ends the discovery_receiver, cleaning up memory
void discovery_receiver_delete(discovery_receiver_t *discovery_receiver);

// discovery_receiver function that listens on network and respond accordingly
void discovery_receiver_listen(discovery_receiver_t *discovery_receiver);

// initialize discovery_transmitter
discovery_transmitter_t *discovery_transmitter_init(void);

// ends the discovery_transmitter, cleaning up memory
void discovery_transmitter_delete(discovery_transmitter_t *discovery_transmitter);

// discovery_transmitter function that multicasts on the network to discover new nodes
doc *discovery_transmitter_ping(discovery_transmitter_t *discovery_transmitter);

#endif