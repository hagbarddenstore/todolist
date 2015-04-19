#include "item.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sqlite3.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#define DB_DIRECTORY "/Users/hagbard/.todo/"

#define DB_CREATE_SQL "CREATE TABLE IF NOT EXISTS items ( " \
"id INTEGER PRIMARY KEY AUTOINCREMENT," \
"description TEXT NOT NULL, " \
"created_on BIGINT NOT NULL, " \
"expires_on BIGINT NOT NULL, " \
"done_on BIGINT NOT NULL DEFAULT 0" \
");"

#define DB_LIST_SQL "SELECT id, description, created_on, expires_on, done_on " \
"FROM items " \
"WHERE done_on >= strftime('%s', date('now', '-10 day')) OR done_on == 0;"

#define DB_INSERT_SQL "INSERT INTO items " \
"(description, created_on, expires_on) " \
"VALUES (?, strftime('%s', 'now'), strftime('%s', date('now', '+10 day')));"

#define DB_REMOVE_SQL "UPDATE items " \
"SET done_on = strftime('%s', 'now') " \
"WHERE id = ?;"

struct item {
    int id;
    char *description;
    int64_t created_on;
    int64_t expires_on;
    int64_t done_on;
};

sqlite3* open_db();
void ensure_db_directory_exists(void);
int callback(
    __attribute__ ((unused)) void *not_used,
    int argc,
    char **argv,
    char **column_name);
void print_item(struct item item);

void remove_item(int id) {
    int return_code;
    const char *error_message = NULL;
    sqlite3_stmt *statement = NULL;

    sqlite3 *db = open_db();

    return_code = sqlite3_prepare(db, DB_REMOVE_SQL, -1, &statement, NULL);

    if (return_code) {
        error_message = sqlite3_errmsg(db);

        fprintf(stderr, "SQL error: %s\n", error_message);

        sqlite3_close(db);

        return;
    }

    return_code = sqlite3_bind_int(
        statement,
        1,
        id);

    if (return_code) {
        error_message = sqlite3_errmsg(db);

        fprintf(stderr, "SQL error: %s\n", error_message);

        sqlite3_close(db);

        return;
    }

    return_code = sqlite3_step(statement);

    if (return_code == SQLITE_DONE) {
        sqlite3_finalize(statement);
        sqlite3_close(db);

        return;
    }

    if (return_code) {
        error_message = sqlite3_errmsg(db);

        fprintf(stderr, "SQL error: %s\n", error_message);

        sqlite3_close(db);

        return;
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
}

void add_item(const char *description) {
    int return_code;
    const char *error_message = NULL;
    sqlite3_stmt *statement = NULL;

    sqlite3 *db = open_db();

    return_code = sqlite3_prepare(db, DB_INSERT_SQL, -1, &statement, NULL);

    if (return_code) {
        error_message = sqlite3_errmsg(db);

        fprintf(stderr, "SQL error: %s\n", error_message);

        sqlite3_close(db);

        return;
    }

    return_code = sqlite3_bind_text(
        statement,
        1,
        description,
        -1,
        SQLITE_STATIC);

    if (return_code) {
        error_message = sqlite3_errmsg(db);

        fprintf(stderr, "SQL error: %s\n", error_message);

        sqlite3_close(db);

        return;
    }

    return_code = sqlite3_step(statement);

    if (return_code == SQLITE_DONE) {
        sqlite3_finalize(statement);
        sqlite3_close(db);

        return;
    }

    if (return_code) {
        error_message = sqlite3_errmsg(db);

        fprintf(stderr, "SQL error: %s\n", error_message);

        sqlite3_close(db);

        return;
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);
}

void list_items(void) {
    int return_code;
    char *error_message = NULL;

    sqlite3 *db = open_db();

    return_code = sqlite3_exec(db, DB_LIST_SQL, callback, 0, &error_message);

    if (return_code) {
        fprintf(stderr, "SQL error: %s\n", error_message);

        sqlite3_free(error_message);
    }

    sqlite3_close(db);
}

sqlite3* open_db(void) {
    sqlite3 *db;
    int return_code;
    char *error_message = NULL;
    char db_filename[strlen(DB_DIRECTORY) + 3];

    sprintf(db_filename, "%sdb", DB_DIRECTORY);

    ensure_db_directory_exists();

    return_code = sqlite3_open(db_filename, &db);

    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));

        sqlite3_close(db);

        return NULL;
    }

    return_code = sqlite3_exec(db, DB_CREATE_SQL, NULL, 0, &error_message);

    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", error_message);

        sqlite3_free(error_message);
    }

    return db;
}

void ensure_db_directory_exists(void) {
    int result = mkdir(DB_DIRECTORY, 0700);

    if (result && EEXIST != errno) {
        perror(DB_DIRECTORY);
    }
}

int callback(
    __attribute__ ((unused)) void *not_used,
    int argc,
    char **argv,
    char **column_name) {
    struct item item;

    for (int i = 0; i < argc; i++) {
        char *value = argv[i] ? argv[i] : "NULL";

        if (strcmp(column_name[i], "id") == 0) {
            item.id = atoi(value);
        }

        if (strcmp(column_name[i], "done_on") == 0) {
            item.done_on = atoi(value);
        }

        if (strcmp(column_name[i], "created_on") == 0) {
            item.created_on = atoi(value);
        }

        if (strcmp(column_name[i], "expires_on") == 0) {
            item.expires_on = atoi(value);
        }

        if (strcmp(column_name[i], "description") == 0) {
            item.description = value;
        }
    }

    print_item(item);

    return 0;
}

void print_item(struct item item) {
    if (item.done_on > 0) {
        printf("#%d \033[32m%s\033[39;49m\n", item.id, item.description);
    } else if(item.expires_on <= (unsigned)time(NULL)) {
        printf("#%d \033[31m%s\033[39;49m\n", item.id, item.description);
    } else {
        printf("#%d %s\n", item.id, item.description);
    }
}
