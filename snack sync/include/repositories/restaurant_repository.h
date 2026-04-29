#ifndef SNACK_SYNC_RESTAURANT_REPOSITORY_H
#define SNACK_SYNC_RESTAURANT_REPOSITORY_H

#include <stddef.h>

#include "db/database.h"
#include "models.h"

typedef struct {
    Database *database;
} RestaurantRepository;

void restaurant_repository_init(RestaurantRepository *repository, Database *database);
int restaurant_repository_create(
        RestaurantRepository *repository,
        int admin_id,
        const char *name,
        const char *cuisine,
        const char *address,
        int *out_restaurant_id,
        char *error,
        size_t error_size
);
int restaurant_repository_find_by_id(
        RestaurantRepository *repository,
        int restaurant_id,
        Restaurant *out_restaurant,
        char *error,
        size_t error_size
);
int restaurant_repository_find_by_admin_id(
        RestaurantRepository *repository,
        int admin_id,
        RestaurantList *out_list,
        char *error,
        size_t error_size
);
int restaurant_repository_find_all(
        RestaurantRepository *repository,
        RestaurantList *out_list,
        char *error,
        size_t error_size
);

#endif
