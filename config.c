/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#include "config.h"
#include <stdlib.h>

#define AVDIR_DEFAULT "/etc/sv"
#define SVDIR_DEFAULT "/var/service"

cfg
cfg_get(void)
{
    char *svdir = getenv("SVDIR");
    if (svdir == NULL) {
        svdir = SVDIR_DEFAULT;
    }

    char *available = getenv("AVDIR");
    if (available == NULL) {
        available = AVDIR_DEFAULT;
    }

    return (cfg){
        .svdir     = svdir,
        .available = available,
    };
}
