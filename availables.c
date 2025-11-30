/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#include "availables.h"
#include "err.h"
#include "io.h"

arr_of(char *) availables_get(cfg *config)
{
    return io_list_dirs(config->available);
}

int
availables_exist(cfg *config, char const *name)
{
    char path[512] = {0};
    if (io_snprintf(path, 512, "%s/%s", config->available, name) == -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    return io_exists(path);
}
