#include "http.h"

#include "../keyreader/keyhook.h"

#include "../data.h"

#include "../utils/array.h"
#include "../utils/http_request.h"
#include "../utils/path.h"
#include "../utils/socket.h"
#include "../utils/thread.h"

SOCKET __http_socket;
Thread *__http_thread;

Thread *__client_threads[HTTP_MAX_THREADS] = {NULL};
size_t __client_thread_len = 0;

// Port file for getting port of the current active
// server after the server is started
// It is opend at init and closed at cleanup
// to prevent file deletion
FILE *port_file;

char *__get_port_path() {
    char *data_dir = data_get_dir();
    if (!path_is_dir(data_dir)) {
        free(data_dir);
        return NULL;
    }

    char *port_path = path_join(data_dir, "port");

    free(data_dir);
    return port_path;
}

void __delete_port_file() {
    fclose(port_file);
    char *port_path = __get_port_path();
    remove(port_path);
    free(port_path);
}

void __handle_client(void *data) {
    SOCKET client = (SOCKET)data;
    char buffer[HTTP_BUFFER_SIZE];

    int size = socket_recv(client, buffer, HTTP_BUFFER_SIZE, 5);
    if (size == -1) {
        socket_close(client);
        return;
    }

    if (size == -2) {
        Response *response = response_new(504, "Timeout");
        socket_send(client, response->raw, response->raw_len);
        socket_close(client);
        response_free(response);
        return;
    }

    Request *request = request_parse(buffer);
    if (request == NULL) {
        Response *response =
            response_new(504, "The request format is not correct");
        socket_send(client, response->raw, response->raw_len);
        socket_close(client);
        response_free(response);
        return;
    }

    Response *response = _http_api_handle(request);

    request_free(request);

    socket_send(client, response->raw, response->raw_len);

    socket_close(client);
    response_free(response);
}

void __http_start(void *_) {
    while (1) {
        SOCKET client = socket_accept(__http_socket);
        if (client == INVALID_SOCKET) {
            continue;
        }

        while (__client_thread_len == HTTP_MAX_THREADS) {
            int nsize = 0;
            for (size_t i = 0; i < __client_thread_len; i++) {
                if (!thread_is_running(__client_threads[i])) {
                    thread_join(__client_threads[i]);
                    continue;
                }
                __client_threads[nsize++] = __client_threads[i];
            }
            __client_thread_len = nsize;
        }

        __client_threads[__client_thread_len++] =
            thread_create(__handle_client, (void *)client);
    }
}

int http_get_active_port() {
    char *port_path = __get_port_path();
    if (!path_is_file(port_path)) {
        free(port_path);
        return -1;
    }

    // wdrec is not running as port file is not locked
    if (remove(port_path) == 0) {
        free(port_path);
        return -1;
    }

    char *contents = path_read_all(port_path);
    int port = atoi(contents);

    free(contents);
    free(port_path);

    if (port < 0)
        return 0;
    return port;
}

bool http_init(int port, const char *host) {

    __http_socket = socket_create();
    if (__http_socket == INVALID_SOCKET) {
        socket_cleanup();
        return false;
    }

    if (socket_bind(__http_socket, host, port) == SOCKET_ERROR) {
        socket_close(__http_socket);
        socket_cleanup();
        return false;
    }

    if (socket_listen(__http_socket, 5) == SOCKET_ERROR) {
        socket_close(__http_socket);
        socket_cleanup();
        return false;
    }

    char *port_path = __get_port_path();

    // Lock the file and prevent deletion while wdrec is running
    port_file = fopen(port_path, "w");

    fprintf(port_file, "%d", port);
    fflush(port_file);
    free(port_path);

    atexit(__delete_port_file);

    return true;
}

void http_start() { __http_thread = thread_create(__http_start, NULL); }

void http_stop() {
    thread_terminate(__http_thread);
    socket_close(__http_socket);

    for (size_t i = 0; i < __client_thread_len; i++) {
        thread_join(__client_threads[i]);
    }
}
