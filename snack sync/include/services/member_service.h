#ifndef SNACK_SYNC_MEMBER_SERVICE_H
#define SNACK_SYNC_MEMBER_SERVICE_H

#include <stddef.h>

#include "datastructures/order_cart.h"
#include "models.h"
#include "repositories/member_repository.h"
#include "repositories/menu_repository.h"
#include "repositories/order_repository.h"
#include "repositories/restaurant_repository.h"

typedef struct {
    MemberRepository member_repository;
    RestaurantRepository restaurant_repository;
    MenuRepository menu_repository;
    OrderRepository order_repository;
} MemberService;

void member_service_init(MemberService *service, Database *database);
int member_service_register(
        MemberService *service,
        const char *name,
        const char *email,
        const char *password,
        Member *out_member,
        char *error,
        size_t error_size
);
int member_service_login(
        MemberService *service,
        const char *email,
        const char *password,
        Member *out_member,
        char *error,
        size_t error_size
);
int member_service_get_restaurants(
        MemberService *service,
        RestaurantList *out_list,
        char *error,
        size_t error_size
);
int member_service_get_restaurant(
        MemberService *service,
        int restaurant_id,
        Restaurant *out_restaurant,
        char *error,
        size_t error_size
);
int member_service_get_available_menu_items(
        MemberService *service,
        int restaurant_id,
        MenuItemList *out_list,
        char *error,
        size_t error_size
);
int member_service_get_available_menu_item(
        MemberService *service,
        int restaurant_id,
        int item_id,
        MenuItem *out_item,
        char *error,
        size_t error_size
);
int member_service_place_order(
        MemberService *service,
        int member_id,
        int restaurant_id,
        const OrderCart *cart,
        int *out_order_id,
        char *error,
        size_t error_size
);
int member_service_get_orders(
        MemberService *service,
        int member_id,
        OrderRecordList *out_list,
        char *error,
        size_t error_size
);

#endif
