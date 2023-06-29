#pragma once

#include <stdbool.h>

typedef struct {
    char character;

    bool is_keydown;
    bool is_ctrldown;

} KeyEvent;

/**
 * @brief Runs the os dependent keyhook. Passes all key events to
 * keyhook_handle_event(KeyEvent event) after parsing.
 *
 * Implemented in keyhook_raw.c containing platform specific code
 */
void keyhook_raw_run();

/**
 * @brief Posts a quit message which closes the raw keyhook
 *
 * Implemented in keyhook_raw.c containing platform specific code
 */
void keyhook_raw_quit();

/**
 * @brief Resets keyhook variables and keybuffer after mouse click, ctrl + key,
 * etc...
 */
void keyhook_reset();

/**
 * @brief Handles the KeyEvent passed from keyhook_raw_run()
 */
bool keyhook_handle_event(KeyEvent event);

/**
 * @brief Runs the actual keyhook
 */
void keyhook_run();
