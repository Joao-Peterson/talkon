#ifndef _DB_HEADER_
#define _DB_HEADER_

#include <sqlite3.h>
#include <doc.h>

/* ----------------------------------------- Structures -------------------------------------- */

typedef struct{
    sqlite3 *sqlite_db;
}db_t;

/* ----------------------------------------- Functions --------------------------------------- */

// initialize databse object
db_t *db_init(void);

// delete database object
void db_delete(db_t *db);

// insert node query, receives a doc* that should contain the fields "name" dt_string, "uuid" dt_string and "pic" dt_array of dt_string
int db_insert_node(db_t *db, doc *node_info);

// select all nodes from database
int db_select_nodes(db_t *db, doc **nodes_array);

#endif