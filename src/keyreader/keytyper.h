#pragma once

#include "../utils/tags.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>
#define KEYTYPER_BACKSPACE VK_BACK
#define KEYTYPER_TAB VK_TAB
#define KEYTYPER_RETURN VK_RETURN

#define KEYTYPER_LEFT_ARROW VK_LEFT
#define KEYTYPER_RIGHT_ARROW VK_RIGHT
#define KEYTYPER_UP_ARROW VK_UP
#define KEYTYPER_DOWN_ARROW VK_DOWN

#define KEYTYPER_SHIFT VK_SHIFT
#define KEYTYPER_CONTROL VK_CONTROL

#define KEYTYPER_WIN VK_LWIN

#else
#error "not implemented in NON WINDOWS PLATFORMS"
#endif

// Signals to Keyhook on how to handle the current key press
typedef enum {
    KEYTYPER_KH_NONE,   // Do nothing
    KEYTYPER_KH_IGNORE, // Do not pass the event to keyhook_handle_event(Event)
} KeyHookInfo;

/**
 * @brief Checks if keycode is pressed
 *
 * @param keycode KEYTYPER_* keycode
 * @return true if is pressed else false
 */
bool keytyper_is_pressed(int keycode);

/**
 * @brief Presses the given keycode
 *
 * @param keycode KEYTYPER_* keycode
 * @param keyhook_info The signal to be passed to keyhook
 */
void keytyper_key_press(int vk_code, KeyHookInfo keyhook_info);

/**
 * @brief Releases the given keycode
 *
 * @param keycode KEYTYPER_* keycode
 * @param keyhook_info The signal to be passed to keyhook
 */
void keytyper_key_release(int vk_code, KeyHookInfo keyhook_info);

/**
 * @brief Presses and releases given keycode
 *
 * @param keycode KEYTYPER_* keycode
 * @param keyhook_info The signal to be passed to keyhook
 */
void keytyper_key(int keycode, KeyHookInfo keyhook_info);

/**
 * @brief Types the given character
 *
 * @param c the character to be typed
 * @param keyhook_info The signal to be passed to keyhook
 */
void keytyper_type(unsigned int c, KeyHookInfo keyhook_info);

/**
 * @brief Presses and releases the given character
 *
 * @param c the character to be pressed and released
 * @param keyhook_info The signal to be passed to keyhook
 */
void keytype_press_release_char(char c, KeyHookInfo keyhook_info);

/**
 * @brief Types the given string
 *
 * @param str the string to be typed
 * @param keyhook_info The signal to be passed to keyhook
 */
void keytyper_string(char *str, KeyHookInfo keyhook_info);

/**
 * @brief Replaces the given word with the word contained in *tag.
 *
 * Used in keyhook.c to replace source word with replace word
 *
 * @param replace_len amount of times backspace is to be pressed
 * @param tag the tag containing the replace word
 * @param keyhook_info The signal to be passed to keyhook
 *
 * @return the number of characters typed
 */
int keytyper_replace(size_t replace_len, Tag *tag, KeyHookInfo keyhook_info);