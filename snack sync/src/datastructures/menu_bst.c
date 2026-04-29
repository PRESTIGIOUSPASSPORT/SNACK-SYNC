#include <ctype.h>
#include <stdlib.h>

#include "datastructures/menu_bst.h"

static MenuTreeNode *create_node(const MenuItem *item) {
    MenuTreeNode *node;

    node = (MenuTreeNode *) malloc(sizeof(MenuTreeNode));
    if (node == NULL) {
        return NULL;
    }

    node->item = *item;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static int compare_case_insensitive(const char *left, const char *right) {
    unsigned char left_char;
    unsigned char right_char;

    while (*left != '\0' && *right != '\0') {
        left_char = (unsigned char) tolower((unsigned char) *left);
        right_char = (unsigned char) tolower((unsigned char) *right);
        if (left_char != right_char) {
            return (int) left_char - (int) right_char;
        }
        ++left;
        ++right;
    }

    return (int) tolower((unsigned char) *left) - (int) tolower((unsigned char) *right);
}

static int compare_menu_items(const MenuItem *left, const MenuItem *right) {
    int by_name;

    by_name = compare_case_insensitive(left->name, right->name);
    if (by_name != 0) {
        return by_name;
    }

    if (left->id < right->id) {
        return -1;
    }
    if (left->id > right->id) {
        return 1;
    }
    return 0;
}

static MenuTreeNode *insert_recursive(MenuTreeNode *root, const MenuItem *item) {
    int comparison;

    if (root == NULL) {
        return create_node(item);
    }

    comparison = compare_menu_items(item, &root->item);
    if (comparison < 0) {
        root->left = insert_recursive(root->left, item);
    } else {
        root->right = insert_recursive(root->right, item);
    }

    return root;
}

static void destroy_recursive(MenuTreeNode *root) {
    if (root == NULL) {
        return;
    }

    destroy_recursive(root->left);
    destroy_recursive(root->right);
    free(root);
}

static const MenuItem *search_recursive(const MenuTreeNode *root, const char *name) {
    int comparison;

    if (root == NULL) {
        return NULL;
    }

    comparison = compare_case_insensitive(name, root->item.name);
    if (comparison == 0) {
        return &root->item;
    }
    if (comparison < 0) {
        return search_recursive(root->left, name);
    }
    return search_recursive(root->right, name);
}

static void in_order_recursive(const MenuTreeNode *root, MenuItemVisitor visitor, void *context) {
    if (root == NULL) {
        return;
    }

    in_order_recursive(root->left, visitor, context);
    visitor(&root->item, context);
    in_order_recursive(root->right, visitor, context);
}

void menu_bst_init(MenuBST *tree) {
    if (tree != NULL) {
        tree->root = NULL;
    }
}

void menu_bst_destroy(MenuBST *tree) {
    if (tree == NULL) {
        return;
    }

    destroy_recursive(tree->root);
    tree->root = NULL;
}

void menu_bst_insert(MenuBST *tree, const MenuItem *item) {
    if (tree == NULL || item == NULL) {
        return;
    }

    tree->root = insert_recursive(tree->root, item);
}

const MenuItem *menu_bst_search(const MenuBST *tree, const char *name) {
    if (tree == NULL || name == NULL || *name == '\0') {
        return NULL;
    }

    return search_recursive(tree->root, name);
}

void menu_bst_in_order(const MenuBST *tree, MenuItemVisitor visitor, void *context) {
    if (tree == NULL || visitor == NULL) {
        return;
    }

    in_order_recursive(tree->root, visitor, context);
}
