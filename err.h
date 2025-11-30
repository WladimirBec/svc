/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#ifndef SVC_ERR_H
#define SVC_ERR_H

/**
 * Returns the last error that happened.
 */
char const *get_last_error(void);

/**
 * Wrapper around printf to set the last error.
 */
void set_last_error(char const *fmt, ...);

/**
 * Wrap the last set error with the given printf arguments.
 */
void wrap_last_error(char const *fmt, ...);

/**
 * Clear the last set error.
 */
void clear_last_error(void);

/**
 * Print an error message and the last_error if set.
 */
void print_last_error(char const *fmt, ...);

#endif
