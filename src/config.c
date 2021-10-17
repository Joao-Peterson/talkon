#include "config.h"
#include "doc.h"
#include "doc_json.h"

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

/* ----------------------------------------- Defines ---------------------------------------- */

// default json file
#define config_default_json "{\"name\": \"talkinho\",\"port\":5002}"

// config filename
#define config_filename "config.json"

// path to config.json file
#if defined(__linux__) || defined(__unix__) || defined(__posix__)

    #define home_path_var "HOME"
    #define config_path ".config/talkon"

#elif defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__NT__)

    #define home_path_var "HOMEPATH"
    #define config_path ".talkon"

#elif defined(__ANDROID__)
    #error android is not supported!
#elif defined(__APPLE__) || defined(__MACH__)
    #error macOS is not supported!
#elif
    #error your OS is not supported!
#endif

/* ----------------------------------------- Globals ---------------------------------------- */

// global config pointer
doc *config = NULL;

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
    char path[500];
    snprintf(path, 500, "%s/%s", getenv(home_path_var), config_path);

    if(!check_fs(path)){
        mkdir(path, S_IRWXU);
    }
    
    snprintf(path, 500, "%s/%s/%s", getenv(home_path_var), config_path, config_filename);

    doc_json_save(config, path);
}

// init config by file or default 
void config_init(void){

    char path[500];
    snprintf(path, 500, "%s/%s/%s", getenv(home_path_var), config_path, config_filename);

    if(check_fs(path)){
        config = doc_json_open(path);
    }
    else{
        config = doc_json_parse(config_default_json);
        config_save();
    }

    if(config == NULL){
        printf("config.json file: %s\n", doc_get_error_msg());
        exit(-1);
    }
}
