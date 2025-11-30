/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#ifndef SVC_AVAILABLES_H
#define SVC_AVAILABLES_H

#include "arr.h"
#include "config.h"

/**
 * Return the list of available services, the list and its elements must be
 * freed upon usage with `arr_free_free(list, free)`.
 *
 * Returns NULL on error and set last_error.
 */
arr_of(char *) availables_get(cfg *config);

/**
 * Returns 1 if the given exists in the available services, otherwise 0.
 *
 * Returns -1 on error and set last_error.
 */
int availables_exist(cfg *config, char const *name);

#endif
