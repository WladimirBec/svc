/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#ifndef SVC_CFG_H
#define SVC_CFG_H

typedef struct {
    /**
     * Dir containing running services.
     */
    char const *svdir;

    /**
     * Dir containing all available services.
     */
    char const *available;
} cfg;

/**
 * Return the current config.
 */
cfg cfg_get(void);

#endif
