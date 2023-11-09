#include "keytyper.h"

#include "../tagmap.h"

#include "../utils/str.h"

// ---------------------------------------------------------
//                      Key Map
// ---------------------------------------------------------

bool __char_is_shifted(char c) {
#ifdef _WIN32
    return HIBYTE(VkKeyScan(c));
#else
#error "not implemented in NON WINDOWS PLATFORMS"
#endif
}

int __char_to_vk(char c) {
#ifdef _WIN32
    return LOBYTE(VkKeyScan(c));
#else
#error "not implemented in NON WINDOWS PLATFORMS"
#endif
}

#ifdef _WIN32
bool __vk_is_extended(int vk) {
    return (vk >= 0x21 && vk <= 0x28) || // Page, Home, End, Arrow keys
           (vk >= 0x2D && vk <= 0x2E) || // Insert, Delete
           (vk >= 0x5B && vk <= 0x5C) || // LWIN, RWIN
           (vk >= 0x70 && vk <= 0x87);   // F1 to F24
}
#endif

// ---------------------------------------------------------

bool keytyper_is_pressed(int keycode) {
#ifdef _WIN32
    return GetAsyncKeyState(keycode) < 0;
#else
#error "not implemented in NON WINDOWS PLATFORMS"
#endif
}

// ---------------------------------------------------------

void keytyper_key_press(int vk_code, KeyHookInfo keyhook_info) {
#ifdef _WIN32
    INPUT input;

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = MapVirtualKeyA(vk_code, 0);
    input.ki.dwFlags = KEYEVENTF_SCANCODE |
                       (__vk_is_extended(vk_code) ? KEYEVENTF_EXTENDEDKEY : 0);
    input.ki.time = 0;
    input.ki.dwExtraInfo = keyhook_info;

    SendInput(1, &input, sizeof(INPUT));
#else
#error "not implemented in NON WINDOWS PLATFORMS"
#endif
}

void keytyper_key_release(int vk_code, KeyHookInfo keyhook_info) {
#ifdef _WIN32
    INPUT input;

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = MapVirtualKeyA(vk_code, 0);
    input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP |
                       (__vk_is_extended(vk_code) ? KEYEVENTF_EXTENDEDKEY : 0);
    input.ki.time = 0;
    input.ki.dwExtraInfo = keyhook_info;

    SendInput(1, &input, sizeof(INPUT));
#else
#error "not implemented in NON WINDOWS PLATFORMS"
#endif
}

void keytyper_key(int keycode, KeyHookInfo keyhook_info) {
    keytyper_key_press(keycode, keyhook_info);
    keytyper_key_release(keycode, keyhook_info);
}

// ---------------------------------------------------------

void keytyper_type(unsigned int c, KeyHookInfo keyhook_info) {
#ifdef _WIN32
    INPUT input;

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = c;
    input.ki.dwFlags = KEYEVENTF_UNICODE;
    input.ki.time = 0;
    input.ki.dwExtraInfo = keyhook_info;

    SendInput(1, &input, sizeof(INPUT));
    input.ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
#else
#error "not implemented in NON WINDOWS PLATFORMS"
#endif
}

void keytype_press_release_char(char c, KeyHookInfo keyhook_info) {
    bool should_shift = __char_is_shifted(c);
    int keycode = __char_to_vk(c);

    if (should_shift)
        keytyper_key_press(keycode, keyhook_info);

    keytyper_key(keycode, keyhook_info);

    if (should_shift)
        keytyper_key_release(keycode, keyhook_info);
}

// ---------------------------------------------------------

void keytyper_string(char *str, KeyHookInfo keyhook_info) {
    while (*str != '\0') {
        keytyper_type(*(str++), keyhook_info);
    }
}

int __keytyper_replace(Tag *tag, KeyHookInfo keyhook_info) {
    bool is_text = str_eq(tag->name, "replace") || str_eq(tag->name, "text");
    const TagMap *map = tagmap_get(tag->name, true);

    if (is_text && tag->text != NULL) {
        keytyper_string(tag->text, keyhook_info);
        return strlen(tag->text);
    } else if (tag->text != NULL) {

        int typed_count = map->press(map, tag, keyhook_info);
        if (map->release == NULL) {
            return typed_count;
        }

        keytyper_string(tag->text, keyhook_info);
        map->release(map, keyhook_info);

        return strlen(tag->text);
    }

    if (tag->tags != NULL) {
        if (!is_text) {
            map->press(map, tag, keyhook_info);
        }

        int replace_len = 0;
        for (size_t i = 0; i < tag->tags_len; i++) {
            replace_len += __keytyper_replace(tag->tags[i], keyhook_info);
            replace_len = (replace_len >= 0) ? replace_len : -1;
        }

        if (!is_text) {
            map->release(map, keyhook_info);
            replace_len = -1;
        }
        return replace_len;
    } else {
        if (map == NULL) {
            if (strlen(tag->name) == 1) {
                keytype_press_release_char(*(tag->name), keyhook_info);
                return 1;
            }
            return 0;
        }

        // Auto releases
        int type_count = map->press(map, tag, keyhook_info);
        return type_count;
    }
}

int keytyper_replace(size_t replace_len, Tag *tag, KeyHookInfo keyhook_info) {
    for (size_t i = 0; i < replace_len; i++) {
        keytyper_key(KEYTYPER_BACKSPACE, keyhook_info);
    }
    return __keytyper_replace(tag, keyhook_info);
}