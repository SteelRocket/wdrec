#pragma once

#define HTTP_MAX_THREADS 10
#define HTTP_BUFFER_SIZE 1024

#include <stdbool.h>

#include "../utils/http_request.h"
#include "../utils/http_response.h"

/**
 * @brief Gets the active port from ${data_folder}/port.
 *
 * Only works if a wdrec instance is already running
 *
 * @return port > 0 if sucess, port = 0 if port <= 0, port = -1 if wdrec
 * instance is not started
 */
int http_get_active_port();

/**
 * @brief Initalizes the http server (Does not start listening, but only init).
 *
 * @param port the server port
 * @param host the server host
 * @return true if sucess else false if there exists a socket listening on given
 * port and host
 */
bool http_init(int port, const char *host);

/**
 * @brief Actually starts the http server after http_init()
 */
void http_start();

/**
 * @brief Closes the http server and all running client threads after execution
 * or timeout
 */
void http_stop();

/**
 * @brief Handles the given request from http server
 *
 * Implemented in http_api.c
 *
 * @param request given request
 * @return sucess response or error response based on request
 */
Response *_http_api_handle(Request *request);