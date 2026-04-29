#include "services/member_service.h"

void member_service_init(MemberService *service, Database *database) {
    member_repository_init(&service->member_repository, database);
    restaurant_repository_init(&service->restaurant_repository, database);
    menu_repository_init(&service->menu_repository, database);
    order_repository_init(&service->order_repository, database);
}

int member_service_register(
        MemberService *service,
        const char *name,
        const char *email,
        const char *password,
        Member *out_member,
        char *error,
        size_t error_size
) {
    return member_repository_create(&service->member_repository, name, email, password, out_member, error, error_size);
}

int member_service_login(
        MemberService *service,
        const char *email,
        const char *password,
        Member *out_member,
        char *error,
        size_t error_size
) {
    return member_repository_authenticate(&service->member_repository, email, password, out_member, error, error_size);
}

int member_service_get_restaurants(
        MemberService *service,
        RestaurantList *out_list,
        char *error,
        size_t error_size
) {
    return restaurant_repository_find_all(&service->restaurant_repository, out_list, error, error_size);
}

int member_service_get_restaurant(
        MemberService *service,
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

int member_service_get_available_menu_items(
        MemberService *service,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
) {
    return menu_repository_find_available_by_restaurant_id(
            &service->menu_repository,
            restaurant_id,
            out_list,
            error,
            error_size
    );
}

int member_service_get_available_menu_item(
        MemberService *service,
        int restaurant_id,
        int item_id,
        MenuItem *out_item,
        char *error,
        size_t error_size
) {
    return menu_repository_find_available_by_restaurant_and_item_id(
            &service->menu_repository,
            restaurant_id,
            item_id,
            out_item,
            error,
            error_size
    );
}

int member_service_place_order(
        MemberService *service,
        int member_id,
        int restaurant_id,
        const OrderCart *cart,
        int *out_order_id,
        char *error,
        size_t error_size
) {
    return order_repository_create_order(
            &service->order_repository,
            member_id,
            restaurant_id,
            cart,
            out_order_id,
            error,
            error_size
    );
}

int member_service_get_orders(
        MemberService *service,
        int member_id,
        OrderRecordList *out_list,
        char *error,
        size_t error_size
) {
    return order_repository_find_by_member_id(
            &service->order_repository,
            member_id,
            out_list,
            error,
            error_size
    );
}
