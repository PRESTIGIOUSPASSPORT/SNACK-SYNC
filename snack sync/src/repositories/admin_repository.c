#include "repositories/admin_repository.h"

void admin_repository_init(AdminRepository *repository, Database *database) {
    repository->database = database;
}

int admin_repository_create(
        AdminRepository *repository,
        const char *name,
        const char *email,
        const char *password,
        Admin *out_admin,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "INSERT INTO admins(name, email, password) VALUES ($1, $2, $3) "
            "RETURNING id";
    const char *parameters[3];
    QueryResult *result;

    parameters[0] = name;
    parameters[1] = email;
    parameters[2] = password;

    result = database_query(repository->database, sql, 3, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    if (query_result_rows(result) == 0) {
        query_result_destroy(result);
        snack_sync_copy_string(error, error_size, "Admin registration did not return a new ID.");
        return SS_ERROR;
    }

    out_admin->id = query_result_int(result, 0, 0);
    snack_sync_copy_string(out_admin->name, sizeof(out_admin->name), name);
    snack_sync_copy_string(out_admin->email, sizeof(out_admin->email), email);

    query_result_destroy(result);
    return SS_OK;
}

int admin_repository_authenticate(
        AdminRepository *repository,
        const char *email,
        const char *password,
        Admin *out_admin,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, name, email FROM admins WHERE email = $1 AND password = $2";
    const char *parameters[2];
    QueryResult *result;

    parameters[0] = email;
    parameters[1] = password;

    result = database_query(repository->database, sql, 2, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    if (query_result_rows(result) == 0) {
        query_result_destroy(result);
        snack_sync_copy_string(error, error_size, "");
        return SS_NOT_FOUND;
    }

    out_admin->id = query_result_int(result, 0, 0);
    snack_sync_copy_string(out_admin->name, sizeof(out_admin->name), query_result_text(result, 0, 1));
    snack_sync_copy_string(out_admin->email, sizeof(out_admin->email), query_result_text(result, 0, 2));

    query_result_destroy(result);
    return SS_OK;
}
