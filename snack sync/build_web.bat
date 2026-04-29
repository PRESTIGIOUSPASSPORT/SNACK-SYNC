@echo off
setlocal

gcc -std=c11 -Wall -Wextra -o snack_sync_web.exe src\web_server.c -lws2_32

if errorlevel 1 (
  echo Web server build failed.
  exit /b 1
)

echo Web server build complete: snack_sync_web.exe
