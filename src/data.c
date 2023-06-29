#include "data.h"

#include "tagmap.h"

#include "utils/array.h"
#include "utils/path.h"
#include "utils/str.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __REPLWORD_SAVE_FILE "replword.tag"

AppData app_data;
char *__data_dir;

char *data_get_dir() {

// Function to get the local data path on Windows
#ifdef _WIN32
    char *data_dir = getenv("LOCALAPPDATA");
#else
    char *data_dir = getenv("HOME");
#endif
    if (data_dir == NULL) {
        return NULL;
    }

    char *path = path_join(data_dir, "wdrec");

    return path;
}

void data_save() {
    if (!path_exists(__data_dir))
        path_make_tree(__data_dir);

    char *file_path = path_join(__data_dir, __REPLWORD_SAVE_FILE);
    FILE *file = fopen(file_path, "w");

    Tag *replwords = replword_unpack_all(true);
    char *tag_string = tag_format(replwords, false);
    fprintf(file, "%s", tag_string);

    free(tag_string);
    tag_free(replwords);

    fclose(file);
    free(file_path);
}

void __data_load() {
    if (!path_exists(__data_dir)) {
        path_make_tree(__data_dir);
        return;
    }
    char *file_path = path_join(__data_dir, __REPLWORD_SAVE_FILE);

    if (!path_is_file(file_path)) {
        free(file_path);
        return;
    }

    char *contents = path_read_all(file_path);

    Tag *root = tag_parse(contents, "root");
    if (root == NULL) {
        free(contents);
        free(file_path);
        return;
    }

    int replword_count;
    Tag **replword_tag_array = tag_get_all(root, "replword", &replword_count);

    for (size_t i = 0; i < replword_count; i++) {
        char *error = replword_add_by_root_tag(replword_tag_array[i]);
        if (error != NULL) {
            free(error);
        }
    }

    if (replword_tag_array != NULL)
        free(replword_tag_array);
    tag_free(root);

    free(contents);
    free(file_path);
}

void data_init() {
    app_data.mutex = mutex_create();

    app_data.replwords = array_create(ReplaceWord *);
    app_data.replwords_len = 0;

    __data_dir = data_get_dir();
    __data_load();
}

void data_free() {
    data_save();

    mutex_lock(app_data.mutex);

    free(__data_dir);
    for (size_t i = 0; i < app_data.replwords_len; i++) {
        replword_free(app_data.replwords[i]);
    }
    free(app_data.replwords);

    mutex_unlock(app_data.mutex);
    mutex_destroy(app_data.mutex);
}
