#ifndef _CONFIG_HEADER_
#define _CONFIG_HEADER_

#include "doc.h"

// global config pointer
extern doc *config;

// init config by file or default 
void config_init(void);

// save config file
void config_save(void);

#endif