#ifndef SNACK_SYNC_ADMIN_SERVICE_H
#define SNACK_SYNC_ADMIN_SERVICE_H

#include <stddef.h>

#include "models.h"
#include "repositories/admin_repository.h"
#include "repositories/menu_repository.h"
#include "repositories/restaurant_repository.h"

typedef struct {
    AdminRepository admin_repository;
    RestaurantRepository restaurant_repository;
    MenuRepository menu_repository;
} AdminService;

void admin_service_init(AdminService *service, Database *database);
int admin_service_register(
        AdminService *service,
        const char *name,
        const char *email,
        const char *password,
        Admin *out_admin,
        char *error,
        size_t error_size
);
int admin_service_login(
        AdminService *service,
        const char *email,
        const char *password,
        Admin *out_admin,
        char *error,
        size_t error_size
);
int admin_service_create_restaurant(
        AdminService *service,
        int admin_id,
        const char *name,
        const char *cuisine,
        const char *address,
        int *out_restaurant_id,
        char *error,
        size_t error_size
);
int admin_service_get_restaurant(
        AdminService *service,
        int restaurant_id,
        Restaurant *out_restaurant,
        char *error,
        size_t error_size
);
int admin_service_get_restaurants_by_admin(
        AdminService *service,
        int admin_id,
        RestaurantList *out_list,
        char *error,
        size_t error_size
);
int admin_service_add_menu_item(
        AdminService *service,
        int restaurant_id,
        const char *name,
        const char *description,
        double price,
        int available,
        int *out_item_id,
        char *error,
        size_t error_size
);
int admin_service_get_menu_for_restaurant(
        AdminService *service,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
);
int admin_service_get_menu_item_for_restaurant(
        AdminService *service,
        int restaurant_id,
        int item_id,
        MenuItem *out_item,
        char *error,
        size_t error_size
);
int admin_service_update_menu_item_availability(
        AdminService *service,
        int item_id,
        int available,
        char *error,
        size_t error_size
);

#endif
