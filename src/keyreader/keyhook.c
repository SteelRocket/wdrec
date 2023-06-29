#include "keyhook.h"
#include "keyhook_raw.h"

#include "keybuffer.h"
#include "keytyper.h"

#include "../data.h"

char *__last_source = NULL;
int __replace_len = 0;
bool __replacing = false;

void __reset_undo() {
    if (__last_source != NULL) {
        free(__last_source);
        __last_source = NULL;
    }
    __replace_len = 0;
}

bool __check_and_replace(KeyEvent event) {
    for (size_t i = 0; i < app_data.replwords_len; i++) {
        if (!app_data.replwords[i]->enabled)
            continue;

        Tag *source = app_data.replwords[i]->source;
        Tag *replace = app_data.replwords[i]->replace;

        int end_offset = keybuffer_endswith(source);

        if (!__replacing && end_offset != -1) {
            while (keybuffer_cursor_move(-1)) {
                keytyper_key(KEYTYPER_RIGHT_ARROW, KEYTYPER_KH_IGNORE);
            }

            __replacing = true;

            int lsource_len = keybuffer_size - end_offset;
            char *last_source = malloc((lsource_len + 1) * sizeof(char));
            memcpy(last_source, keybuffer + end_offset, lsource_len);
            last_source[lsource_len] = '\0';

            keybuffer_pop();
            __replace_len = keytyper_replace(keybuffer_size - end_offset,
                                             replace, KEYTYPER_KH_NONE);

            __replacing = false;

            if (__replace_len != -1)
                __last_source = last_source;
            else
                free(last_source);

            return true;
        }
    }
    return false;
}

bool __handle_keydown(KeyEvent event) {

    if (event.character == '\b') {
        if (__last_source != NULL) {
            keybuffer_replace(__replace_len, __last_source);
            for (size_t i = 0; i < __replace_len; i++) {
                keytyper_key(KEYTYPER_BACKSPACE, KEYTYPER_KH_IGNORE);
            }
            keytyper_string(__last_source, KEYTYPER_KH_IGNORE);

            free(__last_source);
            __last_source = NULL;

            return true;
        }
        keybuffer_pop();
    } else {
        keybuffer_push(event.character);

        mutex_lock(app_data.mutex);
        bool replaced = __check_and_replace(event);
        mutex_unlock(app_data.mutex);

        if (replaced) {
            return true;
        }
    }

    if (__last_source && !__replacing) {
        free(__last_source);
        __last_source = NULL;
    }

    return false;
}

bool keyhook_handle_event(KeyEvent event) {
    if (event.character && !event.is_ctrldown) {
        if (event.is_keydown) {
            return __handle_keydown(event);
        }
        return false;
    }

    // Ctrl + A, ...
    if (event.character && event.is_ctrldown) {
        keyhook_reset();
        return false;
    }

    // Left arrow
    if (keytyper_is_pressed(KEYTYPER_LEFT_ARROW)) {
        if (event.is_ctrldown) {
            keyhook_reset();
            return false;
        }
        keybuffer_cursor_move(1);
        __reset_undo();
        return false;
    }

    // Right arrow
    if (keytyper_is_pressed(KEYTYPER_RIGHT_ARROW)) {
        if (event.is_ctrldown) {
            keyhook_reset();
            return false;
        }
        keybuffer_cursor_move(-1);
        __reset_undo();
        return false;
    }

    // Up and Down arrow
    if (keytyper_is_pressed(KEYTYPER_DOWN_ARROW) ||
        keytyper_is_pressed(KEYTYPER_UP_ARROW)) {
        keyhook_reset();
        return false;
    }

    return false;
}

void keyhook_reset() {
    __reset_undo();
    keybuffer_empty();
}

void keyhook_run() {
    keyhook_reset();
    keyhook_raw_run();
}