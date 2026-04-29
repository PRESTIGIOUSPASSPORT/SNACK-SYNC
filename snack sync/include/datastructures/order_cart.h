#ifndef SNACK_SYNC_ORDER_CART_H
#define SNACK_SYNC_ORDER_CART_H

#include "models.h"

typedef struct OrderCartNode {
    OrderItem item;
    struct OrderCartNode *next;
} OrderCartNode;

typedef struct {
    OrderCartNode *head;
} OrderCart;

typedef int (*OrderCartVisitor)(const OrderItem *item, void *context);

void order_cart_init(OrderCart *cart);
void order_cart_destroy(OrderCart *cart);
void order_cart_add_or_merge(OrderCart *cart, const OrderItem *item);
int order_cart_is_empty(const OrderCart *cart);
double order_cart_total(const OrderCart *cart);
int order_cart_foreach(const OrderCart *cart, OrderCartVisitor visitor, void *context);

#endif
