#include "replword.h"

#include "tagmap.h"

#include "utils/array.h"
#include "utils/str.h"

#include "data.h"

void replword_free(ReplaceWord *replword) {
    tag_free(replword->source);
    tag_free(replword->replace);
    free(replword);
}

// ---------------------------------------------------------
//                         Add
// ---------------------------------------------------------

bool __is_positive_num(const char *str) {
    while (*str)
        if (!isdigit(*str++))
            return false;
    return true;
}

int __generate_replword_id() {
    ReplaceWord **replwords = app_data.replwords;
    size_t size = app_data.replwords_len;

    bool *present = (bool *)calloc(size + 1, sizeof(bool));

    for (int i = 0; i < size; i++) {
        if (replwords[i]->id <= size) {
            present[replwords[i]->id] = true;
        }
    }

    for (int i = 0; i <= size; i++) {
        if (!present[i]) {
            free(present);
            return i;
        }
    }

    free(present);
    return size + 1;
}

char *__add_replword(ReplaceWord *replword) {
    mutex_lock(app_data.mutex);

    array_add(app_data.replwords, app_data.replwords_len, replword,
              ReplaceWord);

    mutex_unlock(app_data.mutex);
    return NULL;
}

ReplaceWord *__replword_from_tags(Tag *word, Tag *replace, char **error_msg) {
    if ((*error_msg = tagmap_word_valid(word)) != NULL) {
        return NULL;
    }

    if ((*error_msg = tagmap_replace_valid(replace)) != NULL) {
        return NULL;
    }

    ReplaceWord *replword = malloc(sizeof(ReplaceWord));
    replword->source = word;
    replword->replace = replace;
    replword->temporary = false;

    return replword;
}

char *replword_add_by_request(Request *request) {
    // This function assumes that all errors such as
    // NULL values for word and replace are handled
    // Before the function
    const char *word = request_get_query(request, "word");
    const char *replace = request_get_query(request, "replace");

    Tag *word_tag = tag_parse(word, "word");
    if (word == NULL) {
        tag_free(word_tag);
        return strdup("The given source tag has invalid tag syntax");
    }

    Tag *replace_tag = tag_parse(replace, "replace");
    if (replace_tag == NULL) {
        tag_free(word_tag);
        tag_free(replace_tag);
        return strdup("The given replace tag has invalid tag syntax");
    }

    char *error;
    ReplaceWord *replword = __replword_from_tags(word_tag, replace_tag, &error);
    if (error != NULL) {
        return error;
    }

    const char *temp = request_get_query(request, "temp");
    if (temp != NULL && str_eq(temp, "true"))
        replword->temporary = true;

    const char *id = request_get_query(request, "id");
    if (id != NULL && __is_positive_num(id)) {
        replword->id = atoi(id);
    } else {
        replword->id = __generate_replword_id();
    }

    replword->enabled = true;
    return __add_replword(replword);
}

char *replword_add_by_root_tag(Tag *root) {
    Tag *word = tag_clone(tag_get(root, "word"));
    Tag *replace = tag_clone(tag_get(root, "replace"));
    if (word == NULL) {
        return strdup("<word> tag not found");
    } else if (replace == NULL) {
        return strdup("<replace> tag not found");
    }

    char *error;
    ReplaceWord *replword = __replword_from_tags(word, replace, &error);
    if (error != NULL) {
        return error;
    }

    const char *enabled = tag_get_text(root, "enabled");
    if (enabled != NULL) {
        replword->enabled = (str_eq(enabled, "true") ? true : false);
    } else {
        replword->enabled = true;
    }

    const char *id = tag_get_text(root, "id");
    if (id != NULL && __is_positive_num(id)) {
        replword->id = atoi(id);
    } else {
        replword->id = __generate_replword_id();
    }

    return __add_replword(replword);
}

// ---------------------------------------------------------
//                        Find
// ---------------------------------------------------------

int replword_find(const char *identifier, RWIdentifier itype) {
    int index = -1;

    mutex_lock(app_data.mutex);

    for (size_t i = 0; i < app_data.replwords_len; i++) {
        ReplaceWord *replword = app_data.replwords[i];
        if (itype == REPLWORD_RW_SOURCE &&
            str_eq(replword->source->tag_source, identifier)) {
            index = i;
            break;
        }

        if (itype == REPLWORD_RW_ID && replword->id == atoi(identifier)) {
            index = i;
            break;
        }
    }

    mutex_unlock(app_data.mutex);
    return index;
}

// ---------------------------------------------------------
//                        Remove
// ---------------------------------------------------------

bool replword_remove(const char *identifier, RWIdentifier itype) {
    int index = replword_find(identifier, itype);
    if (index == -1) {
        return false;
    }

    mutex_lock(app_data.mutex);

    replword_free(app_data.replwords[index]);
    app_data.replwords[index] = app_data.replwords[app_data.replwords_len - 1];
    app_data.replwords_len--;

    mutex_unlock(app_data.mutex);
    return true;
}

// ---------------------------------------------------------
//                        Unpack
// ---------------------------------------------------------

Tag *__replword_unpack(ReplaceWord *replword, bool ignore_temp) {
    Tag *tag = tag_new("replword");
    tag_insert(tag, tag_clone(replword->source));
    tag_insert(tag, tag_clone(replword->replace));

    if (replword->temporary) {
        if (ignore_temp) {
            tag_free(tag);
            return NULL;
        }
        tag_insert(tag, tag_new("temporary"));
    }
    Tag *enabled = tag_new("enabled");
    tag_insert_text(enabled, replword->enabled ? "true" : "false");
    tag_insert(tag, enabled);

    Tag *id = tag_new("id");
    char *sid;
    str_format(sid, "%d", replword->id);

    tag_insert_text(id, sid);
    tag_insert(tag, id);

    free(sid);
    return tag;
}

Tag *replword_unpack_all(bool ignore_temp) {
    Tag *root = tag_new("replwords");

    mutex_lock(app_data.mutex);

    for (size_t i = 0; i < app_data.replwords_len; i++) {
        Tag *replword = __replword_unpack(app_data.replwords[i], ignore_temp);

        if (replword != NULL)
            tag_insert(root, replword);
    }

    mutex_unlock(app_data.mutex);
    return root;
}
