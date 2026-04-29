#include "services/admin_service.h"

void admin_service_init(AdminService *service, Database *database) {
    admin_repository_init(&service->admin_repository, database);
    restaurant_repository_init(&service->restaurant_repository, database);
    menu_repository_init(&service->menu_repository, database);
}

int admin_service_register(
        AdminService *service,
        const char *name,
        const char *email,
        const char *password,
        Admin *out_admin,
        char *error,
        size_t error_size
) {
    return admin_repository_create(&service->admin_repository, name, email, password, out_admin, error, error_size);
}

int admin_service_login(
        AdminService *service,
        const char *email,
        const char *password,
        Admin *out_admin,
        char *error,
        size_t error_size
) {
    return admin_repository_authenticate(&service->admin_repository, email, password, out_admin, error, error_size);
}

int admin_service_create_restaurant(
        AdminService *service,
        int admin_id,
        const char *name,
        const char *cuisine,
        const char *address,
        int *out_restaurant_id,
        char *error,
        size_t error_size
) {
    return restaurant_repository_create(
            &service->restaurant_repository,
            admin_id,
            name,
            cuisine,
            address,
            out_restaurant_id,
            error,
            error_size
    );
}

int admin_service_get_restaurant(
        AdminService *service,
        int restaurant_id,
        Restaurant *out_restaurant,
        char *error,
        size_t error_size
) {
    return restaurant_repository_find_by_id(
            &service->restaurant_repository,
            restaurant_id,
            out_restaurant,
            error,
            error_size
    );
}

int admin_service_get_restaurants_by_admin(
        AdminService *service,
        int admin_id,
        RestaurantList *out_list,
        char *error,
        size_t error_size
) {
    return restaurant_repository_find_by_admin_id(
            &service->restaurant_repository,
            admin_id,
            out_list,
            error,
            error_size
    );
}

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
) {
    return menu_repository_create(
            &service->menu_repository,
            restaurant_id,
            name,
            description,
            price,
            available,
            out_item_id,
            error,
            error_size
    );
}

int admin_service_get_menu_for_restaurant(
        AdminService *service,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
) {
    return menu_repository_find_by_restaurant_id(
            &service->menu_repository,
            restaurant_id,
            out_list,
            error,
            error_size
    );
}

int admin_service_get_menu_item_for_restaurant(
        AdminService *service,
        int restaurant_id,
        int item_id,
        MenuItem *out_item,
        char *error,
        size_t error_size
) {
    return menu_repository_find_by_restaurant_and_item_id(
            &service->menu_repository,
            restaurant_id,
            item_id,
            out_item,
            error,
            error_size
    );
}

int admin_service_update_menu_item_availability(
        AdminService *service,
        int item_id,
        int available,
        char *error,
        size_t error_size
) {
    return menu_repository_update_availability(
            &service->menu_repository,
            item_id,
            available,
            error,
            error_size
    );
}
