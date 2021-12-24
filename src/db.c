#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sqlite3.h>
#include <plibsys.h>
#include "config.h"
#include "log.h"

/* ----------------------------------------- Defines ---------------------------------------- */

// max len gor buffer path
#define buffer_path_len 500

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

// external init.sql file with start queries for sqlite db creation
extern const char _binary_res_init_sql_start[];

/* ----------------------------------------- Private functions ------------------------------- */

/* ----------------------------------------- Functions --------------------------------------- */

// initialize databse object
db_t *db_init(void){
    char *profile_uuid = doc_get(profile_doc, "uuid", char*);

    if(profile_doc == NULL){
        log_error("profile doc_t pointer is NULL.\n");
        return NULL;
    }

    if(profile_uuid == NULL){
        log_error("profile by the name \"%s\" doesn't have a assigned uuid.\n", doc_get(profile_doc, "name", char*));
        return NULL;
    }
        
    db_t *db = (db_t*)calloc(1, sizeof(db_t)); 

    char path[buffer_path_len];
    snprintf(path, buffer_path_len, "%s/%s/%s.db", getenv(home_path_var), config_path, profile_uuid);

    if(p_file_is_exists(path)){                                                     // existing database
        log("opening existing sqlite database \"%s\".\n", path);
        
        if(sqlite3_open(path, &db->sqlite_db) != SQLITE_OK){
            log_error("could not open sqlite database \"%s\".\n", path);
            db_delete(db);
            return NULL;
        }
    }
    else{                                                                           // create new database
        log("creating new sqlite database \"%s\".\n", path);

        if(sqlite3_open(path, &db->sqlite_db) != SQLITE_OK){
            log_error("could not create sqlite database \"%s\".\n", path);
            db_delete(db);
            return NULL;
        }

        char *msg;

        if(sqlite3_exec(db->sqlite_db, _binary_res_init_sql_start, NULL, NULL, &msg) != SQLITE_OK){
            if(msg != NULL){
                log("[SQLITE]: %s.\n", msg);
            }
            log_error("sqlite failed to exec query \"%s\".\n", _binary_res_init_sql_start);
            db_delete(db);
            p_file_remove(path, NULL);
            return NULL;
        }
        else {
            if(msg != NULL){
                log("[SQLITE]: %s.\n", msg);
            }
            log("sqlite executed query \"%s\".\n", _binary_res_init_sql_start);
        }

    }

    return db;
}

// delete database object
void db_delete(db_t *db){
    sqlite3_close(db->sqlite_db);
    free(db);
}


