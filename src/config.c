#include <stdio.h>
#include <errno.h>

#include "config.h"
#include <doc.h>
#include <doc_json.h>

/* ----------------------------------------- Defines ---------------------------------------- */

// max len gor buffer path
#define buffer_path_len 500

// default json file
#define config_default_json "{\"info\":{\"name\":\"talkinho\",\"pic\":[\"# ## #\",\" #### \",\" #### \",\"# ## #\"]},\"discovery\":{\"port_udp_listen\":5000,\"port_udp_send\":5010,\"address_udp_multicast_group\":\"232.0.1.251\",\"port_retry_num\":9,\"port_udp_discovery_range\":[5000,5009],\"timeout_ms\":1000},\"messeging\":{\"port_tcp_listen\":5020,\"port_tcp_send\":5030,\"port_retry_num\":9,\"timeout_ms\":1000}}"

// config filename
#define config_filename "config.json"

// os specific switch
#if defined(__linux__) || defined(__unix__) || defined(__posix__)

    #include <sys/stat.h>

    // path and filename for config
    #define home_path_var "HOME"
    #define config_path ".config/talkon"

#elif defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__NT__)

    #include <direct.h>

    // path and filename for config
    #define home_path_var "HOMEPATH"
    #define config_path ".talkon"
    #define mkdir(path, mode) _mkdir(path)

#elif defined(__ANDROID__)
    #error android is not supported!
#elif defined(__APPLE__) || defined(__MACH__)
    #error macOS is not supported!
#elif
    #error your OS is not supported!
#endif

/* ----------------------------------------- Globals ---------------------------------------- */

// global config pointer
doc *config_doc = NULL;

char *config_folder_path = NULL;

/* ----------------------------------------- Private functions ------------------------------- */

// detect file existence
int check_fs(char *filename_directory){
    FILE *file = fopen(filename_directory, "r+");

    if(file == NULL){
        switch(errno){
            case ENOENT:
            case ENOTDIR:
            case EACCES:
            case EINVAL:
            default:
                return 0;
                break;

            case EEXIST:
            case EISDIR:
                return 1;
                break;
        }
    }
    else{
        fclose(file);
        return 1;
    }
}

/* ----------------------------------------- Functions --------------------------------------- */

// save config file
void config_save(void){
    char path[buffer_path_len];
    snprintf(path, buffer_path_len, "%s/%s", getenv(home_path_var), config_path);

    if(!check_fs(path)){
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

// init config by file or default 
void config_init(void){

    char path[buffer_path_len];
    snprintf(path, buffer_path_len, "%s/%s/%s", getenv(home_path_var), config_path, config_filename);

    if(check_fs(path)){
        config_doc = doc_json_open(path);
    }
    else{
        config_doc = doc_json_parse(config_default_json);
        config_save();
    }

    if(config_doc == NULL){
        printf("config.json file: %s\n", doc_get_error_msg());
        exit(-1);
    }
}