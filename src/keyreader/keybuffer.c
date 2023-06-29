#include "keybuffer.h"

#include "../tagmap.h"

#include "../utils/str.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *keybuffer;

size_t keybuffer_size;
size_t keybuffer_cursor;

void __print_char(char c, bool nline) {
    if (c == '\n') {
        printf("'\\n'");
    } else if (c == '\b') {
        printf("'\\b'");
    } else if (c == '\r') {
        printf("'\\r'");
    } else if (c == '\t') {
        printf("'\\t'");
    } else {
        printf("'%c'", c);
    }
    if (nline)
        printf("\n");
}

void keybuffer_print() {
    printf("[");
    for (size_t i = 0; i < keybuffer_size; i++) {
        __print_char(keybuffer[i], false);
        printf(", ");
    }
    printf("]\n");
}

bool keybuffer_cursor_move(int offset) {
    int new_cursor = keybuffer_cursor + offset;
    if (new_cursor >= 0 && new_cursor <= keybuffer_size) {
        keybuffer_cursor = (size_t)new_cursor;
        return true;
    }
    return false;
}

void keybuffer_empty() {
    if (keybuffer == NULL)
        keybuffer = malloc(sizeof(char));
    else
        keybuffer = realloc(keybuffer, sizeof(char));

    keybuffer_size = 0;
    keybuffer_cursor = 0;
}

void keybuffer_push(char c) {
    keybuffer_size++;
    keybuffer = realloc(keybuffer, keybuffer_size * sizeof(char));

    int cindex = keybuffer_size - keybuffer_cursor - 1;

    memmove(keybuffer + cindex + 1, keybuffer + cindex, keybuffer_cursor);
    keybuffer[cindex] = c;
}

void keybuffer_append(char *str) {
    while (*str != '\0') {
        keybuffer_push(*(str++));
    }
}

void keybuffer_pop() {
    if (keybuffer_size == 0)
        return;

    int rindex = keybuffer_size - keybuffer_cursor - 1;
    for (rindex; rindex < keybuffer_size - 1; rindex++) {
        keybuffer[rindex] = keybuffer[rindex + 1];
    }

    keybuffer_size--;
    keybuffer = realloc(keybuffer, keybuffer_size * sizeof(char));

    if (keybuffer_cursor >= keybuffer_size) {
        keybuffer_cursor = (keybuffer_size != 0) ? keybuffer_size - 1 : 0;
    }
}

void keybuffer_replace(size_t pop_len, char *rstr) {
    for (size_t i = 0; i < pop_len; i++) {
        keybuffer_pop();
    }
    keybuffer_append(rstr);
}

// ----------------------------
//      Endswith Checking
// ----------------------------

typedef struct {
    int *cursor;

    bool case_sensitive;
} __CheckSettings;

void __tagname_to_settings(const char *tag_name, __CheckSettings *settings) {
    if (str_eq(tag_name, "casein")) {
        settings->case_sensitive = false;
        return;
    }
}

bool __char_eq(char c1, char c2, __CheckSettings settings) {
    if (!settings.case_sensitive)
        return tolower(c1) == tolower(c2);

    return c1 == c2;
}

int __tagname_eq(const char *tag_name, __CheckSettings settings) {
    char check_char = tagmap_get_char(tag_name, false);

    if (check_char == '\0')
        return -1;

    (*settings.cursor)--;
    if (*settings.cursor < 0)
        return -1;

    if (!__char_eq(check_char, keybuffer[*settings.cursor], settings))
        return -1;

    return *settings.cursor;
}

int __keybuffer_endswith(Tag *tag, __CheckSettings settings) {
    if (tag->text != NULL) {
        __tagname_to_settings(tag->name, &settings);

        for (int i = strlen(tag->text) - 1; i >= 0; i--) {
            (*settings.cursor)--;

            if (*settings.cursor < 0)
                return -1;

            if (!__char_eq(tag->text[i], keybuffer[*settings.cursor],
                           settings)) {
                return -1;
            }
        }
    } else if (tag->name != NULL && tag->tags_len == 0) {
        return __tagname_eq(tag->name, settings);
    }

    for (int i = tag->tags_len - 1; i >= 0; i--) {
        if (__keybuffer_endswith(tag->tags[i], settings) == -1) {
            return -1;
        }
    }

    return *settings.cursor;
}

int keybuffer_endswith(Tag *tag) {
    int cursor = keybuffer_size;
    __CheckSettings settings = (__CheckSettings){
        .cursor = &cursor,
        .case_sensitive = true,
    };
    return __keybuffer_endswith(tag, settings);
}
