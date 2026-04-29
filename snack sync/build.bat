@echo off
setlocal

if "%LIBPQ_INCLUDE%"=="" (
  echo Please set LIBPQ_INCLUDE to your PostgreSQL include folder.
  exit /b 1
)

if "%LIBPQ_LIB%"=="" (
  echo Please set LIBPQ_LIB to your PostgreSQL lib folder.
  exit /b 1
)

gcc -std=c11 -Wall -Wextra -Iinclude -I"%LIBPQ_INCLUDE%" ^
  src\main.c ^
  src\datastructures\order_cart.c ^
  src\datastructures\menu_bst.c ^
  src\db\database.c ^
  src\db\schema.c ^
  src\repositories\admin_repository.c ^
  src\repositories\member_repository.c ^
  src\repositories\restaurant_repository.c ^
  src\repositories\menu_repository.c ^
  src\repositories\order_repository.c ^
  src\services\admin_service.c ^
  src\services\member_service.c ^
  src\frontend\ui.c ^
  -L"%LIBPQ_LIB%" -lpq -o snack_sync.exe

if errorlevel 1 (
  echo Build failed.
  exit /b 1
)

echo Build complete: snack_sync.exe
