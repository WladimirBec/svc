/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#ifndef SVC_SVC_H
#define SVC_SVC_H

#include "arr.h"
#include "config.h"
#include <sys/types.h>

/**
 * Represents the status of a service.
 */
typedef enum {
    SVC_RUNNING,
    SVC_STOPPED,
    SVC_FINISHING,
    SVC_UNKNOWN,
} svc_status;

/**
 * Returns a string representing the given enum value.
 */
char const *svc_status_str(svc_status status);

/**
 * Represents the time since the last status of service changed.
 */
typedef char svc_time[24 + 1];

/**
 * Represents a runit service.
 */
typedef struct {
    svc_status status;
    int is_down;
    pid_t pid;
    svc_time time;
    char name[];
} svc;

/**
 * Returns a list of current services in $SVDIR, the list and its elements must
 * be freed upon usage with `arr_free_free(list, free)`.
 *
 * Returns NULL on error and set last_error.
 */
arr_of(svc *) svc_list(cfg *config);

/**
 * Returns 1 if the given service name is linked, otherwise 0.
 *
 * Returns -1 on error and set last_error.
 */
int svc_linked(cfg *config, char const *name);

/**
 * Links the given service name.
 *
 * Returns -1 on error and set last_error.
 */
int svc_link(cfg *config, char const *name);

/**
 * Unlinks the given service name.
 *
 * Returns -1 on error and set last_error.
 */
int svc_unlink(cfg *config, char const *name);

/**
 * Send a control command to the given service name.
 *
 * Returns -1 on error and set last_error.
 */
int svc_control(cfg *config, char const *name, char command);

/**
 * Returns 1 if the given service name is currently running, otherwise 0.
 *
 * Returns -1 on error and set last_error.
 */
int svc_running(cfg *config, char const *name);

/**
 * Returns 1 if the given service name is down, otherwise 0.
 *
 * Returns -1 on error and set last_error.
 */
int svc_is_down(cfg *config, char const *name);

/**
 * Downs the given service name.
 *
 * Returns -1 on error and set last_error.
 */
int svc_down(cfg *config, char const *name);

/**
 * Ups the given service name.
 *
 * Returns -1 on error and set last_error.
 */
int svc_up(cfg *config, char const *name);

#endif
