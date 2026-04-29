#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "datastructures/menu_bst.h"
#include "datastructures/order_cart.h"
#include "frontend/ui.h"
#include "models.h"

typedef struct {
    AdminService *admin_service;
    MemberService *member_service;
} UIContext;

static void trim_newline(char *text) {
    size_t length;

    if (text == NULL) {
        return;
    }

    length = strlen(text);
    while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        text[length - 1] = '\0';
        --length;
    }
}

static void print_header(const char *title) {
    printf("\n========================================\n");
    printf("%s\n", title);
    printf("========================================\n");
}

static void print_error_message(const char *error) {
    if (error != NULL && *error != '\0') {
        printf("Error: %s\n", error);
    }
}

static void read_line(const char *prompt, char *buffer, size_t buffer_size) {
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(buffer, (int) buffer_size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }

    trim_newline(buffer);
}

static void read_required_text(const char *prompt, char *buffer, size_t buffer_size) {
    do {
        read_line(prompt, buffer, buffer_size);
        if (buffer[0] == '\0') {
            printf("This field cannot be empty.\n");
        }
    } while (buffer[0] == '\0');
}

static int read_int(const char *prompt) {
    char buffer[64];
    char *end_pointer;
    long value;

    while (1) {
        read_line(prompt, buffer, sizeof(buffer));
        value = strtol(buffer, &end_pointer, 10);
        if (end_pointer != buffer && *end_pointer == '\0') {
            return (int) value;
        }
        printf("Please enter a valid whole number.\n");
    }
}

static int read_positive_int(const char *prompt) {
    int value;

    while (1) {
        value = read_int(prompt);
        if (value > 0) {
            return value;
        }
        printf("Please enter a number greater than zero.\n");
    }
}

static double read_non_negative_double(const char *prompt) {
    char buffer[64];
    char *end_pointer;
    double value;

    while (1) {
        read_line(prompt, buffer, sizeof(buffer));
        value = strtod(buffer, &end_pointer);
        if (end_pointer != buffer && *end_pointer == '\0' && value >= 0.0) {
            return value;
        }
        printf("Please enter a valid non-negative number.\n");
    }
}

static int read_yes_no(const char *prompt) {
    char buffer[16];

    while (1) {
        read_line(prompt, buffer, sizeof(buffer));
        if (strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0 ||
            strcmp(buffer, "yes") == 0 || strcmp(buffer, "YES") == 0 ||
            strcmp(buffer, "Yes") == 0) {
            return 1;
        }
        if (strcmp(buffer, "n") == 0 || strcmp(buffer, "N") == 0 ||
            strcmp(buffer, "no") == 0 || strcmp(buffer, "NO") == 0 ||
            strcmp(buffer, "No") == 0) {
            return 0;
        }
        printf("Please answer with y or n.\n");
    }
}

static const char *availability_label(int available) {
    return available ? "Available" : "Unavailable";
}

static void print_restaurants(const RestaurantList *restaurants) {
    int index;

    for (index = 0; index < restaurants->count; ++index) {
        printf(
                "ID: %d | Name: %s | Cuisine: %s | Address: %s\n",
                restaurants->items[index].id,
                restaurants->items[index].name,
                restaurants->items[index].cuisine,
                restaurants->items[index].address
        );
    }
}

static void print_menu_line(const MenuItem *item) {
    printf(
            "ID: %d | Name: %s | Price: Rs. %.2f | Status: %s | Description: %s\n",
            item->id,
            item->name,
            item->price,
            availability_label(item->available),
            item->description[0] == '\0' ? "No description" : item->description
    );
}

static void print_menu_item_callback(const MenuItem *item, void *context) {
    (void) context;
    print_menu_line(item);
}

static int print_cart_item_callback(const OrderItem *item, void *context) {
    (void) context;
    printf(
            "%s | Qty: %d | Unit Price: Rs. %.2f | Subtotal: Rs. %.2f\n",
            item->item_name,
            item->quantity,
            item->unit_price,
            item->unit_price * item->quantity
    );
    return 1;
}

static void build_menu_tree(const MenuItemList *items, MenuBST *tree) {
    int index;

    menu_bst_init(tree);
    for (index = 0; index < items->count; ++index) {
        menu_bst_insert(tree, &items->items[index]);
    }
}

static int choose_owned_restaurant(UIContext *context, const Admin *admin, const char *action, Restaurant *out_restaurant) {
    char error[APP_ERROR_SIZE];
    RestaurantList restaurants;
    int restaurant_id;
    int status;

    restaurants.items = NULL;
    restaurants.count = 0;

    status = admin_service_get_restaurants_by_admin(
            context->admin_service,
            admin->id,
            &restaurants,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return SS_ERROR;
    }

    if (restaurants.count == 0) {
        printf("Create a restaurant before trying to %s.\n", action);
        restaurant_list_free(&restaurants);
        return SS_NOT_FOUND;
    }

    print_restaurants(&restaurants);
    restaurant_id = read_int("Enter restaurant ID: ");

    status = admin_service_get_restaurant(
            context->admin_service,
            restaurant_id,
            out_restaurant,
            error,
            sizeof(error)
    );
    restaurant_list_free(&restaurants);

    if (status == SS_ERROR) {
        print_error_message(error);
        return SS_ERROR;
    }
    if (status == SS_NOT_FOUND || out_restaurant->admin_id != admin->id) {
        printf("That restaurant does not belong to this admin.\n");
        return SS_NOT_FOUND;
    }

    return SS_OK;
}

static int choose_any_restaurant(UIContext *context, const char *action, Restaurant *out_restaurant) {
    char error[APP_ERROR_SIZE];
    RestaurantList restaurants;
    int restaurant_id;
    int status;

    restaurants.items = NULL;
    restaurants.count = 0;

    status = member_service_get_restaurants(context->member_service, &restaurants, error, sizeof(error));
    if (status != SS_OK) {
        print_error_message(error);
        return SS_ERROR;
    }

    if (restaurants.count == 0) {
        printf("No restaurants are available to %s.\n", action);
        restaurant_list_free(&restaurants);
        return SS_NOT_FOUND;
    }

    print_restaurants(&restaurants);
    restaurant_id = read_int("Enter restaurant ID: ");

    status = member_service_get_restaurant(
            context->member_service,
            restaurant_id,
            out_restaurant,
            error,
            sizeof(error)
    );
    restaurant_list_free(&restaurants);

    if (status == SS_ERROR) {
        print_error_message(error);
        return SS_ERROR;
    }
    if (status == SS_NOT_FOUND) {
        printf("Restaurant not found.\n");
        return SS_NOT_FOUND;
    }

    return SS_OK;
}

static void show_admin_restaurants(UIContext *context, const Admin *admin) {
    char error[APP_ERROR_SIZE];
    RestaurantList restaurants;
    int status;

    restaurants.items = NULL;
    restaurants.count = 0;

    status = admin_service_get_restaurants_by_admin(
            context->admin_service,
            admin->id,
            &restaurants,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    if (restaurants.count == 0) {
        printf("No restaurants found for this admin.\n");
    } else {
        print_restaurants(&restaurants);
    }

    restaurant_list_free(&restaurants);
}

static void add_restaurant(UIContext *context, const Admin *admin) {
    char error[APP_ERROR_SIZE];
    char name[NAME_SIZE];
    char cuisine[CUISINE_SIZE];
    char address[ADDRESS_SIZE];
    int restaurant_id;
    int status;

    read_required_text("Restaurant name: ", name, sizeof(name));
    read_required_text("Cuisine type: ", cuisine, sizeof(cuisine));
    read_required_text("Address: ", address, sizeof(address));

    status = admin_service_create_restaurant(
            context->admin_service,
            admin->id,
            name,
            cuisine,
            address,
            &restaurant_id,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    printf("Restaurant created successfully with ID %d.\n", restaurant_id);
}

static void add_menu_item(UIContext *context, const Admin *admin) {
    char error[APP_ERROR_SIZE];
    Restaurant restaurant;
    char name[NAME_SIZE];
    char description[DESCRIPTION_SIZE];
    double price;
    int available;
    int item_id;
    int status;

    status = choose_owned_restaurant(context, admin, "add a menu item", &restaurant);
    if (status != SS_OK) {
        return;
    }

    read_required_text("Menu item name: ", name, sizeof(name));
    read_line("Description (optional): ", description, sizeof(description));
    price = read_non_negative_double("Price: ");
    available = read_yes_no("Available now? (y/n): ");

    status = admin_service_add_menu_item(
            context->admin_service,
            restaurant.id,
            name,
            description,
            price,
            available,
            &item_id,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    printf("Menu item created successfully with ID %d.\n", item_id);
}

static void show_admin_restaurant_menu(UIContext *context, const Admin *admin) {
    char error[APP_ERROR_SIZE];
    Restaurant restaurant;
    MenuItemList items;
    int status;
    int index;

    items.items = NULL;
    items.count = 0;

    status = choose_owned_restaurant(context, admin, "view the menu", &restaurant);
    if (status != SS_OK) {
        return;
    }

    status = admin_service_get_menu_for_restaurant(
            context->admin_service,
            restaurant.id,
            &items,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    if (items.count == 0) {
        printf("This restaurant has no menu items yet.\n");
    } else {
        for (index = 0; index < items.count; ++index) {
            print_menu_line(&items.items[index]);
        }
    }

    menu_item_list_free(&items);
}

static void update_menu_item_availability(UIContext *context, const Admin *admin) {
    char error[APP_ERROR_SIZE];
    Restaurant restaurant;
    MenuItemList items;
    MenuItem item;
    int status;
    int item_id;
    int available;
    int index;

    items.items = NULL;
    items.count = 0;

    status = choose_owned_restaurant(context, admin, "update menu availability", &restaurant);
    if (status != SS_OK) {
        return;
    }

    status = admin_service_get_menu_for_restaurant(
            context->admin_service,
            restaurant.id,
            &items,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    if (items.count == 0) {
        printf("This restaurant has no menu items yet.\n");
        menu_item_list_free(&items);
        return;
    }

    for (index = 0; index < items.count; ++index) {
        print_menu_line(&items.items[index]);
    }

    item_id = read_int("Enter menu item ID to update: ");
    status = admin_service_get_menu_item_for_restaurant(
            context->admin_service,
            restaurant.id,
            item_id,
            &item,
            error,
            sizeof(error)
    );
    menu_item_list_free(&items);

    if (status == SS_ERROR) {
        print_error_message(error);
        return;
    }
    if (status == SS_NOT_FOUND) {
        printf("That menu item was not found for this restaurant.\n");
        return;
    }

    available = read_yes_no("Should this item be available? (y/n): ");
    status = admin_service_update_menu_item_availability(
            context->admin_service,
            item_id,
            available,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    printf("Menu item availability updated.\n");
}

static void admin_dashboard(UIContext *context, const Admin *admin) {
    int choice;

    while (1) {
        print_header("Admin Dashboard");
        printf("Signed in as: %s\n", admin->name);
        printf("1. Add restaurant\n");
        printf("2. View my restaurants\n");
        printf("3. Add menu item\n");
        printf("4. Update menu item availability\n");
        printf("5. View restaurant menu\n");
        printf("0. Logout\n");

        choice = read_int("Choose an option: ");
        switch (choice) {
            case 1:
                add_restaurant(context, admin);
                break;
            case 2:
                show_admin_restaurants(context, admin);
                break;
            case 3:
                add_menu_item(context, admin);
                break;
            case 4:
                update_menu_item_availability(context, admin);
                break;
            case 5:
                show_admin_restaurant_menu(context, admin);
                break;
            case 0:
                return;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}

static void register_admin(UIContext *context) {
    char error[APP_ERROR_SIZE];
    char name[NAME_SIZE];
    char email[EMAIL_SIZE];
    char password[PASSWORD_SIZE];
    Admin admin;
    int status;

    read_required_text("Admin name: ", name, sizeof(name));
    read_required_text("Admin email: ", email, sizeof(email));
    read_required_text("Admin password: ", password, sizeof(password));

    status = admin_service_register(
            context->admin_service,
            name,
            email,
            password,
            &admin,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    printf("Admin registered successfully with ID %d.\n", admin.id);
}

static void login_admin(UIContext *context) {
    char error[APP_ERROR_SIZE];
    char email[EMAIL_SIZE];
    char password[PASSWORD_SIZE];
    Admin admin;
    int status;

    read_required_text("Admin email: ", email, sizeof(email));
    read_required_text("Admin password: ", password, sizeof(password));

    status = admin_service_login(
            context->admin_service,
            email,
            password,
            &admin,
            error,
            sizeof(error)
    );
    if (status == SS_ERROR) {
        print_error_message(error);
        return;
    }
    if (status == SS_NOT_FOUND) {
        printf("Invalid admin credentials.\n");
        return;
    }

    admin_dashboard(context, &admin);
}

static void admin_portal(UIContext *context) {
    int choice;

    while (1) {
        print_header("Admin Portal");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("0. Back\n");

        choice = read_int("Choose an option: ");
        switch (choice) {
            case 1:
                register_admin(context);
                break;
            case 2:
                login_admin(context);
                break;
            case 0:
                return;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}

static void browse_restaurants(UIContext *context) {
    char error[APP_ERROR_SIZE];
    RestaurantList restaurants;
    int status;

    restaurants.items = NULL;
    restaurants.count = 0;

    status = member_service_get_restaurants(context->member_service, &restaurants, error, sizeof(error));
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    if (restaurants.count == 0) {
        printf("No restaurants are registered yet.\n");
    } else {
        print_restaurants(&restaurants);
    }

    restaurant_list_free(&restaurants);
}

static void show_member_restaurant_menu(UIContext *context) {
    char error[APP_ERROR_SIZE];
    Restaurant restaurant;
    MenuItemList items;
    MenuBST tree;
    const MenuItem *found_item;
    char search_name[NAME_SIZE];
    int status;

    items.items = NULL;
    items.count = 0;

    status = choose_any_restaurant(context, "view", &restaurant);
    if (status != SS_OK) {
        return;
    }

    status = member_service_get_available_menu_items(
            context->member_service,
            restaurant.id,
            &items,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    if (items.count == 0) {
        printf("This restaurant has no available menu items right now.\n");
        menu_item_list_free(&items);
        return;
    }

    build_menu_tree(&items, &tree);
    print_header("BST-Sorted Menu");
    menu_bst_in_order(&tree, print_menu_item_callback, NULL);

    if (read_yes_no("Search for an item by name? (y/n): ")) {
        read_required_text("Enter menu item name: ", search_name, sizeof(search_name));
        found_item = menu_bst_search(&tree, search_name);
        if (found_item == NULL) {
            printf("No matching item was found in the BST.\n");
        } else {
            printf("Found item:\n");
            print_menu_line(found_item);
        }
    }

    menu_bst_destroy(&tree);
    menu_item_list_free(&items);
}

static void print_order_summary(const OrderCart *cart) {
    order_cart_foreach(cart, print_cart_item_callback, NULL);
    printf("Total: Rs. %.2f\n", order_cart_total(cart));
}

static void place_order(UIContext *context, const Member *member) {
    char error[APP_ERROR_SIZE];
    Restaurant restaurant;
    MenuItemList items;
    MenuBST tree;
    OrderCart cart;
    MenuItem selected_item;
    OrderItem order_item;
    int status;
    int item_id;
    int quantity;
    int order_id;

    items.items = NULL;
    items.count = 0;
    order_cart_init(&cart);

    status = choose_any_restaurant(context, "order from", &restaurant);
    if (status != SS_OK) {
        return;
    }

    status = member_service_get_available_menu_items(
            context->member_service,
            restaurant.id,
            &items,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    if (items.count == 0) {
        printf("This restaurant has no available items to order.\n");
        menu_item_list_free(&items);
        return;
    }

    build_menu_tree(&items, &tree);
    print_header("Order Builder");
    printf("Restaurant: %s\n", restaurant.name);
    printf("Menu sorted by BST:\n");
    menu_bst_in_order(&tree, print_menu_item_callback, NULL);

    while (1) {
        item_id = read_int("Enter menu item ID to add (0 to finish): ");
        if (item_id == 0) {
            break;
        }

        status = member_service_get_available_menu_item(
                context->member_service,
                restaurant.id,
                item_id,
                &selected_item,
                error,
                sizeof(error)
        );
        if (status == SS_ERROR) {
            print_error_message(error);
            continue;
        }
        if (status == SS_NOT_FOUND) {
            printf("Invalid item ID or item is unavailable.\n");
            continue;
        }

        quantity = read_positive_int("Enter quantity: ");

        order_item.menu_item_id = selected_item.id;
        snack_sync_copy_string(order_item.item_name, sizeof(order_item.item_name), selected_item.name);
        order_item.quantity = quantity;
        order_item.unit_price = selected_item.price;

        order_cart_add_or_merge(&cart, &order_item);
        printf("Added to linked-list cart. Current total: Rs. %.2f\n", order_cart_total(&cart));
    }

    if (order_cart_is_empty(&cart)) {
        printf("Order cancelled because no items were selected.\n");
        menu_bst_destroy(&tree);
        menu_item_list_free(&items);
        order_cart_destroy(&cart);
        return;
    }

    print_header("Order Summary");
    print_order_summary(&cart);

    if (!read_yes_no("Place this order? (y/n): ")) {
        printf("Order cancelled.\n");
        menu_bst_destroy(&tree);
        menu_item_list_free(&items);
        order_cart_destroy(&cart);
        return;
    }

    status = member_service_place_order(
            context->member_service,
            member->id,
            restaurant.id,
            &cart,
            &order_id,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
    } else {
        printf("Order placed successfully with ID %d.\n", order_id);
    }

    menu_bst_destroy(&tree);
    menu_item_list_free(&items);
    order_cart_destroy(&cart);
}

static void show_member_orders(UIContext *context, const Member *member) {
    char error[APP_ERROR_SIZE];
    OrderRecordList orders;
    int status;
    int index;

    orders.items = NULL;
    orders.count = 0;

    status = member_service_get_orders(context->member_service, member->id, &orders, error, sizeof(error));
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    if (orders.count == 0) {
        printf("No orders found for this member.\n");
    } else {
        for (index = 0; index < orders.count; ++index) {
            printf(
                    "Order #%d | Restaurant: %s | Total: Rs. %.2f | Status: %s | Time: %s\n",
                    orders.items[index].id,
                    orders.items[index].restaurant_name,
                    orders.items[index].total_amount,
                    orders.items[index].status,
                    orders.items[index].created_at
            );
        }
    }

    order_record_list_free(&orders);
}

static void member_dashboard(UIContext *context, const Member *member) {
    int choice;

    while (1) {
        print_header("Member Dashboard");
        printf("Signed in as: %s\n", member->name);
        printf("1. Browse restaurants\n");
        printf("2. View restaurant menu\n");
        printf("3. Place order\n");
        printf("4. View my orders\n");
        printf("0. Logout\n");

        choice = read_int("Choose an option: ");
        switch (choice) {
            case 1:
                browse_restaurants(context);
                break;
            case 2:
                show_member_restaurant_menu(context);
                break;
            case 3:
                place_order(context, member);
                break;
            case 4:
                show_member_orders(context, member);
                break;
            case 0:
                return;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}

static void register_member(UIContext *context) {
    char error[APP_ERROR_SIZE];
    char name[NAME_SIZE];
    char email[EMAIL_SIZE];
    char password[PASSWORD_SIZE];
    Member member;
    int status;

    read_required_text("Member name: ", name, sizeof(name));
    read_required_text("Member email: ", email, sizeof(email));
    read_required_text("Member password: ", password, sizeof(password));

    status = member_service_register(
            context->member_service,
            name,
            email,
            password,
            &member,
            error,
            sizeof(error)
    );
    if (status != SS_OK) {
        print_error_message(error);
        return;
    }

    printf("Member registered successfully with ID %d.\n", member.id);
}

static void login_member(UIContext *context) {
    char error[APP_ERROR_SIZE];
    char email[EMAIL_SIZE];
    char password[PASSWORD_SIZE];
    Member member;
    int status;

    read_required_text("Member email: ", email, sizeof(email));
    read_required_text("Member password: ", password, sizeof(password));

    status = member_service_login(
            context->member_service,
            email,
            password,
            &member,
            error,
            sizeof(error)
    );
    if (status == SS_ERROR) {
        print_error_message(error);
        return;
    }
    if (status == SS_NOT_FOUND) {
        printf("Invalid member credentials.\n");
        return;
    }

    member_dashboard(context, &member);
}

static void member_portal(UIContext *context) {
    int choice;

    while (1) {
        print_header("Member Portal");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("0. Back\n");

        choice = read_int("Choose an option: ");
        switch (choice) {
            case 1:
                register_member(context);
                break;
            case 2:
                login_member(context);
                break;
            case 0:
                return;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}

void ui_run(AdminService *admin_service, MemberService *member_service) {
    UIContext context;
    int choice;

    context.admin_service = admin_service;
    context.member_service = member_service;

    print_header("Snack Sync");
    printf("Frontend: refined console flow\n");
    printf("Backend: services + repositories + PostgreSQL\n");
    printf("Data structures: BST menu index + linked-list cart\n");

    while (1) {
        print_header("Main Menu");
        printf("1. Admin portal\n");
        printf("2. Member portal\n");
        printf("0. Exit\n");

        choice = read_int("Choose an option: ");
        switch (choice) {
            case 1:
                admin_portal(&context);
                break;
            case 2:
                member_portal(&context);
                break;
            case 0:
                printf("Thanks for using Snack Sync.\n");
                return;
            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}
