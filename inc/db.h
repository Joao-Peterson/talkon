#ifndef _DB_HEADER_
#define _DB_HEADER_

#include <sqlite3.h>

/* ----------------------------------------- Structures -------------------------------------- */

typedef struct{
    sqlite3 *sqlite_db;
}db_t;

/* ----------------------------------------- Functions --------------------------------------- */

// initialize databse object
db_t *db_init(void);

// delete database object
void db_delete(db_t *db);

#endif