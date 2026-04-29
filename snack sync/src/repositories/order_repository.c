#include <stdio.h>
#include <stdlib.h>

#include "repositories/order_repository.h"

typedef struct {
    OrderRepository *repository;
    int order_id;
    char *error;
    size_t error_size;
    int status;
} OrderInsertContext;

static int insert_order_item(const OrderItem *item, void *context_pointer) {
    static const char *sql =
            "INSERT INTO order_items(order_id, menu_item_id, item_name, quantity, unit_price) "
            "VALUES ($1, $2, $3, $4, $5)";
    OrderInsertContext *context;
    char order_id_buffer[32];
    char menu_item_id_buffer[32];
    char quantity_buffer[32];
    char unit_price_buffer[32];
    const char *parameters[5];

    context = (OrderInsertContext *) context_pointer;

    snprintf(order_id_buffer, sizeof(order_id_buffer), "%d", context->order_id);
    snprintf(menu_item_id_buffer, sizeof(menu_item_id_buffer), "%d", item->menu_item_id);
    snprintf(quantity_buffer, sizeof(quantity_buffer), "%d", item->quantity);
    snprintf(unit_price_buffer, sizeof(unit_price_buffer), "%.2f", item->unit_price);

    parameters[0] = order_id_buffer;
    parameters[1] = menu_item_id_buffer;
    parameters[2] = item->item_name;
    parameters[3] = quantity_buffer;
    parameters[4] = unit_price_buffer;

    context->status = database_command(
            context->repository->database,
            sql,
            5,
            parameters,
            context->error,
            context->error_size
    );
    return context->status == SS_OK;
}

void order_repository_init(OrderRepository *repository, Database *database) {
    repository->database = database;
}

int order_repository_create_order(
        OrderRepository *repository,
        int member_id,
        int restaurant_id,
        const OrderCart *cart,
        int *out_order_id,
        char *error,
        size_t error_size
) {
    static const char *order_sql =
            "INSERT INTO orders(member_id, restaurant_id, total_amount, status) "
            "VALUES ($1, $2, $3, $4) RETURNING id";
    char rollback_error[APP_ERROR_SIZE];
    char member_id_buffer[32];
    char restaurant_id_buffer[32];
    char total_buffer[32];
    const char *parameters[4];
    QueryResult *result;
    OrderInsertContext context;

    if (order_cart_is_empty(cart)) {
        snack_sync_copy_string(error, error_size, "Cannot place an empty order.");
        return SS_ERROR;
    }

    if (database_begin(repository->database, error, error_size) != SS_OK) {
        return SS_ERROR;
    }

    snprintf(member_id_buffer, sizeof(member_id_buffer), "%d", member_id);
    snprintf(restaurant_id_buffer, sizeof(restaurant_id_buffer), "%d", restaurant_id);
    snprintf(total_buffer, sizeof(total_buffer), "%.2f", order_cart_total(cart));

    parameters[0] = member_id_buffer;
    parameters[1] = restaurant_id_buffer;
    parameters[2] = total_buffer;
    parameters[3] = "PLACED";

    result = database_query(repository->database, order_sql, 4, parameters, error, error_size);
    if (result == NULL) {
        database_rollback(repository->database, rollback_error, sizeof(rollback_error));
        return SS_ERROR;
    }

    if (query_result_rows(result) == 0) {
        query_result_destroy(result);
        snack_sync_copy_string(error, error_size, "Order creation did not return a new ID.");
        database_rollback(repository->database, rollback_error, sizeof(rollback_error));
        return SS_ERROR;
    }

    *out_order_id = query_result_int(result, 0, 0);
    query_result_destroy(result);

    context.repository = repository;
    context.order_id = *out_order_id;
    context.error = error;
    context.error_size = error_size;
    context.status = SS_OK;

    if (order_cart_foreach(cart, insert_order_item, &context) != SS_OK) {
        database_rollback(repository->database, rollback_error, sizeof(rollback_error));
        return SS_ERROR;
    }

    if (database_commit(repository->database, error, error_size) != SS_OK) {
        database_rollback(repository->database, rollback_error, sizeof(rollback_error));
        return SS_ERROR;
    }

    return SS_OK;
}

int order_repository_find_by_member_id(
        OrderRepository *repository,
        int member_id,
        OrderRecordList *out_list,
        char *error,
        size_t error_size
) {
    static const char *sql =
            "SELECT o.id, r.name, o.total_amount, o.status, "
            "TO_CHAR(o.created_at, 'YYYY-MM-DD HH24:MI:SS') "
            "FROM orders o "
            "INNER JOIN restaurants r ON r.id = o.restaurant_id "
            "WHERE o.member_id = $1 "
            "ORDER BY o.created_at DESC";
    char member_id_buffer[32];
    const char *parameters[1];
    QueryResult *result;
    int row_count;
    int index;

    out_list->items = NULL;
    out_list->count = 0;

    snprintf(member_id_buffer, sizeof(member_id_buffer), "%d", member_id);
    parameters[0] = member_id_buffer;

    result = database_query(repository->database, sql, 1, parameters, error, error_size);
    if (result == NULL) {
        return SS_ERROR;
    }

    row_count = query_result_rows(result);
    if (row_count > 0) {
        out_list->items = (OrderRecord *) calloc((size_t) row_count, sizeof(OrderRecord));
        if (out_list->items == NULL) {
            query_result_destroy(result);
            snack_sync_copy_string(error, error_size, "Out of memory while loading orders.");
            return SS_ERROR;
        }

        for (index = 0; index < row_count; ++index) {
            out_list->items[index].id = query_result_int(result, index, 0);
            snack_sync_copy_string(
                    out_list->items[index].restaurant_name,
                    sizeof(out_list->items[index].restaurant_name),
                    query_result_text(result, index, 1)
            );
            out_list->items[index].total_amount = query_result_double(result, index, 2);
            snack_sync_copy_string(
                    out_list->items[index].status,
                    sizeof(out_list->items[index].status),
                    query_result_text(result, index, 3)
            );
            snack_sync_copy_string(
                    out_list->items[index].created_at,
                    sizeof(out_list->items[index].created_at),
                    query_result_text(result, index, 4)
            );
        }
    }

    out_list->count = row_count;
    query_result_destroy(result);
    return SS_OK;
}
