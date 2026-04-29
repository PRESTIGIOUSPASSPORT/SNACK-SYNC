#include <stdlib.h>

#include "repositories/restaurant_repository.h"

static void map_restaurant_row(const QueryResult *result, int row, Restaurant *restaurant) {
    restaurant->id = query_result_int(result, row, 0);
    restaurant->admin_id = query_result_int(result, row, 1);
    snack_sync_copy_string(restaurant->name, sizeof(restaurant->name), query_result_text(result, row, 2));
    snack_sync_copy_string(restaurant->cuisine, sizeof(restaurant->cuisine), query_result_text(result, row, 3));
    snack_sync_copy_string(restaurant->address, sizeof(restaurant->address), query_result_text(result, row, 4));
}

void restaurant_repository_init(RestaurantRepository *repository, Database *database) {
    repository->database = database;
}

int restaurant_repository_create(
        RestaurantRepository *repository,
        int admin_id,
        const char *name,
        const char *cuisine,
        const char *address,
        int *out_restaurant_id,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "INSERT INTO restaurants(admin_id, name, cuisine, address) "
            "VALUES ($1, $2, $3, $4) RETURNING id";
    char admin_id_buffer[32];
    const char *parameters[4];
    QueryResult *result;

    snprintf(admin_id_buffer, sizeof(admin_id_buffer), "%d", admin_id);
    parameters[0] = admin_id_buffer;
    parameters[1] = name;
    parameters[2] = cuisine;
    parameters[3] = address;

    result = database_query(repository->database, sql, 4, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    if (query_result_rows(result) == 0) {
        query_result_destroy(result);
        snack_sync_copy_string(error, error_size, "Restaurant creation did not return a new ID.");
        return SS_ERROR;
    }

    *out_restaurant_id = query_result_int(result, 0, 0);
    query_result_destroy(result);
    return SS_OK;
}

int restaurant_repository_find_by_id(
        RestaurantRepository *repository,
        int restaurant_id,
        Restaurant *out_restaurant,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, admin_id, name, cuisine, address "
            "FROM restaurants WHERE id = $1";
    char restaurant_id_buffer[32];
    const char *parameters[1];
    QueryResult *result;

    snprintf(restaurant_id_buffer, sizeof(restaurant_id_buffer), "%d", restaurant_id);
    parameters[0] = restaurant_id_buffer;

    result = database_query(repository->database, sql, 1, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    if (query_result_rows(result) == 0) {
        query_result_destroy(result);
        snack_sync_copy_string(error, error_size, "");
        return SS_NOT_FOUND;
    }

    map_restaurant_row(result, 0, out_restaurant);
    query_result_destroy(result);
    return SS_OK;
}

int restaurant_repository_find_by_admin_id(
        RestaurantRepository *repository,
        int admin_id,
        RestaurantList *out_list,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, admin_id, name, cuisine, address "
            "FROM restaurants WHERE admin_id = $1 ORDER BY name";
    char admin_id_buffer[32];
    const char *parameters[1];
    QueryResult *result;
    int row_count;
    int index;

    out_list->items = NULL;
    out_list->count = 0;

    snprintf(admin_id_buffer, sizeof(admin_id_buffer), "%d", admin_id);
    parameters[0] = admin_id_buffer;

    result = database_query(repository->database, sql, 1, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    row_count = query_result_rows(result);
    if (row_count > 0) {
        out_list->items = (Restaurant *) calloc((size_t) row_count, sizeof(Restaurant));
        if (out_list->items == NULL) {
            query_result_destroy(result);
            snack_sync_copy_string(error, error_size, "Out of memory while loading restaurants.");
            return SS_ERROR;
        }

        for (index = 0; index < row_count; ++index) {
            map_restaurant_row(result, index, &out_list->items[index]);
        }
    }

    out_list->count = row_count;
    query_result_destroy(result);
    return SS_OK;
}

int restaurant_repository_find_all(
        RestaurantRepository *repository,
        RestaurantList *out_list,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, admin_id, name, cuisine, address "
            "FROM restaurants ORDER BY name";
    QueryResult *result;
    int row_count;
    int index;

    out_list->items = NULL;
    out_list->count = 0;

    result = database_query(repository->database, sql, 0, NULL, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    row_count = query_result_rows(result);
    if (row_count > 0) {
        out_list->items = (Restaurant *) calloc((size_t) row_count, sizeof(Restaurant));
        if (out_list->items == NULL) {
            query_result_destroy(result);
            snack_sync_copy_string(error, error_size, "Out of memory while loading restaurants.");
            return SS_ERROR;
        }

        for (index = 0; index < row_count; ++index) {
            map_restaurant_row(result, index, &out_list->items[index]);
        }
    }

    out_list->count = row_count;
    query_result_destroy(result);
    return SS_OK;
}
