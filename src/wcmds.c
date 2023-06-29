#include "wcmds.h"

#include "data.h"
#include "tagmap.h"

#include "http/http.h"
#include "http/http_client.h"

#include "keyreader/keybuffer.h"
#include "keyreader/keyhook.h"
#include "keyreader/keytyper.h"

// ---------------------------------------------------------
//                        Utils
// ---------------------------------------------------------

#define __check_server_running(port)                                           \
    do {                                                                       \
        port = http_get_active_port();                                         \
        if (port <= 0) {                                                       \
            printf("Error: wdrec server is not started please run wdrec "      \
                   "first\n");                                                 \
            return 1;                                                          \
        }                                                                      \
    } while (0)

char *__execute_request(int port, const char *url) {
    int error;
    Response *response = chttp_request(port, url, &error);
    if (error == 1) {
        printf("Error: wdrec server is not started please run wdrec first\n");
        return NULL;
    } else if (error == 2) {
        printf("Error: Timeout (May be due to another app using port \"%d\")\n",
               port);
        return NULL;
    }

    if (response->status_code == 400) {
        printf("Error: %s\n", response->body);
        response_free(response);
        return NULL;
    }
    char *body = strdup(response->body);
    response_free(response);
    return body;
}

void __run_in_background() {
#ifdef _WIN32
    SetEnvironmentVariable("WDREC_BACKGROUND", "true");
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    if (CreateProcess(NULL, GetCommandLineA(), NULL, NULL, FALSE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
#else
#error "not implemented in NON WINDOWS PLATFORMS"
#endif
}

RWIdentifier __append_identifier(Args *args, char **url, const char *word,
                                 const char *print_info) {

    RWIdentifier ident = -1;
    char *dword = url_decode(word);

    if (argparse_flag_found(args, "--id")) {
        str_rformat(*url, "id=%s", word);
        printf("%s by id: \"%s\"\n", print_info, dword);
        ident = REPLWORD_RW_ID;
    } else {
        str_rformat(*url, "word=%s", word);
        printf("%s by source: \"%s\"\n", print_info, dword);
        ident = REPLWORD_RW_SOURCE;
    }

    free(dword);
    return ident;
}

// ---------------------------------------------------------
//                      Subcommands
// ---------------------------------------------------------

int wcmd_start_server(Args *args) {
    int port = atoi(argparse_flag_get(args, "--port"));
    if (port <= 0) {
        argparse_print_help(args->parser);
        printf("Error: Invalid port number. Port number must be above 0");
        return 1;
    }

    if (!http_init(port, "127.0.0.1")) {
        if (http_get_active_port() == -1) {
            printf("Error: Some other application maybe using port '%d'. "
                   "Please change the port using --port option\n",
                   port);
            return 1;
        }

        printf("Error: wdrec is already running\n");
        return 1;
    }

#ifdef NDEBUG
    if (getenv("WDREC_BACKGROUND") == NULL) {
        __run_in_background();
        return 0;
    }
#endif
    data_init();

    http_start();
    keyhook_run(); // Exits When keyhook_raw_quit() is called

    http_stop();
    data_free();

    return 0;
}

int wcmd_close_server(Args *args) {
    int port;
    __check_server_running(port);

    char *body = __execute_request(port, "/close");
    if (body == NULL) {
        return 1;
    }
    printf("%s\n", body);
    free(body);

    return 0;
}

int wcmd_add_word(Args *args) {
    int port;
    __check_server_running(port);

    char *word = url_encode(argparse_positional_get(args, "word"));
    char *replace = url_encode(argparse_positional_get(args, "replace"));
    bool found = argparse_flag_found(args, "-t");

    char *url;
    str_format(url, "/add?word=%s&replace=%s&temp=%s", word, replace,
               (found) ? "true" : "false");

    printf("Adding %s -> %s\n", argparse_positional_get(args, "word"),
           argparse_positional_get(args, "replace"));

    char *body = __execute_request(port, url);
    if (body == NULL) {
        free(url);
        free(word);
        free(replace);
        return 1;
    }
    printf("%s\n", body);
    free(body);

    free(url);
    free(word);
    free(replace);

    return 0;
}

int wcmd_remove_word(Args *args) {
    int port;
    __check_server_running(port);

    char *identifier = url_encode(argparse_positional_get(args, "identifier"));
    char *url;

    str_mcpy(url, "/remove?");
    RWIdentifier ident =
        __append_identifier(args, &url, identifier, "Removing word");
    if (ident == -1) {
        // Currently this case will not occur
        printf("Invalid Identifier\n");
        free(identifier);
        free(url);
        return 1;
    }

    char *body = __execute_request(port, url);
    if (body == NULL) {
        free(identifier);
        free(url);
        return 1;
    }
    printf("%s\n", body);

    free(identifier);
    free(body);
    free(url);

    return 0;
}

int wcmd_list_words(Args *args) {
    int port;
    __check_server_running(port);

    char *body = __execute_request(port, "/list");
    if (body == NULL) {
        return 1;
    }

    Tag *root = tag_parse(body, "root");

    if (root->tags_len == 0) {
        printf("Empty: No active replace words\n");
    } else if (root != NULL) {
        int replwords_len = 0;
        int max_len = 0;
        Tag **replwords = tag_get_all(root, "replword", &replwords_len);

        for (size_t i = 0; i < replwords_len; i++) {
            Tag *replword = replwords[i];

            printf("\nWord: %s\n", tag_get(replword, "word")->tag_source);
            printf("Replace: %s\n", tag_get(replword, "replace")->tag_source);
            printf("Id: %s\n", tag_get(replword, "id")->tag_source);
            printf("Enabled: %s\n", tag_get(replword, "enabled")->tag_source);
        }
        free(replwords);
    }

    tag_free(root);
    free(body);

    return 0;
}

int wcmd_config_word(Args *args) {
    int port;
    __check_server_running(port);

    const char *identifier = argparse_positional_get(args, "identifier");

    char *url;
    str_mcpy(url, "/config?");

    RWIdentifier ident =
        __append_identifier(args, &url, identifier, "Configuring word");
    if (ident == -1) {
        // Currently this case will not occur
        printf("Invalid Identifier\n");
        free(url);
        return 1;
    }

    if (argparse_flag_found(args, "--enable")) {
        str_rcat(url, "&enable=true");
    } else if (argparse_flag_found(args, "--disable")) {
        str_rcat(url, "&enable=false");
    }

    char *body = __execute_request(port, url);
    if (body == NULL) {
        free(url);
        return 1;
    }
    printf("%s\n", body);
    free(body);
    free(url);

    return 0;
}