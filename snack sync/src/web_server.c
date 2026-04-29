#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SERVER_PORT 8080
#define REQUEST_BUFFER_SIZE 65536
#define OTP_STORE_SIZE 128
#define OTP_TTL_SECONDS 300

typedef struct {
    char email[256];
    char otp[8];
    time_t expires_at;
    int active;
} OtpRecord;

static OtpRecord g_otp_store[OTP_STORE_SIZE];
static char g_web_root[MAX_PATH];

static void copy_string(char *destination, size_t destination_size, const char *source) {
    if (destination_size == 0) {
        return;
    }

    snprintf(destination, destination_size, "%s", source == NULL ? "" : source);
}

static int compare_ignore_case(const char *left, const char *right) {
    unsigned char left_char;
    unsigned char right_char;

    while (*left != '\0' && *right != '\0') {
        left_char = (unsigned char) tolower((unsigned char) *left);
        right_char = (unsigned char) tolower((unsigned char) *right);
        if (left_char != right_char) {
            return (int) left_char - (int) right_char;
        }
        ++left;
        ++right;
    }

    return (int) tolower((unsigned char) *left) - (int) tolower((unsigned char) *right);
}

static void trim_spaces(char *text) {
    char *start;
    char *end;

    if (text == NULL || *text == '\0') {
        return;
    }

    start = text;
    while (*start != '\0' && isspace((unsigned char) *start)) {
        ++start;
    }

    if (start != text) {
        memmove(text, start, strlen(start) + 1);
    }

    end = text + strlen(text);
    while (end > text && isspace((unsigned char) end[-1])) {
        --end;
    }
    *end = '\0';
}

static const char *status_text(int status_code) {
    switch (status_code) {
        case 200:
            return "OK";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 500:
            return "Internal Server Error";
        default:
            return "OK";
    }
}

static void send_response(SOCKET client, int status_code, const char *content_type, const char *body, size_t body_length) {
    char header[1024];
    int header_length;

    header_length = snprintf(
            header,
            sizeof(header),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %lu\r\n"
            "Connection: close\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "\r\n",
            status_code,
            status_text(status_code),
            content_type,
            (unsigned long) body_length
    );

    send(client, header, header_length, 0);
    if (body != NULL && body_length > 0) {
        send(client, body, (int) body_length, 0);
    }
}

static void send_json(SOCKET client, int status_code, const char *json) {
    send_response(client, status_code, "application/json; charset=utf-8", json, strlen(json));
}

static void send_text(SOCKET client, int status_code, const char *text) {
    send_response(client, status_code, "text/plain; charset=utf-8", text, strlen(text));
}

static const char *content_type_for_path(const char *path) {
    const char *extension;

    extension = strrchr(path, '.');
    if (extension == NULL) {
        return "text/plain; charset=utf-8";
    }

    if (compare_ignore_case(extension, ".html") == 0) {
        return "text/html; charset=utf-8";
    }
    if (compare_ignore_case(extension, ".css") == 0) {
        return "text/css; charset=utf-8";
    }
    if (compare_ignore_case(extension, ".js") == 0) {
        return "application/javascript; charset=utf-8";
    }
    if (compare_ignore_case(extension, ".json") == 0) {
        return "application/json; charset=utf-8";
    }

    return "application/octet-stream";
}

static int read_file_bytes(const char *path, char **out_buffer, size_t *out_length) {
    FILE *file;
    long length;
    size_t bytes_read;
    char *buffer;

    file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }

    length = ftell(file);
    if (length < 0) {
        fclose(file);
        return 0;
    }

    rewind(file);

    buffer = (char *) malloc((size_t) length + 1);
    if (buffer == NULL) {
        fclose(file);
        return 0;
    }

    bytes_read = fread(buffer, 1, (size_t) length, file);
    fclose(file);
    buffer[bytes_read] = '\0';

    *out_buffer = buffer;
    *out_length = bytes_read;
    return 1;
}

static int content_length_from_headers(const char *request) {
    const char *header;
    int content_length;

    header = strstr(request, "Content-Length:");
    if (header == NULL) {
        return 0;
    }

    content_length = 0;
    sscanf(header, "Content-Length: %d", &content_length);
    return content_length;
}

static int extract_json_string(const char *body, const char *key, char *output, size_t output_size) {
    char needle[64];
    const char *position;
    const char *value_start;
    const char *value_end;
    size_t value_length;

    snprintf(needle, sizeof(needle), "\"%s\"", key);
    position = strstr(body, needle);
    if (position == NULL) {
        return 0;
    }

    position = strchr(position + strlen(needle), ':');
    if (position == NULL) {
        return 0;
    }

    ++position;
    while (*position != '\0' && isspace((unsigned char) *position)) {
        ++position;
    }

    if (*position != '"') {
        return 0;
    }

    value_start = position + 1;
    value_end = strchr(value_start, '"');
    if (value_end == NULL) {
        return 0;
    }

    value_length = (size_t) (value_end - value_start);
    if (value_length >= output_size) {
        value_length = output_size - 1;
    }

    memcpy(output, value_start, value_length);
    output[value_length] = '\0';
    return 1;
}

static int email_looks_valid(const char *email) {
    return email != NULL && strchr(email, '@') != NULL && strchr(email, '.') != NULL;
}

static void cleanup_otps(void) {
    time_t now;
    int index;

    now = time(NULL);
    for (index = 0; index < OTP_STORE_SIZE; ++index) {
        if (g_otp_store[index].active && g_otp_store[index].expires_at < now) {
            g_otp_store[index].active = 0;
        }
    }
}

static OtpRecord *find_otp_record(const char *email) {
    int index;

    for (index = 0; index < OTP_STORE_SIZE; ++index) {
        if (g_otp_store[index].active && compare_ignore_case(g_otp_store[index].email, email) == 0) {
            return &g_otp_store[index];
        }
    }

    return NULL;
}

static OtpRecord *reserve_otp_record(const char *email) {
    OtpRecord *record;
    int index;

    record = find_otp_record(email);
    if (record != NULL) {
        return record;
    }

    for (index = 0; index < OTP_STORE_SIZE; ++index) {
        if (!g_otp_store[index].active) {
            return &g_otp_store[index];
        }
    }

    return NULL;
}

static void generate_otp(char *buffer, size_t buffer_size) {
    int value;

    value = rand() % 900000 + 100000;
    snprintf(buffer, buffer_size, "%06d", value);
}

static int create_mail_file(const char *sender_email, const char *recipient_email, const char *otp, char *path_buffer, size_t path_buffer_size) {
    char temp_dir[MAX_PATH];
    char temp_name[MAX_PATH];
    FILE *file;

    if (GetTempPathA((DWORD) sizeof(temp_dir), temp_dir) == 0) {
        return 0;
    }

    if (GetTempFileNameA(temp_dir, "otp", 0, temp_name) == 0) {
        return 0;
    }

    file = fopen(temp_name, "wb");
    if (file == NULL) {
        return 0;
    }

    fprintf(file, "To: <%s>\r\n", recipient_email);
    fprintf(file, "From: Snack Sync <%s>\r\n", sender_email);
    fprintf(file, "Subject: Snack Sync OTP Verification\r\n");
    fprintf(file, "\r\n");
    fprintf(file, "Your Snack Sync OTP is: %s\r\n", otp);
    fprintf(file, "This OTP is valid for 5 minutes.\r\n");
    fprintf(file, "If you did not request this code, you can ignore this email.\r\n");
    fclose(file);

    copy_string(path_buffer, path_buffer_size, temp_name);
    return 1;
}

static int send_otp_email(const char *recipient_email, const char *otp, char *error_message, size_t error_message_size) {
    char sender_email[256];
    char app_password[256];
    char smtp_url[256];
    char mail_file[MAX_PATH];
    char command[2048];
    int exit_code;

    copy_string(sender_email, sizeof(sender_email), getenv("SNACK_SYNC_OTP_EMAIL"));
    copy_string(app_password, sizeof(app_password), getenv("SNACK_SYNC_OTP_APP_PASSWORD"));
    copy_string(smtp_url, sizeof(smtp_url), getenv("SNACK_SYNC_SMTP_URL"));

    trim_spaces(sender_email);
    trim_spaces(app_password);
    trim_spaces(smtp_url);

    if (sender_email[0] == '\0' || app_password[0] == '\0') {
        copy_string(
                error_message,
                error_message_size,
                "OTP email is not configured. Set SNACK_SYNC_OTP_EMAIL and SNACK_SYNC_OTP_APP_PASSWORD."
        );
        return 0;
    }

    if (smtp_url[0] == '\0') {
        copy_string(smtp_url, sizeof(smtp_url), "smtp://smtp.gmail.com:587");
    }

    if (!create_mail_file(sender_email, recipient_email, otp, mail_file, sizeof(mail_file))) {
        copy_string(error_message, error_message_size, "Could not prepare the OTP email file.");
        return 0;
    }

    snprintf(
            command,
            sizeof(command),
            "\"C:\\Windows\\System32\\curl.exe\" --url \"%s\" --ssl-reqd --mail-from \"%s\" --mail-rcpt \"%s\" --user \"%s:%s\" -T \"%s\" --silent --show-error --connect-timeout 10",
            smtp_url,
            sender_email,
            recipient_email,
            sender_email,
            app_password,
            mail_file
    );

    exit_code = system(command);
    DeleteFileA(mail_file);

    if (exit_code != 0) {
        copy_string(error_message, error_message_size, "OTP email sending failed. Check your email, app password, and SMTP settings.");
        return 0;
    }

    copy_string(error_message, error_message_size, "");
    return 1;
}

static void handle_send_otp(SOCKET client, const char *body) {
    char email[256];
    char otp[8];
    char error_message[512];
    OtpRecord *record;

    cleanup_otps();

    if (!extract_json_string(body, "email", email, sizeof(email))) {
        send_json(client, 400, "{\"ok\":false,\"message\":\"Email is required.\"}");
        return;
    }

    trim_spaces(email);
    if (!email_looks_valid(email)) {
        send_json(client, 400, "{\"ok\":false,\"message\":\"Please enter a valid email address.\"}");
        return;
    }

    generate_otp(otp, sizeof(otp));
    if (!send_otp_email(email, otp, error_message, sizeof(error_message))) {
        {
            char json[768];
            snprintf(json, sizeof(json), "{\"ok\":false,\"message\":\"%s\"}", error_message);
            send_json(client, 500, json);
        }
        return;
    }

    record = reserve_otp_record(email);
    if (record == NULL) {
        send_json(client, 500, "{\"ok\":false,\"message\":\"OTP storage is full. Try again.\"}");
        return;
    }

    copy_string(record->email, sizeof(record->email), email);
    copy_string(record->otp, sizeof(record->otp), otp);
    record->expires_at = time(NULL) + OTP_TTL_SECONDS;
    record->active = 1;

    send_json(client, 200, "{\"ok\":true,\"message\":\"OTP sent successfully.\"}");
}

static void handle_verify_otp(SOCKET client, const char *body) {
    char email[256];
    char otp[16];
    OtpRecord *record;

    cleanup_otps();

    if (!extract_json_string(body, "email", email, sizeof(email)) || !extract_json_string(body, "otp", otp, sizeof(otp))) {
        send_json(client, 400, "{\"ok\":false,\"message\":\"Email and OTP are required.\"}");
        return;
    }

    trim_spaces(email);
    trim_spaces(otp);

    record = find_otp_record(email);
    if (record == NULL) {
        send_json(client, 400, "{\"ok\":false,\"message\":\"No active OTP found for this email.\"}");
        return;
    }

    if (record->expires_at < time(NULL)) {
        record->active = 0;
        send_json(client, 400, "{\"ok\":false,\"message\":\"OTP has expired. Send a new one.\"}");
        return;
    }

    if (strcmp(record->otp, otp) != 0) {
        send_json(client, 400, "{\"ok\":false,\"message\":\"Incorrect OTP. Please try again.\"}");
        return;
    }

    record->active = 0;
    send_json(client, 200, "{\"ok\":true,\"message\":\"OTP verified successfully.\"}");
}

static void serve_static_file(SOCKET client, const char *request_path) {
    char relative_path[512];
    char full_path[MAX_PATH];
    char *file_bytes;
    size_t file_length;
    const char *content_type;

    if (strstr(request_path, "..") != NULL) {
        send_text(client, 404, "Not Found");
        return;
    }

    if (strcmp(request_path, "/") == 0) {
        copy_string(relative_path, sizeof(relative_path), "index.html");
    } else {
        copy_string(relative_path, sizeof(relative_path), request_path + 1);
    }

    snprintf(full_path, sizeof(full_path), "%s\\%s", g_web_root, relative_path);

    if (!read_file_bytes(full_path, &file_bytes, &file_length)) {
        send_text(client, 404, "File not found.");
        return;
    }

    content_type = content_type_for_path(full_path);
    send_response(client, 200, content_type, file_bytes, file_length);
    free(file_bytes);
}

static void set_web_root(void) {
    char executable_path[MAX_PATH];
    char *last_backslash;

    GetModuleFileNameA(NULL, executable_path, (DWORD) sizeof(executable_path));
    last_backslash = strrchr(executable_path, '\\');
    if (last_backslash != NULL) {
        *last_backslash = '\0';
    }

    snprintf(g_web_root, sizeof(g_web_root), "%s\\web", executable_path);
}

static void handle_request(SOCKET client, const char *request) {
    char method[16];
    char path[1024];
    const char *body;

    method[0] = '\0';
    path[0] = '\0';
    sscanf(request, "%15s %1023s", method, path);

    body = strstr(request, "\r\n\r\n");
    if (body != NULL) {
        body += 4;
    } else {
        body = "";
    }

    if (compare_ignore_case(method, "OPTIONS") == 0) {
        send_response(client, 204, "text/plain; charset=utf-8", "", 0);
        return;
    }

    if (compare_ignore_case(method, "GET") == 0 && strcmp(path, "/api/health") == 0) {
        send_json(client, 200, "{\"ok\":true,\"message\":\"Snack Sync OTP service is running.\"}");
        return;
    }

    if (compare_ignore_case(method, "POST") == 0 && strcmp(path, "/api/send-otp") == 0) {
        handle_send_otp(client, body);
        return;
    }

    if (compare_ignore_case(method, "POST") == 0 && strcmp(path, "/api/verify-otp") == 0) {
        handle_verify_otp(client, body);
        return;
    }

    if (compare_ignore_case(method, "GET") == 0) {
        serve_static_file(client, path);
        return;
    }

    send_json(client, 405, "{\"ok\":false,\"message\":\"Method not allowed.\"}");
}

static int receive_request(SOCKET client, char *buffer, size_t buffer_size) {
    int total_received;
    int received;
    char *headers_end;
    int expected_length;
    int current_body_length;

    total_received = 0;
    expected_length = 0;

    while (total_received < (int) buffer_size - 1) {
        received = recv(client, buffer + total_received, (int) buffer_size - total_received - 1, 0);
        if (received <= 0) {
            break;
        }

        total_received += received;
        buffer[total_received] = '\0';

        headers_end = strstr(buffer, "\r\n\r\n");
        if (headers_end != NULL) {
            expected_length = content_length_from_headers(buffer);
            current_body_length = total_received - (int) ((headers_end + 4) - buffer);
            if (current_body_length >= expected_length) {
                break;
            }
        }
    }

    buffer[total_received] = '\0';
    return total_received;
}

int main(void) {
    WSADATA wsa_data;
    SOCKET server_socket;
    struct sockaddr_in address;
    char request_buffer[REQUEST_BUFFER_SIZE];

    srand((unsigned int) time(NULL));
    set_web_root();

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Could not create socket.\n");
        WSACleanup();
        return 1;
    }

    {
        int reuse = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse));
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) == SOCKET_ERROR) {
        fprintf(stderr, "Could not bind to port %d.\n", SERVER_PORT);
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 10) == SOCKET_ERROR) {
        fprintf(stderr, "Could not listen on port %d.\n", SERVER_PORT);
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Snack Sync web server running on http://127.0.0.1:%d/\n", SERVER_PORT);
    printf("Static frontend root: %s\n", g_web_root);

    while (1) {
        SOCKET client_socket;

        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            continue;
        }

        if (receive_request(client_socket, request_buffer, sizeof(request_buffer)) > 0) {
            handle_request(client_socket, request_buffer);
        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
