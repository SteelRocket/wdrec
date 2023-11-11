#include "tagmap.h"

#include "keyreader/keytyper.h"

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

const TagMap __tagmap_replace[] = {
    __STD_KEYMAP("enter", __ENTER_CHAR, KEYTYPER_RETURN, TOPT_STANDALONE),
    __STD_KEYMAP("tab", '\t', KEYTYPER_TAB, TOPT_STANDALONE),
    __STD_KEYMAP("backspace", '\b', KEYTYPER_BACKSPACE, TOPT_STANDALONE),

    __STD_KEYMAP("shift", '\0', KEYTYPER_SHIFT, TOPT_NONE),
    __STD_KEYMAP("ctrl", '\0', KEYTYPER_CONTROL, TOPT_NONE),
    __STD_KEYMAP("alt", '\0', KEYTYPER_ALT, TOPT_NONE),

    __STD_KEYMAP("win", '\0', KEYTYPER_WIN, TOPT_NONE),

    {"uni", '\0', -1, TOPT_NSTANDALONE_TEXT, .press = __tg_unicode_type,
     .validator = __tv_unicode},
};

const size_t __tagmap_word_len = __ARRAY_LEN(__tagmap_word);
const size_t __tagmap_replace_len = __ARRAY_LEN(__tagmap_replace);