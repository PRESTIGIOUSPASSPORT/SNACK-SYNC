#ifndef SNACK_SYNC_ADMIN_REPOSITORY_H
#define SNACK_SYNC_ADMIN_REPOSITORY_H

#include <stddef.h>

#include "db/database.h"
#include "models.h"

typedef struct {
    Database *database;
} AdminRepository;

void admin_repository_init(AdminRepository *repository, Database *database);
int admin_repository_create(
        AdminRepository *repository,
        const char *name,
        const char *email,
        const char *password,
        Admin *out_admin,
        char *error,
        size_t error_size
);
int admin_repository_authenticate(
        AdminRepository *repository,
        const char *email,
        const char *password,
        Admin *out_admin,
        char *error,
        size_t error_size
);

#endif
