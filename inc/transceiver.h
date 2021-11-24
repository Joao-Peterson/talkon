#ifndef _TRANSCEIVER_HEADER_
#define _TRANSCEIVER_HEADER_

#include <doc.h>

// ping ip range for other talkon application
void transceiver_discover(void);

// get pointer to node list doc*
doc *transceiver_get_nodes(void);

// clean the list of nodes
void transceiver_delete_nodes(void);

#endif