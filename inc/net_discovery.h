#ifndef _NET_DISCOVER_HEADER_
#define _NET_DISCOVER_HEADER_

#include <plibsys.h>
#include <doc.h>

/* ----------------------------------------- Struct's ----------------------------------------- */

// compund psocket and psocketadress struct
typedef struct{
    PSocketAddress *address;
    PSocket *socket;
}psocket_address_t;

// discovery_receiver struct
typedef struct{
    psocket_address_t *socket;
    PUThread *thread;
    bool flag_running;
}discovery_receiver_t;

// discovery_transmitter struct
typedef struct{
    psocket_address_t *socket;
    PUThread *thread;
    PRWLock *flags_lock;
    bool flag_ping;
    bool flag_done;
    bool flag_running;
    uint32_t port_remote_min; 
    uint32_t port_remote_max;
    char *multicast_group_ip_address;
}discovery_transmitter_t;

/* ----------------------------------------- Functions ---------------------------------------- */

// initialize discovery_receiver
discovery_receiver_t *discovery_receiver_init(uint32_t udp_port, uint32_t port_retry_num);

// ends the discovery_receiver, cleaning up memory
void discovery_receiver_delete(discovery_receiver_t *receiver);

// initialize discovery_transmitter
discovery_transmitter_t *discovery_transmitter_init(
    uint32_t udp_port, uint32_t port_retry_num, uint32_t timeout_ms,
    uint32_t port_remote_min, uint32_t port_remote_max,
    char *multicast_group_ip_address
);

// ends the discovery_transmitter, cleaning up memory
void discovery_transmitter_delete(discovery_transmitter_t *transmitter);

// discovery_transmitter function that multicasts on the network to discover new nodes
doc *discovery_transmitter_ping(discovery_transmitter_t *transmitter);

#endif