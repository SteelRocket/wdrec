#include "tagmap.h"

#include "utils/array.h"
#include "utils/str.h"

#include "keyreader/keytyper.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define __ENTER_CHAR '\r'
#else
#define __ENTER_CHAR '\n'
#endif

// ---------------------------------------------------------
//                        Utils
// ---------------------------------------------------------

// Tries to convert string to int by hex or dec
unsigned long __try_hex_dec(const char *str, bool *error) {
    *error = false;

    if (strlen(str) < 3 || (str[0] != '0' && str[1] != 'x')) {
        char *end;
        unsigned long val = strtoul(str, &end, 10);
        if (*end != '\0') {
            *error = true;
        }
        return val;
    }

    char *end;
    unsigned long val = strtoul(str, &end, 16);
    if (*end != '\0') {
        *error = true;
    }
    return val;
}

// ---------------------------------------------------------
//                        Typers
// ---------------------------------------------------------

int __tg_press(const struct TagMap *map, Tag *tag, int keyhook_info) {

    if (map == NULL) {
        if (strlen(tag->name) == 1) {
            keytype_press_release_char(*(tag->name), keyhook_info);
            return 1;
        }
        return 0;
    }

    if (tag->text != NULL || tag->tags != NULL) {
        keytyper_key_press(map->key_code, keyhook_info);
        return 0;
    }

    keytyper_key(map->key_code, keyhook_info);

    if (map->char_value == '\b')
        return -1;
    if (map->char_value)
        return 1;
    return 0;
}

void __tg_release(const struct TagMap *map, int keyhook_info) {
    keytyper_key_release(map->key_code, keyhook_info);
}

int __tg_unicode_type(const struct TagMap *map, Tag *tag, int keyhook_info) {
    bool _;
    keytyper_type(__try_hex_dec(tag->text, &_), keyhook_info);
    return 1;
}

// ---------------------------------------------------------
//                       Validators
// ---------------------------------------------------------

const char *__tv_unicode(Tag *tag) {
    bool error;
    unsigned long codepoint = __try_hex_dec(tag->text, &error);
    if (error) {
        return "contains invalid hexadecimal or decimal number";
    }

    if (codepoint <= 0x10FFFF && (codepoint < 0xD800 || codepoint > 0xDFFF)) {
        return NULL;
    }
    return "contains unvalid unicode codepoint";
}

// ---------------------------------------------------------

#define __STD_KEYMAP(name, char, key, opt)                                     \
    { name, char, key, opt, __tg_press, __tg_release }

#define __ARRAY_LEN(x) sizeof(x) / sizeof(x[0])

const TagMap __tagmap_word[] = {
    __STD_KEYMAP("enter", __ENTER_CHAR, KEYTYPER_RETURN, TOPT_STANDALONE),
    __STD_KEYMAP("tab", '\t', KEYTYPER_TAB, TOPT_STANDALONE),

    __STD_KEYMAP("casein", '\0', -1, TOPT_NSTANDALONE),
};
const size_t __tagmap_word_len = __ARRAY_LEN(__tagmap_word);

const TagMap __tagmap_replace[] = {
    __STD_KEYMAP("enter", __ENTER_CHAR, KEYTYPER_RETURN, TOPT_STANDALONE),
    __STD_KEYMAP("tab", '\t', KEYTYPER_TAB, TOPT_STANDALONE),
    __STD_KEYMAP("backspace", '\b', KEYTYPER_BACKSPACE, TOPT_STANDALONE),

    __STD_KEYMAP("shift", '\0', KEYTYPER_SHIFT, TOPT_NONE),
    __STD_KEYMAP("ctrl", '\0', KEYTYPER_CONTROL, TOPT_NONE),

    {"uni", '\0', -1, TOPT_NSTANDALONE_TEXT, .press = __tg_unicode_type,
     .validator = __tv_unicode},
};
const size_t __tagmap_replace_len = __ARRAY_LEN(__tagmap_replace);

char tagmap_get_char(const char *tag_name, bool is_replace) {
    if (strlen(tag_name) == 1) {
        return *tag_name;
    }

    const TagMap *map;
    size_t len;

    if (is_replace) {
        map = __tagmap_replace;
        len = __tagmap_replace_len;
    } else {
        map = __tagmap_word;
        len = __tagmap_word_len;
    }

    for (size_t i = 0; i < len; i++) {
        if (str_eq(map[i].tag_name, tag_name)) {
            return map[i].char_value;
        }
    }
    return '\0';
}

int tagmap_get_key(const char *tag_name, bool is_replace) {
    const TagMap *map;
    size_t len;

    if (is_replace) {
        map = __tagmap_replace;
        len = __tagmap_replace_len;
    } else {
        map = __tagmap_word;
        len = __tagmap_word_len;
    }

    for (size_t i = 0; i < len; i++) {
        if (str_eq(map[i].tag_name, tag_name)) {
            return map[i].key_code;
        }
    }
    return -1;
}

const TagMap *tagmap_get(const char *tag_name, bool is_replace) {
    const TagMap *map;
    size_t len;

    if (is_replace) {
        map = __tagmap_replace;
        len = __tagmap_replace_len;
    } else {
        map = __tagmap_word;
        len = __tagmap_word_len;
    }

    for (size_t i = 0; i < len; i++) {
        if (str_eq(map[i].tag_name, tag_name)) {
            return &map[i];
        }
    }
    return NULL;
}

char *__handle_tagname(Tag *tag, const TagMap map[], size_t map_len,
                       bool replace) {
    const TagMap *found = NULL;
    const char *map_type = (replace) ? "replace" : "word";
    char *error_msg;

    if (strlen(tag->name) == 1) {
        if (tag->text != NULL || tag->tags != NULL) {
            str_format(error_msg,
                       "(%s) \"<%s>\" tag should not contain value like text "
                       "or sub tags",
                       map_type, tag->name);
            return error_msg;
        }
        return NULL;
    }

    // Check if tag name is a valid name
    for (size_t i = 0; i < map_len; i++) {
        if (str_eq(tag->name, map[i].tag_name)) {
            found = &map[i];
            break;
        }
    }

    if (found == NULL) {
        str_format(error_msg, "(%s) Invalid tag \"<%s>\"", map_type, tag->name);
        return error_msg;
    }

    if (found->opts & TOPT_NSTANDALONE_TEXT && tag->text == NULL) {

        if (tag->tags != NULL)
            str_format(error_msg, "(%s) \"%s\" tag should contain text only",
                       map_type, tag->name);
        else
            str_format(error_msg, "(%s) \"%s\" tag should contain text",
                       map_type, tag->name);

        return error_msg;
    }

    if (found->opts & TOPT_NSTANDALONE && tag->text == NULL &&
        tag->tags_len == 0) {
        str_format(
            error_msg,
            "(%s) \"<%s>\" tag should contain some value like text or sub tags",
            map_type, tag->name);

        return error_msg;
    }

    if (found->opts & TOPT_STANDALONE &&
        (tag->text != NULL || tag->tags_len != 0)) {
        str_format(
            error_msg,
            "(%s) \"<%s>\" tag should not contain value like text or sub tags",
            map_type, tag->name);
        return error_msg;
    }

    if (found->validator != NULL) {
        const char *verror = found->validator(tag);
        if (verror != NULL) {
            str_format(error_msg, "(%s) \"<%s>\" %s", map_type, tag->name,
                       verror);
            return error_msg;
        }
    }

    return NULL;
}

char *__tagmap_tag_valid(Tag *tag, bool is_replace) {
    char *error_msg;
    const TagMap *map;
    size_t len;

    if (is_replace) {
        map = __tagmap_replace;
        len = __tagmap_replace_len;
    } else {
        map = __tagmap_word;
        len = __tagmap_word_len;
    }

    // Check if tag name is a special tag name and not a container
    if (!str_eq(tag->name, "word") && !str_eq(tag->name, "replace") &&
        !str_eq(tag->name, "root") && !str_eq(tag->name, "text")) {

        error_msg = __handle_tagname(tag, map, len, is_replace);
        if (error_msg != NULL)
            return error_msg;

    } else if (str_eq(tag->name, "text") && tag->tags_len != 0) {
        str_format(error_msg,
                   "(%s) <text> tag should not contain any sub tags "
                   "in the given %s tag",
                   (is_replace) ? "replace" : "word");

        return error_msg;
    } else if (str_eq(tag->name, "root") && tag->tags_len == 0) {
        str_format(error_msg, "(%s) <root> tag should not be empty",
                   (is_replace) ? "replace" : "word");

        return error_msg;
    }

    // Validate for child tags also
    for (size_t i = 0; i < tag->tags_len; i++) {
        error_msg = __tagmap_tag_valid(tag->tags[i], is_replace);
        if (error_msg != NULL)
            return error_msg;
    }

    return NULL;
}

char *tagmap_word_valid(Tag *word) { return __tagmap_tag_valid(word, false); }

char *tagmap_replace_valid(Tag *replace) {
    return __tagmap_tag_valid(replace, true);
}