#include "db.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sqlite3.h>
#include <plibsys.h>
#include <doc.h>
#include <doc_print.h>
#include <doc_json.h>
#include "config.h"
#include "simple_tcp_msg.h"
#include "log.h"

/* ----------------------------------------- Defines ---------------------------------------- */

// max query string len 
#define sqlite_query_maxlen 1024

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

// calback function for sqlite3_exec() call, used for nodes select queries
int sqlite_nodes_cb(void *data, int colq, char **colv, char **col_names){
    doc *nodes_array = (doc*)data;

    doc *node = doc_new("node", dt_obj, ";");

    // log_debug("sqlite_exec_callback: colq: %d.\n", colq);

    for(int i = 0; i < colq; i++){
        doc_add(node, ".", col_names[i], dt_string, colv[i], (size_t)strlen(colv[i]) + 1);
        // log_debug("col: %s, val: %s.\n", col_names[i], colv[i]);
    }

    doc_append(nodes_array, ".", node);

    // 0 on success
    return 0;
}

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

// insert node query, receives a doc* that should contain the fields "name" dt_string, "uuid" dt_string and "pic" dt_array of dt_string
int db_insert_node(db_t *db, doc *node_info){
    char query[sqlite_query_maxlen];

    char *uuid = doc_get(node_info, "uuid", char*);
    char *name = doc_get(node_info, "name", char*);

    doc *pic_doc = doc_get_ptr(node_info, "pic");
    char pic[profile_pic_string_len] = {0};

    for(doc_loop(line, pic_doc)){
        strcat(pic, doc_get(line, ".", char*));    

        // append line break on every line except the last one
        if(line->next != NULL)
            strcat(pic, "\n");
    }
        
    snprintf(
        query, sqlite_query_maxlen, 
        "insert into nodes (uuid, name, pic) values (\"%s\", \"%s\", \"%s\");", 
        uuid, 
        name,
        pic
    );

    char *sqlite_err; 
    int res = sqlite3_exec(db->sqlite_db, query, NULL, NULL, &sqlite_err) != SQLITE_OK;

    switch(res){
        case SQLITE_CONSTRAINT:
            log("sqlite database executed the query \"%s\", node is already registered onto the database.\n", query);

            if(sqlite_err != NULL)
                log("[SQLITE] %s", sqlite_err);

            return 0;
        break;
        
        case SQLITE_OK:
            log("sqlite database executed the query \"%s\".\n", query);

            if(sqlite_err != NULL)
                log("[SQLITE] %s", sqlite_err);

            return 0;
        break;

        default:
            log_error("sqlite database couldn't execute the query \"%s\".\n", query);

            if(sqlite_err != NULL)
                log_error("[SQLITE] %s", sqlite_err);

            return 1;
        break;
    }
}

// select all nodes from database
int db_select_nodes(db_t *db, doc **nodes_array){
    char *sqlite_err;
    char query[sqlite_query_maxlen];
    doc *query_res = doc_new("nodes", dt_obj, ";");

    snprintf(
        query, sqlite_query_maxlen, 
        "select name, pic, uuid from nodes;"
    );

    if(sqlite3_exec(db->sqlite_db, query, sqlite_nodes_cb, (void*)query_res, &sqlite_err) != SQLITE_OK){
        log_error("couldn't execute query \"%s\".\n", query);

        if(sqlite_err != NULL)
            log_error("[SQLITE] \"%s\".\n", sqlite_err);

        doc_delete(query_res, ".");
        *nodes_array = NULL;
        return 1;
    }
    else{
        log("executed the query \"%s\".\n", query);
        *nodes_array = query_res;
        log("%s\n", doc_json_stringify(query_res));
        return 0;
    }
}
