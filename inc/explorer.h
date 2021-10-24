#ifndef _EXPLORER_HEADER_
#define _EXPLORER_HEADER_

#include <doc.h>

// ping ip range for other talkon application
void explorer_discover(void);

// get pointer to node list doc*
doc *explorer_get_nodes(void);

// clean the list of nodes
void explorer_delete_nodes(void);

#endif