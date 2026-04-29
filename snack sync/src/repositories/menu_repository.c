#include <stdio.h>
#include <stdlib.h>

#include "repositories/menu_repository.h"

static void map_menu_item_row(const QueryResult *result, int row, MenuItem *item) {
    item->id = query_result_int(result, row, 0);
    item->restaurant_id = query_result_int(result, row, 1);
    snack_sync_copy_string(item->name, sizeof(item->name), query_result_text(result, row, 2));
    snack_sync_copy_string(item->description, sizeof(item->description), query_result_text(result, row, 3));
    item->price = query_result_double(result, row, 4);
    item->available = query_result_bool(result, row, 5);
}

static int query_menu_list(
        MenuRepository *repository,
        const char *sql,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
) {
    char restaurant_id_buffer[32];
    const char *parameters[1];
    QueryResult *result;
    int row_count;
    int index;

    out_list->items = NULL;
    out_list->count = 0;

    snprintf(restaurant_id_buffer, sizeof(restaurant_id_buffer), "%d", restaurant_id);
    parameters[0] = restaurant_id_buffer;

    result = database_query(repository->database, sql, 1, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    row_count = query_result_rows(result);
    if (row_count > 0) {
        out_list->items = (MenuItem *) calloc((size_t) row_count, sizeof(MenuItem));
        if (out_list->items == NULL) {
            query_result_destroy(result);
            snack_sync_copy_string(error, error_size, "Out of memory while loading menu items.");
            return SS_ERROR;
        }

        for (index = 0; index < row_count; ++index) {
            map_menu_item_row(result, index, &out_list->items[index]);
        }
    }

    out_list->count = row_count;
    query_result_destroy(result);
    return SS_OK;
}

void menu_repository_init(MenuRepository *repository, Database *database) {
    repository->database = database;
}

int menu_repository_create(
        MenuRepository *repository,
        int restaurant_id,
        const char *name,
        const char *description,
        double price,
        int available,
        int *out_item_id,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "INSERT INTO menu_items(restaurant_id, name, description, price, available) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id";
    char restaurant_id_buffer[32];
    char price_buffer[32];
    char availability_buffer[8];
    const char *parameters[5];
    QueryResult *result;

    snprintf(restaurant_id_buffer, sizeof(restaurant_id_buffer), "%d", restaurant_id);
    snprintf(price_buffer, sizeof(price_buffer), "%.2f", price);
    snack_sync_copy_string(availability_buffer, sizeof(availability_buffer), available ? "true" : "false");

    parameters[0] = restaurant_id_buffer;
    parameters[1] = name;
    parameters[2] = description;
    parameters[3] = price_buffer;
    parameters[4] = availability_buffer;

    result = database_query(repository->database, sql, 5, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    if (query_result_rows(result) == 0) {
        query_result_destroy(result);
        snack_sync_copy_string(error, error_size, "Menu item creation did not return a new ID.");
        return SS_ERROR;
    }

    *out_item_id = query_result_int(result, 0, 0);
    query_result_destroy(result);
    return SS_OK;
}

int menu_repository_find_by_restaurant_id(
        MenuRepository *repository,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, restaurant_id, name, description, price, available "
            "FROM menu_items WHERE restaurant_id = $1 ORDER BY name";
    return query_menu_list(repository, sql, restaurant_id, out_list, error, error_size);
}

int menu_repository_find_available_by_restaurant_id(
        MenuRepository *repository,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, restaurant_id, name, description, price, available "
            "FROM menu_items WHERE restaurant_id = $1 AND available = TRUE ORDER BY name";
    return query_menu_list(repository, sql, restaurant_id, out_list, error, error_size);
}

int menu_repository_find_by_restaurant_and_item_id(
        MenuRepository *repository,
        int restaurant_id,
        int item_id,
        MenuItem *out_item,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, restaurant_id, name, description, price, available "
            "FROM menu_items WHERE restaurant_id = $1 AND id = $2";
    char restaurant_id_buffer[32];
    char item_id_buffer[32];
    const char *parameters[2];
    QueryResult *result;

    snprintf(restaurant_id_buffer, sizeof(restaurant_id_buffer), "%d", restaurant_id);
    snprintf(item_id_buffer, sizeof(item_id_buffer), "%d", item_id);
    parameters[0] = restaurant_id_buffer;
    parameters[1] = item_id_buffer;

    result = database_query(repository->database, sql, 2, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    if (query_result_rows(result) == 0) {
        query_result_destroy(result);
        snack_sync_copy_string(error, error_size, "");
        return SS_NOT_FOUND;
    }

    map_menu_item_row(result, 0, out_item);
    query_result_destroy(result);
    return SS_OK;
}

int menu_repository_find_available_by_restaurant_and_item_id(
        MenuRepository *repository,
        int restaurant_id,
        int item_id,
        MenuItem *out_item,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT id, restaurant_id, name, description, price, available "
            "FROM menu_items WHERE restaurant_id = $1 AND id = $2 AND available = TRUE";
    char restaurant_id_buffer[32];
    char item_id_buffer[32];
    const char *parameters[2];
    QueryResult *result;

    snprintf(restaurant_id_buffer, sizeof(restaurant_id_buffer), "%d", restaurant_id);
    snprintf(item_id_buffer, sizeof(item_id_buffer), "%d", item_id);
    parameters[0] = restaurant_id_buffer;
    parameters[1] = item_id_buffer;

    result = database_query(repository->database, sql, 2, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    if (query_result_rows(result) == 0) {
        query_result_destroy(result);
        snack_sync_copy_string(error, error_size, "");
        return SS_NOT_FOUND;
    }

    map_menu_item_row(result, 0, out_item);
    query_result_destroy(result);
    return SS_OK;
}

int menu_repository_update_availability(
        MenuRepository *repository,
        int item_id,
        int available,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "UPDATE menu_items SET available = $1 WHERE id = $2";
    char item_id_buffer[32];
    char availability_buffer[8];
    const char *parameters[2];

    snack_sync_copy_string(availability_buffer, sizeof(availability_buffer), available ? "true" : "false");
    snprintf(item_id_buffer, sizeof(item_id_buffer), "%d", item_id);

    parameters[0] = availability_buffer;
    parameters[1] = item_id_buffer;

    return database_command(repository->database, sql, 2, parameters, error, error_size);
}
