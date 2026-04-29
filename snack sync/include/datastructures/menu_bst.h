#ifndef SNACK_SYNC_MENU_BST_H
#define SNACK_SYNC_MENU_BST_H

#include "models.h"

typedef struct MenuTreeNode {
    MenuItem item;
    struct MenuTreeNode *left;
    struct MenuTreeNode *right;
} MenuTreeNode;

typedef struct {
    MenuTreeNode *root;
} MenuBST;

typedef void (*MenuItemVisitor)(const MenuItem *item, void *context);

void menu_bst_init(MenuBST *tree);
void menu_bst_destroy(MenuBST *tree);
void menu_bst_insert(MenuBST *tree, const MenuItem *item);
const MenuItem *menu_bst_search(const MenuBST *tree, const char *name);
void menu_bst_in_order(const MenuBST *tree, MenuItemVisitor visitor, void *context);

#endif
