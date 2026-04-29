#ifndef SNACK_SYNC_MEMBER_REPOSITORY_H
#define SNACK_SYNC_MEMBER_REPOSITORY_H

#include <stddef.h>

#include "db/database.h"
#include "models.h"

typedef struct {
    Database *database;
} MemberRepository;

void member_repository_init(MemberRepository *repository, Database *database);
int member_repository_create(
        MemberRepository *repository,
        const char *name,
        const char *email,
        const char *password,
        Member *out_member,
        char *error,
        size_t error_size
);
int member_repository_authenticate(
        MemberRepository *repository,
        const char *email,
        const char *password,
        Member *out_member,
        char *error,
        size_t error_size
);

#endif
