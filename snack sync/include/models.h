#ifndef SNACK_SYNC_MODELS_H
#define SNACK_SYNC_MODELS_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define APP_ERROR_SIZE 512
#define NAME_SIZE 128
#define EMAIL_SIZE 128
#define PASSWORD_SIZE 128
#define CUISINE_SIZE 96
#define ADDRESS_SIZE 256
#define DESCRIPTION_SIZE 256
#define STATUS_SIZE 48
#define DATETIME_SIZE 64

#define SS_OK 1
#define SS_NOT_FOUND 0
#define SS_ERROR -1

typedef struct {
    int id;
    char name[NAME_SIZE];
    char email[EMAIL_SIZE];
} Admin;

typedef struct {
    int id;
    char name[NAME_SIZE];
    char email[EMAIL_SIZE];
} Member;

typedef struct {
    int id;
    int admin_id;
    char name[NAME_SIZE];
    char cuisine[CUISINE_SIZE];
    char address[ADDRESS_SIZE];
} Restaurant;

typedef struct {
    int id;
    int restaurant_id;
    char name[NAME_SIZE];
    char description[DESCRIPTION_SIZE];
    double price;
    int available;
} MenuItem;

typedef struct {
    int menu_item_id;
    char item_name[NAME_SIZE];
    int quantity;
    double unit_price;
} OrderItem;

typedef struct {
    int id;
    char restaurant_name[NAME_SIZE];
    double total_amount;
    char status[STATUS_SIZE];
    char created_at[DATETIME_SIZE];
} OrderRecord;

typedef struct {
    Restaurant *items;
    int count;
} RestaurantList;

typedef struct {
    MenuItem *items;
    int count;
} MenuItemList;

typedef struct {
    OrderRecord *items;
    int count;
} OrderRecordList;

static inline void snack_sync_copy_string(char *destination, size_t destination_size, const char *source) {
    if (destination_size == 0) {
        return;
    }
    snprintf(destination, destination_size, "%s", source == NULL ? "" : source);
}

static inline void restaurant_list_free(RestaurantList *list) {
    if (list == NULL) {
        return;
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
}

static inline void menu_item_list_free(MenuItemList *list) {
    if (list == NULL) {
        return;
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
}

static inline void order_record_list_free(OrderRecordList *list) {
    if (list == NULL) {
        return;
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
}

#endif
