#ifndef _RECEIVER_HEADER_
#define _RECEIVER_HEADER_

#include <plibsys.h>

// used to de-opaque the Psocket struct making the members available, ie. the socket fd
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

// receiver struct
typedef struct{
    PSocketAddress *address;
    PSocket *socket;
}receiver_t;

// initialize receiver
receiver_t *receiver_init(void);

// ends the receiver, cleaning up memory
void receiver_delete(receiver_t *receiver);

// receiver function that listens on network and respond accordingly
void *receiver_listen(void *data);

#endif