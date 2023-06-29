#include "http.h"

#include "../keyreader/keyhook.h"

#include "../data.h"
#include "../tagmap.h"

#include "../utils/array.h"
#include "../utils/str.h"

// ---------------------------------------------------------

#define __ERR_QUERRY_NOT_GIVEN(param, type)                                    \
    response_new(400, "The query '" param ": " type " is required")

#define __ERR_QUERRY_BOOL_INVALID(param)                                       \
    response_new(400, "The query '" param                                      \
                      " Should only contain 'true' or 'false' values ")

#define __ERR_GENERAL(msg)                                                     \
    response_new(400, "Server sent a error "                                   \
                      "\"" msg "\"")

#define __RESPONSE_SUCESS response_new(200, "Sucess")

// ---------------------------------------------------------

#define __QUERY_REQUIRED_GET(request, var, query, type)                        \
    do {                                                                       \
        var = request_get_query(request, query);                               \
        if (var == NULL) {                                                     \
            return __ERR_QUERRY_NOT_GIVEN(query, type);                        \
        }                                                                      \
    } while (0)

#define __QUERY_BOOL_GET(request, var, query, default)                         \
    do {                                                                       \
        const char *__qbool = request_get_query(request, query);               \
        if (__qbool == NULL) {                                                 \
            var = default;                                                     \
            break;                                                             \
        }                                                                      \
        if (str_eq(__qbool, "true")) {                                         \
            var = true;                                                        \
            break;                                                             \
        } else if (str_eq(__qbool, "false")) {                                 \
            var = false;                                                       \
            break;                                                             \
        }                                                                      \
        return __ERR_QUERRY_BOOL_INVALID(query);                               \
    } while (0)

// ---------------------------------------------------------

const char *__get_rw_id(Request *request, RWIdentifier *id, char *error_msg) {
    const char *__ids[] = {
        [REPLWORD_RW_SOURCE] = "word",
        [REPLWORD_RW_ID] = "id",
    };

    const char *identifier = NULL;
    error_msg = NULL;

    for (size_t i = 0; i < sizeof(__ids) / sizeof(__ids[0]); i++) {
        const char *rid = request_get_query(request, __ids[i]);
        if (rid != NULL) {
            if (identifier != NULL) {
                str_format(error_msg,
                           "Multiple identifiers \"%s\"\"%s\" is not allowed",
                           identifier, rid);
                return NULL;
            }

            identifier = rid;
            *id = i;
        }
    }
    return identifier;
}

// ---------------------------------------------------------

Response *__handle_add(Request *request) {

    const char *word;
    __QUERY_REQUIRED_GET(request, word, "word", "string");

    const char *replace;
    __QUERY_REQUIRED_GET(request, replace, "replace", "string");

    bool temporary;
    __QUERY_BOOL_GET(request, temporary, "temp", false);

    const char *id = request_get_query(request, "id");
    if (id != NULL) {
        if (atoi(id) < 0) {
            return __ERR_GENERAL("The word id cannot be below 0");
        }

        if (replword_find(id, REPLWORD_RW_ID) != -1)
            return __ERR_GENERAL("The word with the given id already exists in "
                                 "the replace list");
    }

    if (replword_find(word, REPLWORD_RW_SOURCE) != -1) {
        return __ERR_GENERAL(
            "The given word already exists in the replace list");
    }

    char *error_msg = replword_add_by_request(request);

    if (error_msg != NULL) {
        Response *error = response_new(400, error_msg);
        free(error_msg);
        return error;
    }

    data_save();

    return __RESPONSE_SUCESS;
}

Response *__handle_remove(Request *request) {
    RWIdentifier rwi;
    char *error_msg = NULL;
    const char *identifier = __get_rw_id(request, &rwi, error_msg);

    if (error_msg != NULL) {
        Response *error = response_new(400, error_msg);
        free(error_msg);
        return error;
    }

    if (!replword_remove(identifier, rwi)) {
        return __ERR_GENERAL("The given identifier does not exists");
    }

    data_save();

    return __RESPONSE_SUCESS;
}

Response *__handle_close(Request *request) {
    keyhook_raw_quit(); // Signal the keyhook to exit
    return __RESPONSE_SUCESS;
}

Response *__handle_list(Request *request) {
    mutex_lock(app_data.mutex);
    if (app_data.replwords_len == 0) {
        mutex_unlock(app_data.mutex);
        return response_new(200, " ");
    }

    Tag *root = replword_unpack_all(false);
    char *body = tag_format(root, false);

    Response *response = response_new(200, body);

    free(body);
    tag_free(root);

    mutex_unlock(app_data.mutex);

    return response;
}

Response *__handle_config(Request *request) {
    RWIdentifier rwi;
    char *error_msg = NULL;
    const char *identifier = __get_rw_id(request, &rwi, error_msg);

    if (error_msg != NULL) {
        Response *error = response_new(400, error_msg);
        free(error_msg);
        return error;
    }

    int index;
    if ((index = replword_find(identifier, rwi)) == -1) {
        return __ERR_GENERAL("The given identifier does not exists");
    }

    mutex_lock(app_data.mutex);

    ReplaceWord *replword = app_data.replwords[index];
    __QUERY_BOOL_GET(request, replword->enabled, "enable", replword->enabled);

    mutex_unlock(app_data.mutex);

    data_save();

    return __RESPONSE_SUCESS;
}

Response *_http_api_handle(Request *request) {

    if (str_eq(request->path, "/add")) {
        return __handle_add(request);
    } else if (str_eq(request->path, "/remove")) {
        return __handle_remove(request);
    } else if (str_eq(request->path, "/close")) {
        return __handle_close(request);
    } else if (str_eq(request->path, "/list")) {
        return __handle_list(request);
    } else if (str_eq(request->path, "/config")) {
        return __handle_config(request);
    }

    return __RESPONSE_SUCESS;
}
