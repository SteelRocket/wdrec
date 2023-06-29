#pragma once

#include "utils/argparse.h"

#include <stdbool.h>

int wcmd_start_server(Args *args);
int wcmd_close_server(Args *args);

int wcmd_add_word(Args *args);
int wcmd_remove_word(Args *args);

int wcmd_list_words(Args *args);

int wcmd_config_word(Args *args);
