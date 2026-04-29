#ifndef SNACK_SYNC_ORDER_REPOSITORY_H
#define SNACK_SYNC_ORDER_REPOSITORY_H

#include <stddef.h>

#include "datastructures/order_cart.h"
#include "db/database.h"
#include "models.h"

typedef struct {
    Database *database;
} OrderRepository;

void order_repository_init(OrderRepository *repository, Database *database);
int order_repository_create_order(
        OrderRepository *repository,
        int member_id,
        int restaurant_id,
        const OrderCart *cart,
        int *out_order_id,
        char *error,
        size_t error_size
);
int order_repository_find_by_member_id(
        OrderRepository *repository,
        int member_id,
        OrderRecordList *out_list,
        char *error,
        size_t error_size
);

#endif
