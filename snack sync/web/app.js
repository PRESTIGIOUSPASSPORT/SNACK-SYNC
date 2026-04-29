const STORAGE_KEY = "snacksync-web-state-v2";
const SESSION_KEY = "snacksync-web-session-v1";

const seedState = {
    admins: [
        {
            id: "admin-1",
            name: "Aarav Mehta",
            email: "admin@snacksync.com",
            password: "admin123"
        },
        {
            id: "admin-2",
            name: "Naina Kapoor",
            email: "naina@snacksync.com",
            password: "chefdesk"
        }
    ],
    users: [],
    restaurants: [
        {
            id: "rest-1",
            adminId: "admin-1",
            name: "Urban Spice Kitchen",
            cuisine: "North Indian",
            address: "Sector 14, Noida"
        },
        {
            id: "rest-2",
            adminId: "admin-1",
            name: "Metro Wrap House",
            cuisine: "Street Food",
            address: "Rajouri Garden, Delhi"
        },
        {
            id: "rest-3",
            adminId: "admin-2",
            name: "Velvet Bean Cafe",
            cuisine: "Cafe",
            address: "Koramangala, Bengaluru"
        }
    ],
    menuItems: [
        {
            id: "item-1",
            restaurantId: "rest-1",
            name: "Smoked Paneer Tikka Bowl",
            category: "Bowls",
            description: "Charred paneer, mint rice, pickled onion, and cooling garlic dip.",
            price: 329,
            prepMinutes: 20,
            available: true
        },
        {
            id: "item-2",
            restaurantId: "rest-1",
            name: "Lucknowi Chicken Biryani",
            category: "Rice",
            description: "Fragrant basmati with tender chicken, saffron notes, and spiced raita.",
            price: 389,
            prepMinutes: 30,
            available: true
        },
        {
            id: "item-3",
            restaurantId: "rest-2",
            name: "Peri Peri Crunch Wrap",
            category: "Wraps",
            description: "Loaded wrap with crisp fries, spicy paneer, and smoky peri dressing.",
            price: 249,
            prepMinutes: 15,
            available: true
        },
        {
            id: "item-4",
            restaurantId: "rest-2",
            name: "Tandoori Soya Roll",
            category: "Rolls",
            description: "Tandoori spiced soya strips, onion slaw, and coriander crema.",
            price: 219,
            prepMinutes: 12,
            available: false
        },
        {
            id: "item-5",
            restaurantId: "rest-3",
            name: "Cold Brew Citrus Float",
            category: "Beverages",
            description: "Slow-steeped coffee with orange zest tonic and a vanilla float.",
            price: 189,
            prepMinutes: 8,
            available: true
        },
        {
            id: "item-6",
            restaurantId: "rest-3",
            name: "Cocoa Hazelnut Croissant",
            category: "Bakery",
            description: "Buttery laminated pastry filled with cocoa cream and toasted hazelnut.",
            price: 159,
            prepMinutes: 10,
            available: true
        }
    ],
    orders: []
};

let toastTimeoutId = null;
const userViewState = {
    selectedCategory: "all"
};

document.addEventListener("DOMContentLoaded", () => {
    ensureState();

    const page = document.body.dataset.page;
    if (!guardPage(page)) {
        return;
    }

    if (page === "login") {
        initLoginPage();
    } else if (page === "signup") {
        initSignupPage();
    } else if (page === "admin") {
        initAdminPage();
    } else if (page === "user") {
        initUserPage();
    }
});

function ensureState() {
    if (!localStorage.getItem(STORAGE_KEY)) {
        writeState(clone(seedState));
    }
}

function clone(value) {
    return JSON.parse(JSON.stringify(value));
}

function readState() {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
        const fresh = clone(seedState);
        writeState(fresh);
        return fresh;
    }

    try {
        const parsed = JSON.parse(raw);
        return {
            admins: Array.isArray(parsed.admins) ? parsed.admins : clone(seedState.admins),
            users: Array.isArray(parsed.users) ? parsed.users : [],
            restaurants: Array.isArray(parsed.restaurants) ? parsed.restaurants : clone(seedState.restaurants),
            menuItems: Array.isArray(parsed.menuItems) ? parsed.menuItems : clone(seedState.menuItems),
            orders: Array.isArray(parsed.orders) ? parsed.orders : []
        };
    } catch (error) {
        const fresh = clone(seedState);
        writeState(fresh);
        return fresh;
    }
}

function writeState(state) {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(state));
}

function readSession() {
    const raw = localStorage.getItem(SESSION_KEY);
    if (!raw) {
        return null;
    }

    try {
        return JSON.parse(raw);
    } catch (error) {
        localStorage.removeItem(SESSION_KEY);
        return null;
    }
}

function writeSession(session) {
    localStorage.setItem(SESSION_KEY, JSON.stringify(session));
}

function clearSession() {
    localStorage.removeItem(SESSION_KEY);
}

function guardPage(page) {
    const session = readSession();

    if ((page === "login" || page === "signup") && session) {
        redirectForRole(session.role);
        return false;
    }

    if (page === "admin" && (!session || session.role !== "admin")) {
        window.location.replace("index.html");
        return false;
    }

    if (page === "user" && (!session || session.role !== "user")) {
        window.location.replace("index.html");
        return false;
    }

    return true;
}

function redirectForRole(role) {
    window.location.replace(role === "admin" ? "admin.html" : "user.html");
}

function normalizeEmail(email) {
    return email.trim().toLowerCase();
}

function generateId(prefix) {
    return `${prefix}-${Date.now()}-${Math.floor(Math.random() * 10000)}`;
}

function getAdminById(state, id) {
    return state.admins.find((item) => item.id === id) || null;
}

function getUserById(state, id) {
    return state.users.find((item) => item.id === id) || null;
}

function getRestaurantById(state, id) {
    return state.restaurants.find((item) => item.id === id) || null;
}

function getMenuItemById(state, id) {
    return state.menuItems.find((item) => item.id === id) || null;
}

function formatCurrency(value) {
    return new Intl.NumberFormat("en-IN", {
        style: "currency",
        currency: "INR",
        maximumFractionDigits: 0
    }).format(value);
}

function formatDateTime(value) {
    const date = new Date(value);
    if (Number.isNaN(date.getTime())) {
        return value || "Not scheduled";
    }

    return new Intl.DateTimeFormat("en-IN", {
        dateStyle: "medium",
        timeStyle: "short"
    }).format(date);
}

function escapeHtml(value) {
    return String(value)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#39;");
}

function showToast(message) {
    const toast = document.getElementById("toast");
    if (!toast) {
        return;
    }

    toast.textContent = message;
    toast.classList.add("is-visible");
    window.clearTimeout(toastTimeoutId);
    toastTimeoutId = window.setTimeout(() => {
        toast.classList.remove("is-visible");
    }, 2600);
}

function setLogoutHandler() {
    const button = document.getElementById("logoutButton");
    if (!button) {
        return;
    }

    button.addEventListener("click", () => {
        clearSession();
        window.location.replace("index.html");
    });
}

function initLoginPage() {
    const roleInput = document.getElementById("loginRole");
    const roleHelpText = document.getElementById("roleHelpText");
    const tabs = document.querySelectorAll("[data-role-option]");
    const form = document.getElementById("loginForm");

    tabs.forEach((tab) => {
        tab.addEventListener("click", () => {
            const role = tab.dataset.roleOption;
            roleInput.value = role;

            tabs.forEach((button) => button.classList.toggle("active", button === tab));
            roleHelpText.textContent = role === "admin"
                ? "Admins use the same login page and move into the admin control room."
                : "New customer? Create your user account first.";
        });
    });

    form.addEventListener("submit", (event) => {
        event.preventDefault();

        const state = readState();
        const email = normalizeEmail(document.getElementById("loginEmail").value);
        const password = document.getElementById("loginPassword").value;
        const role = roleInput.value;
        const pool = role === "admin" ? state.admins : state.users;

        const account = pool.find((item) => normalizeEmail(item.email) === email && item.password === password);
        if (!account) {
            showToast(`Invalid ${role} credentials.`);
            return;
        }

        writeSession({
            role,
            userId: account.id
        });

        showToast(`Welcome back, ${account.name.split(" ")[0]}.`);
        window.setTimeout(() => redirectForRole(role), 200);
    });
}

function initSignupPage() {
    const form = document.getElementById("signupForm");
    const emailInput = document.getElementById("signupEmail");

    form.addEventListener("submit", (event) => {
        event.preventDefault();

        const state = readState();
        const name = document.getElementById("signupName").value.trim();
        const email = normalizeEmail(emailInput.value);
        const phone = document.getElementById("signupPhone").value.trim();
        const password = document.getElementById("signupPassword").value;
        const confirmPassword = document.getElementById("signupConfirmPassword").value;

        if (!name || !email || !phone || !password || !confirmPassword) {
            showToast("Please fill all signup fields.");
            return;
        }

        if (password.length < 6) {
            showToast("Password must be at least 6 characters.");
            return;
        }

        if (password !== confirmPassword) {
            showToast("Passwords do not match.");
            return;
        }

        const emailUsed = [...state.users, ...state.admins].some((item) => normalizeEmail(item.email) === email);
        if (emailUsed) {
            showToast("This email is already registered.");
            return;
        }

        const user = {
            id: generateId("user"),
            name,
            email,
            phone,
            password,
            joinedAt: new Date().toISOString()
        };

        state.users.push(user);
        writeState(state);
        writeSession({
            role: "user",
            userId: user.id
        });

        showToast("Account created successfully.");
        window.setTimeout(() => window.location.replace("user.html"), 220);
    });
}

function initAdminPage() {
    const state = readState();
    const session = readSession();
    const admin = getAdminById(state, session.userId);

    if (!admin) {
        clearSession();
        window.location.replace("index.html");
        return;
    }

    setLogoutHandler();
    document.getElementById("adminWelcome").textContent = `${admin.name}, manage your restaurant catalog.`;

    bindAdminEvents(admin);
    renderAdmin(admin);
}

function bindAdminEvents(admin) {
    document.getElementById("restaurantForm").addEventListener("submit", (event) => {
        event.preventDefault();

        const state = readState();
        const name = document.getElementById("restaurantName").value.trim();
        const cuisine = document.getElementById("restaurantCuisine").value.trim();
        const address = document.getElementById("restaurantAddress").value.trim();

        if (!name || !cuisine || !address) {
            showToast("Complete the restaurant form.");
            return;
        }

        state.restaurants.push({
            id: generateId("rest"),
            adminId: admin.id,
            name,
            cuisine,
            address
        });

        writeState(state);
        event.target.reset();
        renderAdmin(admin);
        showToast("Restaurant added.");
    });

    document.getElementById("menuItemForm").addEventListener("submit", (event) => {
        event.preventDefault();

        const state = readState();
        const restaurantId = document.getElementById("menuRestaurantSelect").value;
        const name = document.getElementById("menuItemName").value.trim();
        const category = document.getElementById("menuItemCategory").value.trim();
        const description = document.getElementById("menuItemDescription").value.trim();
        const price = Number(document.getElementById("menuItemPrice").value);
        const prepMinutes = Number(document.getElementById("menuItemPrep").value);
        const available = document.getElementById("menuItemAvailable").checked;

        if (!restaurantId || !name || !category || !description || !Number.isFinite(price) || price < 0 || !Number.isFinite(prepMinutes) || prepMinutes <= 0) {
            showToast("Complete the menu item form.");
            return;
        }

        state.menuItems.push({
            id: generateId("item"),
            restaurantId,
            name,
            category,
            description,
            price,
            prepMinutes,
            available
        });

        writeState(state);
        event.target.reset();
        document.getElementById("menuItemAvailable").checked = true;
        renderAdmin(admin);
        showToast("Menu item published.");
    });

    document.getElementById("adminMenuSearch").addEventListener("input", () => renderAdmin(admin));
    document.getElementById("adminRestaurantFilter").addEventListener("change", () => renderAdmin(admin));

    document.getElementById("adminMenuList").addEventListener("click", (event) => {
        const button = event.target.closest("[data-toggle-item]");
        if (!button) {
            return;
        }

        const state = readState();
        const item = getMenuItemById(state, button.dataset.toggleItem);
        if (!item) {
            showToast("Menu item not found.");
            return;
        }

        item.available = !item.available;
        writeState(state);
        renderAdmin(admin);
        showToast(item.available ? "Item is available again." : "Item has been paused.");
    });
}

function renderAdmin(admin) {
    const state = readState();
    const restaurants = state.restaurants.filter((restaurant) => restaurant.adminId === admin.id);
    const restaurantIds = restaurants.map((restaurant) => restaurant.id);
    const menuItems = state.menuItems.filter((item) => restaurantIds.includes(item.restaurantId));
    const orders = state.orders.filter((order) => restaurantIds.includes(order.restaurantId));

    document.getElementById("statRestaurants").textContent = String(restaurants.length);
    document.getElementById("statMenuItems").textContent = String(menuItems.length);
    document.getElementById("statBookings").textContent = String(orders.length);

    populateAdminSelects(restaurants);
    renderAdminSidebar(restaurants);
    renderAdminMenu(state, restaurants, menuItems);
    renderAdminBookings(state, orders);
}

function populateAdminSelects(restaurants) {
    const restaurantSelect = document.getElementById("menuRestaurantSelect");
    const filterSelect = document.getElementById("adminRestaurantFilter");
    const previousFilter = filterSelect.value || "all";

    const options = restaurants.map((restaurant) => `
        <option value="${escapeHtml(restaurant.id)}">${escapeHtml(restaurant.name)}</option>
    `).join("");

    restaurantSelect.innerHTML = restaurants.length ? options : `<option value="">Add a restaurant first</option>`;
    filterSelect.innerHTML = `<option value="all">All restaurants</option>${options}`;
    filterSelect.value = restaurants.some((restaurant) => restaurant.id === previousFilter) ? previousFilter : "all";
}

function renderAdminSidebar(restaurants) {
    const container = document.getElementById("adminRestaurantDeck");

    if (!restaurants.length) {
        container.innerHTML = `
            <div class="empty-state">
                <strong>No restaurants yet.</strong>
                <p>Create your first restaurant to begin publishing menu items.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = restaurants.map((restaurant) => `
        <article class="stack-card">
            <div class="stack-card__top">
                <div>
                    <strong>${escapeHtml(restaurant.name)}</strong>
                    <p>${escapeHtml(restaurant.cuisine)}</p>
                </div>
            </div>
            <div class="stack-card__meta">
                <span class="tag">${escapeHtml(restaurant.address)}</span>
            </div>
        </article>
    `).join("");
}

function renderAdminMenu(state, restaurants, menuItems) {
    const search = document.getElementById("adminMenuSearch").value.trim().toLowerCase();
    const filterRestaurantId = document.getElementById("adminRestaurantFilter").value;
    const container = document.getElementById("adminMenuList");
    const restaurantMap = new Map(restaurants.map((restaurant) => [restaurant.id, restaurant]));

    const filtered = menuItems.filter((item) => {
        const restaurant = restaurantMap.get(item.restaurantId);
        const haystack = `${item.name} ${item.category} ${item.description} ${restaurant ? restaurant.name : ""}`.toLowerCase();
        const matchesRestaurant = filterRestaurantId === "all" || item.restaurantId === filterRestaurantId;
        return matchesRestaurant && (!search || haystack.includes(search));
    });

    if (!filtered.length) {
        container.innerHTML = `
            <div class="empty-state">
                <strong>No matching dishes.</strong>
                <p>Try a different search or add more menu items.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = filtered.map((item) => {
        const restaurant = restaurantMap.get(item.restaurantId);
        return `
            <article class="stack-card">
                <div class="stack-card__top">
                    <div>
                        <strong>${escapeHtml(item.name)}</strong>
                        <p>${escapeHtml(restaurant ? restaurant.name : "Unknown restaurant")}</p>
                    </div>
                    <span class="status-pill ${item.available ? "" : "status-pill--off"}">${item.available ? "Available" : "Unavailable"}</span>
                </div>
                <div class="food-card__meta">
                    <span class="tag">${escapeHtml(item.category)}</span>
                    <span class="tag">${formatCurrency(item.price)}</span>
                    <span class="tag">${item.prepMinutes} mins prep</span>
                </div>
                <p>${escapeHtml(item.description)}</p>
                <div class="stack-card__actions">
                    <button type="button" class="status-button status-button--primary" data-toggle-item="${escapeHtml(item.id)}">
                        ${item.available ? "Pause Item" : "Make Live"}
                    </button>
                </div>
            </article>
        `;
    }).join("");
}

function renderAdminBookings(state, orders) {
    const container = document.getElementById("adminBookingFeed");
    const recent = [...orders]
        .sort((left, right) => new Date(right.createdAt).getTime() - new Date(left.createdAt).getTime())
        .slice(0, 8);

    if (!recent.length) {
        container.innerHTML = `
            <div class="empty-state">
                <strong>No bookings yet.</strong>
                <p>Customer activity will appear here once users start booking dishes.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = recent.map((order) => {
        const user = getUserById(state, order.userId);
        const restaurant = getRestaurantById(state, order.restaurantId);
        const item = getMenuItemById(state, order.menuItemId);

        return `
            <article class="stack-card">
                <div class="stack-card__top">
                    <div>
                        <strong>${escapeHtml(item ? item.name : "Dish")}</strong>
                        <p>${escapeHtml(restaurant ? restaurant.name : "Restaurant")} | ${escapeHtml(user ? user.name : "User")}</p>
                    </div>
                    <span class="status-pill status-pill--booking">${order.bookingType === "prebook" ? "Prebook" : "Instant"}</span>
                </div>
                <div class="stack-card__meta">
                    <span class="tag">Qty ${order.quantity}</span>
                    <span class="tag">${formatDateTime(order.bookingType === "prebook" ? order.scheduledAt : order.createdAt)}</span>
                </div>
            </article>
        `;
    }).join("");
}

function initUserPage() {
    const state = readState();
    const session = readSession();
    const user = getUserById(state, session.userId);

    if (!user) {
        clearSession();
        window.location.replace("index.html");
        return;
    }

    setLogoutHandler();
    document.getElementById("userWelcome").textContent = `${user.name}, find your next meal.`;

    bindUserEvents(user);
    renderUser(user);
}

function bindUserEvents(user) {
    document.getElementById("searchForm").addEventListener("submit", (event) => {
        event.preventDefault();
        renderUser(user);
    });

    document.getElementById("foodSearchInput").addEventListener("input", () => renderUser(user));
    document.getElementById("restaurantFilterSelect").addEventListener("change", () => renderUser(user));

    document.getElementById("categoryRail").addEventListener("click", (event) => {
        const button = event.target.closest("[data-category]");
        if (!button) {
            return;
        }

        userViewState.selectedCategory = button.dataset.category;
        renderUser(user);
    });

    bindBookingFlow(user);
}

function renderUser(user) {
    const state = readState();
    const restaurants = state.restaurants;
    const allVisibleItems = state.menuItems.filter((item) => item.available);
    const search = document.getElementById("foodSearchInput").value.trim().toLowerCase();
    const restaurantFilter = document.getElementById("restaurantFilterSelect");
    const previousRestaurant = restaurantFilter.value || "all";
    const userOrders = state.orders.filter((order) => order.userId === user.id);

    populateRestaurantFilter(restaurants, previousRestaurant);
    renderCategoryRail(allVisibleItems);
    renderRestaurantHighlights(restaurants, state.menuItems);

    const selectedRestaurantId = document.getElementById("restaurantFilterSelect").value;
    const filteredItems = allVisibleItems.filter((item) => {
        const restaurant = getRestaurantById(state, item.restaurantId);
        const matchesRestaurant = selectedRestaurantId === "all" || item.restaurantId === selectedRestaurantId;
        const matchesCategory = userViewState.selectedCategory === "all" || item.category === userViewState.selectedCategory;
        const haystack = `${item.name} ${item.category} ${item.description} ${restaurant ? restaurant.name : ""} ${restaurant ? restaurant.cuisine : ""}`.toLowerCase();
        return matchesRestaurant && matchesCategory && (!search || haystack.includes(search));
    });

    document.getElementById("userRestaurantCount").textContent = String(restaurants.length);
    document.getElementById("userMenuCount").textContent = String(allVisibleItems.length);
    document.getElementById("userPrebookingCount").textContent = String(
        userOrders.filter((order) => order.bookingType === "prebook").length
    );

    renderFoodResults(state, filteredItems);
    renderUpcomingOrders(state, userOrders);
    renderRecentOrders(state, userOrders);
}

function populateRestaurantFilter(restaurants, previousValue) {
    const filter = document.getElementById("restaurantFilterSelect");
    filter.innerHTML = `
        <option value="all">All restaurants</option>
        ${restaurants.map((restaurant) => `
            <option value="${escapeHtml(restaurant.id)}">${escapeHtml(restaurant.name)}</option>
        `).join("")}
    `;

    filter.value = restaurants.some((restaurant) => restaurant.id === previousValue) ? previousValue : "all";
}

function renderCategoryRail(menuItems) {
    const categories = Array.from(new Set(menuItems.map((item) => item.category))).sort();
    const container = document.getElementById("categoryRail");

    const buttons = ["all", ...categories].map((category) => {
        const active = userViewState.selectedCategory === category;
        const label = category === "all" ? "All" : category;
        return `
            <button type="button" class="category-chip ${active ? "active" : ""}" data-category="${escapeHtml(category)}">
                ${escapeHtml(label)}
            </button>
        `;
    }).join("");

    container.innerHTML = buttons;
}

function renderRestaurantHighlights(restaurants, menuItems) {
    const container = document.getElementById("restaurantHighlights");

    if (!restaurants.length) {
        container.innerHTML = `
            <div class="empty-state">
                <strong>No restaurants available.</strong>
                <p>Restaurants will appear here once admins publish them.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = restaurants.map((restaurant) => {
        const liveCount = menuItems.filter((item) => item.restaurantId === restaurant.id && item.available).length;
        return `
            <article class="restaurant-card">
                <div class="restaurant-card__art"></div>
                <div class="restaurant-card__body">
                    <strong>${escapeHtml(restaurant.name)}</strong>
                    <p>${escapeHtml(restaurant.cuisine)}</p>
                    <div class="restaurant-card__meta">
                        <span class="tag">${liveCount} live dishes</span>
                        <span class="tag">${escapeHtml(restaurant.address)}</span>
                    </div>
                </div>
            </article>
        `;
    }).join("");
}

function renderFoodResults(state, items) {
    const container = document.getElementById("foodResults");

    if (!items.length) {
        container.innerHTML = `
            <div class="empty-state">
                <strong>No dishes matched your search.</strong>
                <p>Try another dish name, category, or restaurant.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = items.map((item) => {
        const restaurant = getRestaurantById(state, item.restaurantId);
        return `
            <article class="food-card">
                <div class="food-card__top">
                    <div>
                        <strong class="food-card__title">${escapeHtml(item.name)}</strong>
                        <p>${escapeHtml(restaurant ? restaurant.name : "Restaurant")} | ${escapeHtml(restaurant ? restaurant.cuisine : "")}</p>
                    </div>
                    <span class="food-card__price">${formatCurrency(item.price)}</span>
                </div>
                <div class="food-card__meta">
                    <span class="tag">${escapeHtml(item.category)}</span>
                    <span class="tag">${item.prepMinutes} mins prep</span>
                    <span class="tag">${escapeHtml(restaurant ? restaurant.address : "")}</span>
                </div>
                <p>${escapeHtml(item.description)}</p>
                <div class="food-card__actions">
                    <button type="button" class="cta-button" data-book-item="${escapeHtml(item.id)}" data-book-type="instant">Book Now</button>
                    <button type="button" class="status-button status-button--primary" data-book-item="${escapeHtml(item.id)}" data-book-type="prebook">Prebook</button>
                </div>
            </article>
        `;
    }).join("");
}

function renderUpcomingOrders(state, orders) {
    const container = document.getElementById("upcomingOrders");
    const upcoming = [...orders]
        .filter((order) => order.bookingType === "prebook")
        .sort((left, right) => new Date(left.scheduledAt).getTime() - new Date(right.scheduledAt).getTime());

    if (!upcoming.length) {
        container.innerHTML = `
            <div class="empty-state">
                <strong>No upcoming prebookings.</strong>
                <p>Use the prebook button on any dish card to reserve a later slot.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = upcoming.map((order) => {
        const item = getMenuItemById(state, order.menuItemId);
        const restaurant = getRestaurantById(state, order.restaurantId);
        return `
            <article class="stack-card">
                <div class="stack-card__top">
                    <div>
                        <strong>${escapeHtml(item ? item.name : "Dish")}</strong>
                        <p>${escapeHtml(restaurant ? restaurant.name : "Restaurant")}</p>
                    </div>
                    <span class="status-pill status-pill--booking">Prebooked</span>
                </div>
                <div class="stack-card__meta">
                    <span class="tag">Qty ${order.quantity}</span>
                    <span class="tag">${formatDateTime(order.scheduledAt)}</span>
                </div>
                ${order.note ? `<p>${escapeHtml(order.note)}</p>` : ""}
            </article>
        `;
    }).join("");
}

function renderRecentOrders(state, orders) {
    const container = document.getElementById("recentOrders");
    const recent = [...orders].sort((left, right) => new Date(right.createdAt).getTime() - new Date(left.createdAt).getTime());

    if (!recent.length) {
        container.innerHTML = `
            <div class="empty-state">
                <strong>No bookings yet.</strong>
                <p>Book or prebook a dish to start building your history.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = recent.map((order) => {
        const item = getMenuItemById(state, order.menuItemId);
        const restaurant = getRestaurantById(state, order.restaurantId);
        return `
            <article class="stack-card">
                <div class="stack-card__top">
                    <div>
                        <strong>${escapeHtml(item ? item.name : "Dish")}</strong>
                        <p>${escapeHtml(restaurant ? restaurant.name : "Restaurant")} | ${escapeHtml(order.status)}</p>
                    </div>
                    <span class="status-pill ${order.bookingType === "prebook" ? "status-pill--booking" : ""}">
                        ${order.bookingType === "prebook" ? "Prebook" : "Instant"}
                    </span>
                </div>
                <div class="stack-card__meta">
                    <span class="tag">Qty ${order.quantity}</span>
                    <span class="tag">${formatDateTime(order.bookingType === "prebook" ? order.scheduledAt : order.createdAt)}</span>
                </div>
                ${order.note ? `<p>${escapeHtml(order.note)}</p>` : ""}
            </article>
        `;
    }).join("");
}

function bindBookingFlow(user) {
    const results = document.getElementById("foodResults");
    const modal = document.getElementById("bookingModal");
    const bookingForm = document.getElementById("bookingForm");
    const scheduleField = document.getElementById("scheduleField");
    const bookingSchedule = document.getElementById("bookingSchedule");

    results.addEventListener("click", (event) => {
        const button = event.target.closest("[data-book-item]");
        if (!button) {
            return;
        }

        openBookingModal(button.dataset.bookItem, button.dataset.bookType);
    });

    modal.querySelectorAll("[data-close-modal]").forEach((target) => {
        target.addEventListener("click", closeBookingModal);
    });

    bookingForm.addEventListener("submit", (event) => {
        event.preventDefault();

        const state = readState();
        const itemId = document.getElementById("bookingItemId").value;
        const bookingType = document.getElementById("bookingType").value;
        const quantity = Number(document.getElementById("bookingQuantity").value);
        const scheduledAt = bookingType === "prebook" ? bookingSchedule.value : "";
        const note = document.getElementById("bookingNotes").value.trim();
        const item = getMenuItemById(state, itemId);

        if (!item || !item.available) {
            showToast("This dish is no longer available.");
            closeBookingModal();
            renderUser(user);
            return;
        }

        if (!Number.isFinite(quantity) || quantity <= 0) {
            showToast("Choose a valid quantity.");
            return;
        }

        if (bookingType === "prebook") {
            if (!scheduledAt) {
                showToast("Choose a date and time for prebooking.");
                return;
            }

            const date = new Date(scheduledAt);
            if (Number.isNaN(date.getTime()) || date.getTime() <= Date.now()) {
                showToast("Prebooking time must be in the future.");
                return;
            }
        }

        state.orders.push({
            id: generateId("order"),
            userId: user.id,
            restaurantId: item.restaurantId,
            menuItemId: item.id,
            quantity,
            bookingType,
            scheduledAt,
            note,
            status: bookingType === "prebook" ? "Scheduled" : "Booked",
            createdAt: new Date().toISOString()
        });

        writeState(state);
        closeBookingModal();
        renderUser(user);
        showToast(bookingType === "prebook" ? "Prebooking confirmed." : "Booking confirmed.");
    });

    function openBookingModal(itemId, bookingType) {
        const state = readState();
        const item = getMenuItemById(state, itemId);
        const restaurant = item ? getRestaurantById(state, item.restaurantId) : null;

        if (!item || !restaurant) {
            showToast("Could not open booking form.");
            return;
        }

        document.getElementById("bookingItemId").value = item.id;
        document.getElementById("bookingType").value = bookingType;
        document.getElementById("bookingQuantity").value = "1";
        document.getElementById("bookingNotes").value = "";
        document.getElementById("bookingDishName").textContent = item.name;
        document.getElementById("bookingDishMeta").textContent = `${restaurant.name} | ${formatCurrency(item.price)} | ${item.prepMinutes} mins prep`;
        document.getElementById("bookingModeLabel").textContent = bookingType === "prebook" ? "Prebook pickup" : "Book instantly";
        document.getElementById("bookingSubmitButton").textContent = bookingType === "prebook" ? "Confirm Prebooking" : "Confirm Booking";

        if (bookingType === "prebook") {
            const minimum = new Date(Date.now() + 30 * 60 * 1000);
            scheduleField.classList.remove("hidden");
            bookingSchedule.required = true;
            bookingSchedule.min = toDateTimeLocal(minimum);
            bookingSchedule.value = "";
        } else {
            scheduleField.classList.add("hidden");
            bookingSchedule.required = false;
            bookingSchedule.value = "";
        }

        modal.classList.add("is-open");
        modal.setAttribute("aria-hidden", "false");
    }

    function closeBookingModal() {
        modal.classList.remove("is-open");
        modal.setAttribute("aria-hidden", "true");
    }
}

function toDateTimeLocal(date) {
    const year = date.getFullYear();
    const month = String(date.getMonth() + 1).padStart(2, "0");
    const day = String(date.getDate()).padStart(2, "0");
    const hours = String(date.getHours()).padStart(2, "0");
    const minutes = String(date.getMinutes()).padStart(2, "0");
    return `${year}-${month}-${day}T${hours}:${minutes}`;
}
