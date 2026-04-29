#ifndef SNACK_SYNC_SCHEMA_H
#define SNACK_SYNC_SCHEMA_H

#include <stddef.h>

#include "db/database.h"

int schema_initialize(Database *database, const char *schema_path, char *error, size_t error_size);

#endif
