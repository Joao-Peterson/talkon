#ifndef _CONFIG_HEADER_
#define _CONFIG_HEADER_

#include <doc.h>
#include <plibsys.h>

// global config pointer
extern doc *config_doc;

// current profile pointer
extern doc *profile_doc;

// rw lock for acessing the config obj
extern PRWLock *config_rw_lock;

// init config by file or default, receives the profile to be used
void config_init(uint32_t profile_id);

// free memory
void config_end(void);

// save config file
void config_save(void);

// returns the path to the talkon config folder, DO NOT FREE
char *config_get_config_folder_path(void);

// get config by name
#define config_get(name, type) \
    (*(type*)((void*)doc_get_ptr(config_doc, name) + sizeof(doc)))

// get profile info by name
#define profile_get(name, type) \
    (*(type*)((void*)doc_get_ptr(profile_doc, name) + sizeof(doc)))

// #define config_get(name, type) \
//     p_rwlock_reader_lock(config_rw_lock); \
//     (*(type*)((void*)doc_get_ptr(config_doc, name) + sizeof(doc))); \
//     p_rwlock_reader_unlock(config_rw_lock)

#endif