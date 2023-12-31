#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif

char *path_join(const char *path1, const char *path2);

bool path_exists(const char *path);

bool path_is_dir(const char *path);

bool path_is_file(const char *path);

bool path_make_dir(const char *path);

bool path_make_tree(const char *path);

char *path_read_all(const char *file_path);

#ifdef UTILS_IMPLEMENTATION

#include "str.h"

#ifdef _WIN32
#define _WINSOCKAPI_
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

char *path_join(const char *path1, const char *path2) {

    bool sep_in_path1 = path1[strlen(path1) - 1] == *PATH_SEPERATOR;
    bool sep_in_path2 = *path2 == *PATH_SEPERATOR;

    char *joined;
    if (sep_in_path1 || sep_in_path2) {
        str_format(joined, "%s%s", path1,
                   path2 + (sep_in_path1 && sep_in_path2));
        return joined;
    }
    str_format(joined, "%s" PATH_SEPERATOR "%s", path1, path2);
    return joined;
}

bool path_exists(const char *path) {
#if defined(_WIN32) || defined(_WIN64)
    DWORD attrib = GetFileAttributesA(path);
    return (attrib != INVALID_FILE_ATTRIBUTES);
#else
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
#endif
}

bool path_is_dir(const char *path) {
#if defined(_WIN32) || defined(_WIN64)
    DWORD attrib = GetFileAttributesA(path);
    return (attrib != INVALID_FILE_ATTRIBUTES &&
            (attrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

bool path_is_file(const char *path) {
    return path_exists(path) && !path_is_dir(path);
}

bool path_make_dir(const char *path) {
#ifdef _WIN32
    return CreateDirectoryA(path, NULL) != 0;
#else
    return mkdir(path, 0777) == 0;
#endif
}

bool path_make_tree(const char *path) {
    char *copy_path = strdup(path);
    char *slash = strchr(copy_path, *PATH_SEPERATOR);

    while (slash != NULL) {
        char temp = *slash;
        *slash = '\0';

        if (!path_is_dir(copy_path)) {
            if (!path_make_dir(copy_path))
                return false;
        }

        *slash = temp;
        slash = strchr(slash + 1, *PATH_SEPERATOR);
    }

    free(copy_path);
    return path_make_dir(path);
}

char *path_read_all(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read > file_size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);

    return buffer;
}

#endif