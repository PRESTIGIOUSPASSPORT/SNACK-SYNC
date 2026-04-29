#include <stdlib.h>

#include "datastructures/order_cart.h"

void order_cart_init(OrderCart *cart) {
    if (cart != NULL) {
        cart->head = NULL;
    }
}

void order_cart_destroy(OrderCart *cart) {
    OrderCartNode *current;
    OrderCartNode *next;

    if (cart == NULL) {
        return;
    }

    current = cart->head;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    cart->head = NULL;
}

void order_cart_add_or_merge(OrderCart *cart, const OrderItem *item) {
    OrderCartNode *current;
    OrderCartNode *node;

    if (cart == NULL || item == NULL) {
        return;
    }

    current = cart->head;
    while (current != NULL) {
        if (current->item.menu_item_id == item->menu_item_id) {
            current->item.quantity += item->quantity;
            return;
        }
        current = current->next;
    }

    node = (OrderCartNode *) malloc(sizeof(OrderCartNode));
    if (node == NULL) {
        return;
    }

    node->item = *item;
    node->next = NULL;

    if (cart->head == NULL) {
        cart->head = node;
        return;
    }

    current = cart->head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = node;
}

int order_cart_is_empty(const OrderCart *cart) {
    return cart == NULL || cart->head == NULL;
}

double order_cart_total(const OrderCart *cart) {
    const OrderCartNode *current;
    double total;

    if (cart == NULL) {
        return 0.0;
    }

    total = 0.0;
    current = cart->head;
    while (current != NULL) {
        total += current->item.unit_price * current->item.quantity;
        current = current->next;
    }

    return total;
}

int order_cart_foreach(const OrderCart *cart, OrderCartVisitor visitor, void *context) {
    const OrderCartNode *current;

    if (cart == NULL || visitor == NULL) {
        return SS_ERROR;
    }

    current = cart->head;
    while (current != NULL) {
        if (!visitor(&current->item, context)) {
            return SS_ERROR;
        }
        current = current->next;
    }

    return SS_OK;
}
