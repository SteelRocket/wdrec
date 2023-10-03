#include "wparser.h"

#define __LEN(a) sizeof(a) / sizeof(a[0])

// ---------------------------------------------------------
//                    Server Command
// ---------------------------------------------------------

Option __server_opts[] = {{
    .flags = "-p,--port",
    .help = "Port of the api server",
    .has_value = true,
    .default_value = "8000",
    .is_required = false,
}};

ArgParser *__wparser_server() {
    return argparse_init("server", "Starts the wdrec server", __server_opts,
                         __LEN(__server_opts), NULL, 0);
}

// ---------------------------------------------------------
//                      Add Command
// ---------------------------------------------------------

Option __add_opts[] = {{
    .flags = "-t,--tmp",
    .help = "Sets the replace word to be temporary",
    .has_value = false,
}};

Positional __add_pos[] = {
    {
        .name = "word",
        .required = true,
    },
    {
        .name = "replace",
        .required = true,
    },
};

ArgParser *__wparser_add() {

    return argparse_init("add", "Adds a word to be replaced", __add_opts,
                         __LEN(__add_opts), __add_pos, __LEN(__add_pos));
}

// ---------------------------------------------------------
//                      Remove Command
// ---------------------------------------------------------

Positional __remove_pos[] = {{
    .name = "identifier",
    .required = true,
}};

Option __remove_opts[] = {{
    .flags = "-i,--id",
    .help = "Removes the word by id",
    .has_value = false,
    .is_required = false,
}};

ArgParser *__wparser_remove() {
    return argparse_init("remove", "Removes a word from the replace list",
                         __remove_opts, __LEN(__remove_opts), __remove_pos,
                         __LEN(__remove_pos));
}

// ---------------------------------------------------------
//        Remove Command
// ---------------------------------------------------------

ArgParser *__wparser_close() {
    return argparse_init("close", "Closes the running wdrec instance", NULL, 0,
                         NULL, 0);
}

// ---------------------------------------------------------
//                      List Command
// ---------------------------------------------------------

ArgParser *__wparser_list() {
    return argparse_init("list", "List the given replace words", NULL, 0, NULL,
                         0);
}

// ---------------------------------------------------------
//                      Config Command
// ---------------------------------------------------------
Option __config_opts[] = {
    {
        .flags = "-i,--id",
        .help = "sets config based on id of the word",
        .has_value = false,
        .is_required = false,
    },
    {
        .flags = "--enable,-e",
        .help = "Activates the disabled word",
        .has_value = false,

        .conflicting_flags = "--disable,-d",
        .is_required = false,
    },
    {
        .flags = "--disable,-d",
        .help = "Disables the active word",
        .has_value = false,

        .conflicting_flags = "--enable,-e",
        .is_required = false,
    },
};

Positional __config_pos[] = {
    {
        .name = "identifier",
        .required = true,
    },
};

ArgParser *__wparser_config() {
    return argparse_init("config", "Modifies config of a word", __config_opts,
                         __LEN(__config_opts), __config_pos,
                         __LEN(__config_pos));
}

// ---------------------------------------------------------
//              Default(no subparser) Command
// ---------------------------------------------------------
Option __default_opts[] = {
    {
        .flags = "-p,--port",
        .help = "Port of the api server",
        .has_value = true,
        .default_value = "8000",
        .is_required = false,
    },
    {
        .flags = "-v,--version",
        .help = "Returns wdrec version",
        .has_value = false,
        .is_required = false,
    },
};

ArgParser *__wparser_default() {
    return argparse_init("wdrec", "Command line app to replace typed words",
                         __default_opts, __LEN(__default_opts), NULL, 0);
}

ArgParser *wparser_init() {
    ArgParser *wdrec_parser = __wparser_default();

    argparse_add_subparser(wdrec_parser, __wparser_server());
    argparse_add_subparser(wdrec_parser, __wparser_close());
    argparse_add_subparser(wdrec_parser, __wparser_add());
    argparse_add_subparser(wdrec_parser, __wparser_remove());
    argparse_add_subparser(wdrec_parser, __wparser_list());
    argparse_add_subparser(wdrec_parser, __wparser_config());

    return wdrec_parser;
}