#pragma once

#include "utils/tags.h"
#include "utils/thread.h"

#include "replword.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    Mutex *mutex; // Thread mutex Shared between keyhook and http server

    ReplaceWord **replwords; // The array of ReplaceWords
    size_t replwords_len;
} AppData;

// Global variable to share data between http server and keyhook
extern AppData app_data;

/**
 * @brief Gets the user data folder to save files
 */
char *data_get_dir();

/**
 * @brief Saves all replwords (non temporary) to a .tag file
 */
void data_save();

/**
 * @brief initalizes app_data global variable and loads all ReplaceWords
 */
void data_init();

/**
 * @brief frees the app_data global variable
 */
void data_free();