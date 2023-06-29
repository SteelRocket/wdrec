#include "http_client.h"

#include <stdio.h>
#include <stdlib.h>

#include "../utils/socket.h"

const char *DEFAULT_CREQUEST =
    "GET %s HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";

Response *chttp_request(int port, const char *url, int *error) {
    SOCKET socket = socket_create();
    *error = 0;

    if (socket == INVALID_SOCKET ||
        socket_connect(socket, "127.0.0.1", port) == SOCKET_ERROR) {
        *error = 1;
        return NULL;
    }

    size_t buffer_len = strlen(DEFAULT_CREQUEST) + strlen(url) + 1;
    char *buffer = malloc(buffer_len * sizeof(char));

    sprintf(buffer, DEFAULT_CREQUEST, url);
    socket_send(socket, buffer, buffer_len);
    free(buffer);

    char response_buffer[1024];
    int bytes_read;

    bytes_read = socket_recv(socket, response_buffer, 1024, 5);

    // Timeout
    if (bytes_read == -2 || bytes_read == -1) {
        *error = 2;
        return NULL;
    }

    response_buffer[1023] = '\0';

    return response_from(response_buffer);
}