#include <stdio.h>
#include <stdlib.h>

#include "db/database.h"
#include "db/schema.h"
#include "frontend/ui.h"
#include "models.h"
#include "services/admin_service.h"
#include "services/member_service.h"

int main(void) {
    AdminService admin_service;
    MemberService member_service;
    Database *database;
    char error[APP_ERROR_SIZE];

    database = database_create();
    if (database == NULL) {
        fprintf(stderr, "Startup failed: could not allocate database context.\n");
        return EXIT_FAILURE;
    }

    if (database_open(database, error, sizeof(error)) != SS_OK) {
        fprintf(stderr, "Database startup failed: %s\n", error);
        database_destroy(database);
        return EXIT_FAILURE;
    }

    if (schema_initialize(database, "sql/schema.sql", error, sizeof(error)) != SS_OK) {
        fprintf(stderr, "Schema initialization failed: %s\n", error);
        database_destroy(database);
        return EXIT_FAILURE;
    }

    admin_service_init(&admin_service, database);
    member_service_init(&member_service, database);

    ui_run(&admin_service, &member_service);

    database_destroy(database);
    return EXIT_SUCCESS;
}
