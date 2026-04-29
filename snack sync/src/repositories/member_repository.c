#include "repositories/member_repository.h"

void member_repository_init(MemberRepository *repository, Database *database) {
    repository->database = database;
}

int member_repository_create(
        MemberRepository *repository,
        const char *name,
        const char *email,
        const char *password,
        Member *out_member,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "INSERT INTO members(name, email, password) VALUES ($1, $2, $3) "
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
        snack_sync_copy_string(error, error_size, "Member registration did not return a new ID.");
        return SS_ERROR;
    }

    out_member->id = query_result_int(result, 0, 0);
    snack_sync_copy_string(out_member->name, sizeof(out_member->name), name);
    snack_sync_copy_string(out_member->email, sizeof(out_member->email), email);

    query_result_destroy(result);
    return SS_OK;
}

int member_repository_authenticate(
        MemberRepository *repository,
        const char *email,
        const char *password,
        Member *out_member,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, name, email FROM members WHERE email = $1 AND password = $2";
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

    out_member->id = query_result_int(result, 0, 0);
    snack_sync_copy_string(out_member->name, sizeof(out_member->name), query_result_text(result, 0, 1));
    snack_sync_copy_string(out_member->email, sizeof(out_member->email), query_result_text(result, 0, 2));

    query_result_destroy(result);
    return SS_OK;
}
