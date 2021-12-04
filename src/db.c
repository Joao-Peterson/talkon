#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sqlite3.h>
#include <plibsys.h>

/* ----------------------------------------- Defines ---------------------------------------- */

// max len gor buffer path
#define buffer_path_len 500

// config filename
#define db_filename "sqlite.db"

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

/* ----------------------------------------- Private functions ------------------------------- */

/* ----------------------------------------- Functions --------------------------------------- */

void db_init(void){
    // sqlite3 *db; 
    // sqlite3_open("./sqlite.db", &db);
}

void db_delete(void){
    // sqlite3_close(db);
}