CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Iinclude
LDFLAGS = -lpq
TARGET = snack_sync.exe

SOURCES = \
	src/main.c \
	src/datastructures/order_cart.c \
	src/datastructures/menu_bst.c \
	src/db/database.c \
	src/db/schema.c \
	src/repositories/admin_repository.c \
	src/repositories/member_repository.c \
	src/repositories/restaurant_repository.c \
	src/repositories/menu_repository.c \
	src/repositories/order_repository.c \
	src/services/admin_service.c \
	src/services/member_service.c \
	src/frontend/ui.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	del /Q $(TARGET) 2>nul || exit 0
