#pragma once

#include "utils/http_request.h"
#include "utils/http_response.h"
#include "utils/tags.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    Tag *source;  // The word to be replaced
    Tag *replace; // The replaced word
    size_t id;

    bool enabled;   // Indicates whether replword is active in keyhook
    bool temporary; // If true, will not save the ReplaceWord to save file
} ReplaceWord;

typedef enum {
    REPLWORD_RW_SOURCE,
    REPLWORD_RW_ID,
} RWIdentifier;

/**
 * @brief fress replword
 *
 * @param replword the given replword
 */
void replword_free(ReplaceWord *replword);

/**
 * @brief Finds and returns the index of replword by the given identifier
 *
 * @param identifier The tag identifier to find the replword with
 * @return index if sucess, returns -1 if identifier is not found in
 * data->replwords
 */
int replword_find(const char *identifier, RWIdentifier itype);

/**
 * @brief Adds ReplaceWord to app_data->replwords definied in data.c by Request
 *
 * @param request The http request from api server
 * @return error message on error else it returns NULL
 */
char *replword_add_by_request(Request *request);

/**
 * @brief Adds ReplaceWord to app_data->replwords definied in data.c by a Root
 * tag
 *
 * @param root The tag which contains <word> and <replace> subtags
 * @return error message on error else it returns NULL
 */
char *replword_add_by_root_tag(Tag *root);

/**
 * @brief Removes ReplaceWord from app_data->replwords by (*Tag)->tag_source
 *
 * @param identifier The tag identifier to remove ReplaceWord by
 * @return true if sucess, returns false if identifier is not found in
 * data->replwords
 */
bool replword_remove(const char *identifier, RWIdentifier itype);

/**
 * @brief Returns a tag containing the list of all replwords as tags
 *
 * @param ignore_temp whether to ignore the tags with temporary attribute
 * @return tag containing all replwords
 */
Tag *replword_unpack_all(bool ignore_temp);
