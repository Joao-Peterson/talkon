#ifndef _TRANSMITTER_HEADER_
#define _TRANSMITTER_HEADER_

#include <doc.h>

/* ----------------------------------------- Struct's ----------------------------------------- */



/* ----------------------------------------- Functions ---------------------------------------- */

// ping ip range for other talkon application
void transmitter_discover(void);

// get pointer to node list doc*
doc *transmitter_get_nodes(void);

// clean the list of nodes
void transmitter_delete_nodes(void);

#endif