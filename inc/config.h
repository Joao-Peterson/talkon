#ifndef _CONFIG_HEADER_
#define _CONFIG_HEADER_

#include "doc.h"

// global config pointer
extern doc *config_doc;

// init config by file or default 
void config_init(void);

// save config file
void config_save(void);

// get config by name
#define config_get(name, type) (*(type*)((void*)doc_get_ptr(config_doc, name) + sizeof(doc)))
#endif