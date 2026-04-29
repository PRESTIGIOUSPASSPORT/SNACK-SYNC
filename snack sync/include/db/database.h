#ifndef SNACK_SYNC_DATABASE_H
#define SNACK_SYNC_DATABASE_H

#include <stddef.h>

typedef struct Database Database;
typedef struct QueryResult QueryResult;

Database *database_create(void);
int database_open(Database *database, char *error, size_t error_size);
void database_close(Database *database);
void database_destroy(Database *database);
const char *database_last_error(const Database *database);

int database_execute(Database *database, const char *sql, char *error, size_t error_size);
QueryResult *database_query(
        Database *database,
        const char *sql,
        int parameter_count,
        const char *const *parameters,
        char *error,
        size_t error_size
);
int database_command(
        Database *database,
        const char *sql,
        int parameter_count,
        const char *const *parameters,
        char *error,
        size_t error_size
);
int database_begin(Database *database, char *error, size_t error_size);
int database_commit(Database *database, char *error, size_t error_size);
int database_rollback(Database *database, char *error, size_t error_size);

int query_result_rows(const QueryResult *result);
const char *query_result_text(const QueryResult *result, int row, int column);
int query_result_int(const QueryResult *result, int row, int column);
double query_result_double(const QueryResult *result, int row, int column);
int query_result_bool(const QueryResult *result, int row, int column);
void query_result_destroy(QueryResult *result);

#endif
