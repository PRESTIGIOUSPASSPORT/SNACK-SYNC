#ifndef SNACK_SYNC_MENU_REPOSITORY_H
#define SNACK_SYNC_MENU_REPOSITORY_H

#include <stddef.h>

#include "db/database.h"
#include "models.h"

typedef struct {
    Database *database;
} MenuRepository;

void menu_repository_init(MenuRepository *repository, Database *database);
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
);
int menu_repository_find_by_restaurant_id(
        MenuRepository *repository,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
);
int menu_repository_find_available_by_restaurant_id(
        MenuRepository *repository,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
);
int menu_repository_find_by_restaurant_and_item_id(
        MenuRepository *repository,
        int restaurant_id,
        int item_id,
        MenuItem *out_item,
        char *error,
        size_t error_size
);
int menu_repository_find_available_by_restaurant_and_item_id(
        MenuRepository *repository,
        int restaurant_id,
        int item_id,
        MenuItem *out_item,
        char *error,
        size_t error_size
);
int menu_repository_update_availability(
        MenuRepository *repository,
        int item_id,
        int available,
        char *error,
        size_t error_size
);

#endif
