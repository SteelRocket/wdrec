#pragma once

#include <stdbool.h>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int inet_pton(int af, const char *src, void *dst);

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif

int socket_init();

void socket_cleanup();

SOCKET socket_create();

void socket_close(SOCKET socket);

int socket_shutdown(SOCKET socket);

int socket_bind(SOCKET socket, const char *address, int port);

bool socket_is_bound(SOCKET socket);

int socket_listen(SOCKET socket, int backlog);

SOCKET socket_accept(SOCKET socket);

int socket_connect(SOCKET socket, const char *address, int port);

int socket_send(SOCKET socket, const void *data, int length);

int socket_recv(SOCKET socket, char *buffer, int buffer_length,
                int timeout_sec);

#ifdef UTILS_IMPLEMENTATION

int socket_init() {
#ifdef _WIN32
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data);
#else
    return 0; // No initialization needed on Unix-like systems
#endif
}

void socket_cleanup() {
#ifdef _WIN32
    WSACleanup();
#endif
}

SOCKET socket_create() { return socket(AF_INET, SOCK_STREAM, 0); }

void socket_close(SOCKET socket) {
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

int socket_shutdown(SOCKET socket) {
#ifdef _WIN32
    return shutdown(socket, SD_BOTH);
#else
    return shutdown(socket, SHUT_RDWR);
#endif
}

int socket_bind(SOCKET socket, const char *address, int port) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
#ifdef _WIN32
    if (inet_pton(AF_INET, address, &(server_address.sin_addr)) != 1) {
#else
    if (inet_pton(AF_INET, address, &(server_address.sin_addr.s_addr)) != 1) {
#endif
        return SOCKET_ERROR;
    }
    server_address.sin_port = htons(port);

    return bind(socket, (struct sockaddr *)&server_address,
                sizeof(server_address));
}

bool socket_is_bound(SOCKET socket) {
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
    return getsockname(socket, (struct sockaddr *)&addr, &addr_len) != -1;
}

int socket_listen(SOCKET socket, int backlog) {
    return listen(socket, backlog);
}

SOCKET socket_accept(SOCKET socket) {
    struct sockaddr_in client_address;
    int client_address_size = sizeof(client_address);

    return accept(socket, (struct sockaddr *)&client_address,
                  &client_address_size);
}

int socket_connect(SOCKET socket, const char *address, int port) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
#ifdef _WIN32
    if (inet_pton(AF_INET, address, &(server_address.sin_addr)) != 1) {
#else
    if (inet_pton(AF_INET, address, &(server_address.sin_addr.s_addr)) != 1) {
#endif
        return SOCKET_ERROR;
    }
    server_address.sin_port = htons(port);

    return connect(socket, (struct sockaddr *)&server_address,
                   sizeof(server_address));
}

int socket_send(SOCKET socket, const void *data, int length) {
#ifdef _WIN32
    return send(socket, data, length, 0);
#else
    return send(socket, data, (size_t)length, 0);
#endif
}

int socket_recv(SOCKET socket, char *buffer, int buffer_length,
                int timeout_sec) {
    if (timeout_sec > 0) {
        struct timeval timeout;
        timeout.tv_sec = timeout_sec;
        timeout.tv_usec = 0;

        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(socket, &read_set);

        int read_timeout = select(0, &read_set, NULL, NULL, &timeout);

        if (read_timeout == -1)
            return -1;

        if (read_timeout == 0)
            return -2;
    }

#ifdef _WIN32
    int recv_size = recv(socket, buffer, buffer_length - 1, 0);
#else
    int recv_size = recv(socket, buffer, (size_t)(buffer_length - 1), 0);
#endif
    buffer[recv_size] = '\0';
    return recv_size;
}

#endif