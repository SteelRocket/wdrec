#pragma once

#include "../utils/tags.h"

#include <stdbool.h>

extern char *keybuffer;

extern size_t keybuffer_size;
// The position of key cursor within keybuffer
extern size_t keybuffer_cursor;

/**
 * @brief Moves the key cursor as per offset
 *
 * @param offset Moves to left if offset > 0 and moves to right if offset < 0
 * @return true if cursor was able(within keybuffer range) to move, else returns
 * false
 */
bool keybuffer_cursor_move(int offset);

/**
 * @brief (Debug) Prints all the characters in keybuffer
 */
void keybuffer_print();

/**
 * @brief Empties the keybuffer to have 0 characters
 */
void keybuffer_empty();

/**
 * @brief Adds a character to keybuffer at the keybuffer_cursor
 *
 * @param c the character to add
 */
void keybuffer_push(char c);

/**
 * @brief Adds a string to keybuffer at the keybuffer_cursor
 *
 * @param str the string to add
 */
void keybuffer_append(char *str);

/**
 * @brief Removes a character from keybuffer at keybuffer_cursor
 */
void keybuffer_pop();

/**
 * @brief Replaces a word(or letter) with another word(or letter) in keybuffer
 * at keybuffer_cursor
 *
 * @param pop_len how many letter(s) to remove
 * @param rstr the replace string
 */
void keybuffer_replace(size_t pop_len, char *rstr);

/**
 * @brief Checks if keybuffer endswith the given tag
 *
 * @param tag (ReplaceWord->word) tag to check if keybuffer ends with it
 * @return returns the word length of the tag
 */
int keybuffer_endswith(Tag *tag);
