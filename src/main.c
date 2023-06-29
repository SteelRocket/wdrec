#define UTILS_IMPLEMENTATION
#include "utils/socket.h"

#include "utils/array.h"
#include "utils/http_request.h"
#include "utils/http_response.h"
#include "utils/path.h"
#include "utils/str.h"
#include "utils/thread.h"
#include "utils/url.h"

#include "wcmds.h"
#include "wparser.h"

// ---------------------------------------

int main(int argc, char *argv[]) {
    ArgParser *parser = wparser_init();
    Args *args = argparse_parse(parser, argc, argv);

    if (args == NULL) {
        return 1;
    }

    socket_init();

    const char *cmd_name = args->parser->name;
    int exit_code;

    if (str_eq(cmd_name, "wdrec") || str_eq(cmd_name, "server")) {
        exit_code = wcmd_start_server(args);
    } else if (str_eq(cmd_name, "add")) {
        exit_code = wcmd_add_word(args);
    } else if (str_eq(cmd_name, "remove")) {
        exit_code = wcmd_remove_word(args);
    } else if (str_eq(cmd_name, "close")) {
        exit_code = wcmd_close_server(args);
    } else if (str_eq(cmd_name, "list")) {
        exit_code = wcmd_list_words(args);
    } else if (str_eq(cmd_name, "config")) {
        exit_code = wcmd_config_word(args);
    }

    socket_cleanup();

    argparse_free_args(args);
    argparse_free(parser);
    return exit_code;
}
