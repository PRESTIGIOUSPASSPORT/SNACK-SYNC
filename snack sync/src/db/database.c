#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>

#include "db/database.h"
#include "models.h"

struct Database {
    PGconn *connection;
    char last_error[APP_ERROR_SIZE];
};

struct QueryResult {
    PGresult *result;
};

static void copy_error(Database *database, char *error, size_t error_size, const char *message) {
    if (database != NULL) {
        snack_sync_copy_string(database->last_error, sizeof(database->last_error), message);
    }
    if (error != NULL && error_size > 0) {
        snack_sync_copy_string(error, error_size, message);
    }
}

static int ensure_connection(Database *database, char *error, size_t error_size) {
    if (database == NULL || database->connection == NULL) {
        copy_error(database, error, error_size, "Database connection is not open.");
        return SS_ERROR;
    }
    return SS_OK;
}

static void copy_pg_error(Database *database, char *error, size_t error_size, const char *fallback) {
    const char *message;

    message = fallback;
    if (database != NULL && database->connection != NULL) {
        const char *pg_message = PQerrorMessage(database->connection);
        if (pg_message != NULL && *pg_message != '\0') {
            message = pg_message;
        }
    }
    copy_error(database, error, error_size, message);
}

static void build_conninfo(char *buffer, size_t buffer_size) {
    const char *conninfo;
    const char *host;
    const char *port;
    const char *name;
    const char *user;
    const char *password;

    conninfo = getenv("SNACK_SYNC_DB_CONNINFO");
    if (conninfo != NULL && *conninfo != '\0') {
        snack_sync_copy_string(buffer, buffer_size, conninfo);
        return;
    }

    host = getenv("SNACK_SYNC_DB_HOST");
    port = getenv("SNACK_SYNC_DB_PORT");
    name = getenv("SNACK_SYNC_DB_NAME");
    user = getenv("SNACK_SYNC_DB_USER");
    password = getenv("SNACK_SYNC_DB_PASSWORD");

    snprintf(
            buffer,
            buffer_size,
            "host=%s port=%s dbname=%s user=%s password=%s",
            (host != NULL && *host != '\0') ? host : "localhost",
            (port != NULL && *port != '\0') ? port : "5432",
            (name != NULL && *name != '\0') ? name : "snack_sync",
            (user != NULL && *user != '\0') ? user : "postgres",
            (password != NULL && *password != '\0') ? password : "postgres"
    );
}

Database *database_create(void) {
    Database *database;

    database = (Database *) calloc(1, sizeof(Database));
    if (database != NULL) {
        snack_sync_copy_string(database->last_error, sizeof(database->last_error), "");
    }
    return database;
}

int database_open(Database *database, char *error, size_t error_size) {
    char conninfo[APP_ERROR_SIZE * 2];

    if (database == NULL) {
        if (error != NULL && error_size > 0) {
            snack_sync_copy_string(error, error_size, "Database object was not created.");
        }
        return SS_ERROR;
    }

    build_conninfo(conninfo, sizeof(conninfo));
    database->connection = PQconnectdb(conninfo);
    if (PQstatus(database->connection) != CONNECTION_OK) {
        copy_pg_error(database, error, error_size, "Failed to connect to PostgreSQL.");
        database_close(database);
        return SS_ERROR;
    }

    copy_error(database, error, error_size, "");
    return SS_OK;
}

void database_close(Database *database) {
    if (database != NULL && database->connection != NULL) {
        PQfinish(database->connection);
        database->connection = NULL;
    }
}

void database_destroy(Database *database) {
    if (database == NULL) {
        return;
    }

    database_close(database);
    free(database);
}

const char *database_last_error(const Database *database) {
    if (database == NULL) {
        return "";
    }
    return database->last_error;
}

int database_execute(Database *database, const char *sql, char *error, size_t error_size) {
    PGresult *result;
    ExecStatusType status;

    if (ensure_connection(database, error, error_size) != SS_OK) {
        return SS_ERROR;
    }

    result = PQexec(database->connection, sql);
    status = PQresultStatus(result);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        copy_pg_error(database, error, error_size, "Failed to execute SQL statement.");
        PQclear(result);
        return SS_ERROR;
    }

    PQclear(result);
    copy_error(database, error, error_size, "");
    return SS_OK;
}

QueryResult *database_query(
        Database *database,
        const char *sql,
        int parameter_count,
        const char *const *parameters,
        char *error,
        size_t error_size
) {
    PGresult *result;
    QueryResult *query_result;

    if (ensure_connection(database, error, error_size) != SS_OK) {
        return NULL;
    }

    result = PQexecParams(database->connection, sql, parameter_count, NULL, parameters, NULL, NULL, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        copy_pg_error(database, error, error_size, "Failed to query PostgreSQL.");
        PQclear(result);
        return NULL;
    }

    query_result = (QueryResult *) malloc(sizeof(QueryResult));
    if (query_result == NULL) {
        copy_error(database, error, error_size, "Out of memory while handling query results.");
        PQclear(result);
        return NULL;
    }

    query_result->result = result;
    copy_error(database, error, error_size, "");
    return query_result;
}

int database_command(
        Database *database,
        const char *sql,
        int parameter_count,
        const char *const *parameters,
        char *error,
        size_t error_size
) {
    PGresult *result;

    if (ensure_connection(database, error, error_size) != SS_OK) {
        return SS_ERROR;
    }

    result = PQexecParams(database->connection, sql, parameter_count, NULL, parameters, NULL, NULL, 0);
    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        copy_pg_error(database, error, error_size, "Failed to execute PostgreSQL command.");
        PQclear(result);
        return SS_ERROR;
    }

    PQclear(result);
    copy_error(database, error, error_size, "");
    return SS_OK;
}

int database_begin(Database *database, char *error, size_t error_size) {
    return database_execute(database, "BEGIN", error, error_size);
}

int database_commit(Database *database, char *error, size_t error_size) {
    return database_execute(database, "COMMIT", error, error_size);
}

int database_rollback(Database *database, char *error, size_t error_size) {
    return database_execute(database, "ROLLBACK", error, error_size);
}

int query_result_rows(const QueryResult *result) {
    if (result == NULL || result->result == NULL) {
        return 0;
    }
    return PQntuples(result->result);
}

const char *query_result_text(const QueryResult *result, int row, int column) {
    if (result == NULL || result->result == NULL || PQgetisnull(result->result, row, column)) {
        return "";
    }
    return PQgetvalue(result->result, row, column);
}

int query_result_int(const QueryResult *result, int row, int column) {
    return atoi(query_result_text(result, row, column));
}

double query_result_double(const QueryResult *result, int row, int column) {
    return strtod(query_result_text(result, row, column), NULL);
}

int query_result_bool(const QueryResult *result, int row, int column) {
    const char *value;

    value = query_result_text(result, row, column);
    return value[0] == 't' || value[0] == '1';
}

void query_result_destroy(QueryResult *result) {
    if (result == NULL) {
        return;
    }

    if (result->result != NULL) {
        PQclear(result->result);
    }
    free(result);
}
