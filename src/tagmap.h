#pragma once

#include "utils/tags.h"

#include <stdbool.h>

typedef enum {
    // No options
    TOPT_NONE = 0,

    // Makes sure tag contains sub tags or text
    // Works with TOPT_TEXT_ONLY
    TOPT_NSTANDALONE = 1 << 0,

    // Makes sure tag has text
    TOPT_NSTANDALONE_TEXT = 1 << 1,

    // Makes sure tag does not contain sub tags
    TOPT_STANDALONE = 1 << 2,

} TagOptions;

typedef struct TagMap {
    const char *tag_name;

    char char_value;
    int key_code;
    TagOptions opts;

    int (*press)(const struct TagMap *, Tag *, int);
    void (*release)(const struct TagMap *, int);
    const char *(*validator)(Tag *);
} TagMap;

extern const TagMap __tagmap_word[];
extern const TagMap __tagmap_replace[];
extern const size_t __tagmap_word_len;
extern const size_t __tagmap_replace_len;

/**
 * @brief Returns the character for the respective tag name
 *
 * @param tag_name The tag name to be converted to character
 * @param is_replace whether the tag name from replace tag or word tag
 * @return character of tag name or if not found '\0'
 */
char tagmap_get_char(const char *tag_name, bool is_replace);

/**
 * @brief Returns the keycode for the respective tag name
 *
 * @param tag_name The tag name to be converted to keycode
 * @param is_replace whether the tag name from replace tag or word tag
 * @return keycode of tag name or if not found -1
 */
int tagmap_get_key(const char *tag_name, bool is_replace);

/**
 * @brief Returns the TagMap of the respective tag_name
 *
 * @param tag_name The tag name to be converted to tag map
 * @param is_replace whether the tag name from replace tagmap or word tagmap
 * @return TagMap reference if sucess else returns NULL
 */
const TagMap *tagmap_get(const char *tag_name, bool is_replace);

/**
 * @brief Checks if given word tag contains valid sub tags
 *
 * @param word the input word tag
 * @return error message on error else it returns NULL
 */
char *tagvalid_word(Tag *word);

/**
 * @brief Checks if given replace tag contains valid sub tags
 *
 * @param replace the input replace tag
 * @return error message on error else it returns NULL
 */
char *tagvalid_replace(Tag *replace);
