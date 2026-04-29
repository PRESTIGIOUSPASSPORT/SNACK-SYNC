# Snack Sync

Snack Sync is now a modular C console application with a clearer split between:
- `frontend` for screens, prompts, and interaction flow
- `services` for use-case logic
- `repositories` for PostgreSQL access
- `datastructures` for the linked list cart and BST menu index

## Core Features

- Admin registration and login
- Restaurant creation and menu management
- Member registration and login
- Restaurant browsing and menu search
- BST-backed menu sorting and name lookup
- Linked-list-backed cart/order building
- PostgreSQL-backed storage for users, restaurants, menu items, and orders

## Project Structure

- `include/` - public headers
- `src/frontend/` - console frontend
- `src/services/` - backend service layer
- `src/repositories/` - PostgreSQL repository layer
- `src/db/` - database connection and schema bootstrap
- `src/datastructures/` - BST and linked list
- `src/main.c` - application entry point
- `src/web_server.c` - local web server
- `sql/schema.sql` - PostgreSQL schema
- `web/` - professional HTML/CSS/JavaScript frontend
- `build.bat` - Windows build helper
- `build_web.bat` - Windows build for the local frontend server
- `Makefile` - optional GCC build

## Web Frontend

The project now also includes a polished browser frontend in `web/` with:
- `index.html` - shared login page for both admin and user
- `signup.html` - user-only sign up page
- `admin.html` - admin dashboard to add restaurants and publish menu items
- `user.html` - user dashboard to search dishes, book instantly, and prebook for a selected time
- `styles.css` - shared design system
- `app.js` - frontend state, auth, search, booking, and prebooking logic

The current web design is food-delivery inspired with:
- a shared admin/user login
- a user-only signup flow
- simple Gmail/email and password signup
- admin menu creation and availability controls
- user search, booking, and timed prebooking

### Open The Frontend

1. Build the local web server:

```powershell
.\build_web.bat
```

2. Start the server from the project root:

```powershell
.\snack_sync_web.exe
```

3. Open:

```text
http://127.0.0.1:8080/
```

4. Use the shared login page for both roles.
5. Use the signup page if you want to create a new user account.

### Demo Admin Login

- Email: `admin@snacksync.com`
- Password: `admin123`

### Frontend Storage

- The web frontend currently stores demo data in browser `localStorage`.
- That means admin-created restaurants, menu items, user signups, bookings, and prebookings persist in the browser.
- This frontend is ready to be connected to the C/PostgreSQL backend later if you want a full integrated web application.

## PostgreSQL Setup

1. Install PostgreSQL.
2. Create a database named `snack_sync`.
3. Make sure the `libpq` client library and headers are installed.
4. Optionally configure the connection:

```powershell
$env:SNACK_SYNC_DB_HOST="localhost"
$env:SNACK_SYNC_DB_PORT="5432"
$env:SNACK_SYNC_DB_NAME="snack_sync"
$env:SNACK_SYNC_DB_USER="postgres"
$env:SNACK_SYNC_DB_PASSWORD="postgres"
```

Or set a full connection string:

```powershell
$env:SNACK_SYNC_DB_CONNINFO="host=localhost port=5432 dbname=snack_sync user=postgres password=postgres"
```

## Build On Windows

Set the PostgreSQL include and lib folders if they are not already on your compiler path:

```powershell
$env:LIBPQ_INCLUDE="C:\Program Files\PostgreSQL\16\include"
$env:LIBPQ_LIB="C:\Program Files\PostgreSQL\16\lib"
.\build.bat
```

## Build With Make

```powershell
mingw32-make
```

## Notes

- Tables are created automatically on startup from `sql/schema.sql`.
- The current machine has `gcc`, but PostgreSQL headers/libraries were not present in the detected paths, so a full linked build could not be executed here.
