#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "config.h"
#include <doc.h>
#include <doc_json.h>
#include <plibsys.h>
#include <string.h>
#include <uuid/uuid.h>

/* ----------------------------------------- Defines ---------------------------------------- */

// max len gor buffer path
#define buffer_path_len 500

// config filename
#define config_filename "config.json"

// os specific switch
#if defined(P_OS_LINUX)

    #include <sys/stat.h>
    // path and filename for config
    #define home_path_var "HOME"
    #define config_path ".config/talkon"

#elif defined(P_OS_WIN) || defined(P_OS_WIN64)

    #include <direct.h>

    // path and filename for config
    #define home_path_var "HOMEPATH"
    #define config_path ".talkon"
    #define mkdir(path, mode) _mkdir(path)

#elif defined(P_OS_ANDROID)
    #error android is not supported because of OS specific calls like mkdir() and OS specific filesystem hierarchy paths for config files. \
Feel free to add a OS preprocessor switch for OS and implement the necessary functionality.
#elif defined(P_OS_MAC) || defined(P_OS_MAC9)
    #error macOS9 and macOS are not supported because of OS specific calls like mkdir() and OS specific filesystem hierarchy paths for config files. \
Feel free to add a OS preprocessor switch for OS and implement the necessary functionality.
#elif
    #error your OS is not supported because of OS specific calls like mkdir() and OS specific filesystem hierarchy paths for config files. \
Feel free to add a OS preprocessor switch for OS and implement the necessary functionality.
#endif

/* ----------------------------------------- Globals ---------------------------------------- */

// global config pointer
doc *config_doc = NULL;

doc *profile_doc = NULL;

// config folder path
char *config_folder_path = NULL;

// default json file
extern const char _binary_res_config_json_start[];

// rw lock for acessing the config obj
PRWLock *config_rw_lock = NULL;

/* ----------------------------------------- Private functions ------------------------------- */

/* ----------------------------------------- Functions --------------------------------------- */

// save config file
void config_save(void){
    char path[buffer_path_len];
    snprintf(path, buffer_path_len, "%s/%s", getenv(home_path_var), config_path);

    if(!p_file_is_exists(path)){
        mkdir(path, S_IRWXU);
    }
    
    snprintf(path, buffer_path_len, "%s/%s/%s", getenv(home_path_var), config_path, config_filename);

    doc_json_save(config_doc, path);
}

// returns the path to the talkon config folder, DO NOT FREE
char *config_get_config_folder_path(void){
    if(config_folder_path == NULL){
        config_folder_path = calloc(buffer_path_len + 1, sizeof(char));
        snprintf(config_folder_path, buffer_path_len, "%s/%s", getenv(home_path_var), config_path);
    }
    
    return config_folder_path;
}

// init config by file or default, receives the profile to be used
void config_init(uint32_t profile_id){

    char path[buffer_path_len];
    snprintf(path, buffer_path_len, "%s/%s/%s", getenv(home_path_var), config_path, config_filename);

    // check for config file existence
    if(p_file_is_exists(path)){
        config_doc = doc_json_open(path);
    }
    else{
        config_doc = doc_json_parse((char*)_binary_res_config_json_start);
        config_save(); 
    }

    if(config_doc == NULL){
        printf("config.json file: %s\n", doc_get_error_msg());
        exit(-1);
    }

    // profile information
    doc *profiles = doc_get_ptr(config_doc, "profiles");
    for(doc_loop(profile, profiles)){
        if(doc_get_ptr(profile, "id") == NULL){
            uuid_t uuid; 
            uuid_generate(uuid);
            char *uuid_str = (char*)calloc(37, sizeof(char));

            uuid_unparse_lower(uuid, uuid_str);

            doc_add(profile, ".", "id", dt_string, uuid_str, strlen(uuid_str) + 1, ";");
        }
    }
    snprintf(path, buffer_path_len, "profiles[%u]", profile_id);
    profile_doc = doc_get_ptr(config_doc, path);

    // create read lock, future usage
    config_rw_lock = p_rwlock_new();
}

// free memory
void config_end(void){
    config_save();
    doc_delete(config_doc, ".");
    config_doc = NULL;
    profile_doc = NULL;
    p_rwlock_free(config_rw_lock);
}
