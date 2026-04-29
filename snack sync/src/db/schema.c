#include <stdio.h>
#include <stdlib.h>

#include "db/schema.h"
#include "models.h"

static char *read_text_file(const char *path, char *error, size_t error_size) {
    FILE *file;
    long length;
    size_t bytes_read;
    char *buffer;

    file = fopen(path, "rb");
    if (file == NULL) {
        snack_sync_copy_string(error, error_size, "Could not open schema.sql.");
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        snack_sync_copy_string(error, error_size, "Could not seek inside schema.sql.");
        return NULL;
    }

    length = ftell(file);
    if (length < 0) {
        fclose(file);
        snack_sync_copy_string(error, error_size, "Could not read schema.sql length.");
        return NULL;
    }

    rewind(file);

    buffer = (char *) malloc((size_t) length + 1);
    if (buffer == NULL) {
        fclose(file);
        snack_sync_copy_string(error, error_size, "Out of memory while loading schema.sql.");
        return NULL;
    }

    bytes_read = fread(buffer, 1, (size_t) length, file);
    fclose(file);

    buffer[bytes_read] = '\0';
    return buffer;
}

int schema_initialize(Database *database, const char *schema_path, char *error, size_t error_size) {
    char *sql;
    int status;

    sql = read_text_file(schema_path, error, error_size);
    if (sql == NULL) {
        return SS_ERROR;
    }

    status = database_execute(database, sql, error, error_size);
    free(sql);
    return status;
}
