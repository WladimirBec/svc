/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#ifndef SVC_IO_H
#define SVC_IO_H

#include "arr.h"

/**
 * Returns the list of present directories inside the given path, the list and
 * its elements must be freed upon usage with `arr_free_free(list, free)`.
 *
 * Returns NULL on error and set last_error.
 */
arr_of(char *) io_list_dirs(char const *path);

/**
 * Returns 1 if the given path exists, otherwise 0.
 *
 * Returns -1 on error and set last_error.
 */
int io_exists(char const *path);

/**
 * Returns 1 if the given name exists relative to fd, otherwise 0.
 *
 * Reuturns -1 on error and set last_error.
 */
int io_existsat(int fd, char const *name);

/**
 * Wrapper around snprintf that also returns an error when overflow.
 *
 * Returns -1 on error and set last_error.
 */
int io_snprintf(char *buf, size_t buf_len, char const *fmt, ...);

/**
 * Wrapper around `open()` and `read()`, returns the amount of data read.
 *
 * Returns -1 on error and set last_error.
 */
int io_readat(int fd, char const *path, char *buf, size_t buf_len);

/**
 * Wrapper around `open()` and `write()`, returns the amount of wrote read.
 *
 * Returns -1 on error and set last_error.
 */
int io_writeat(int fd, char const *path, char *buf, size_t buf_len);

#endif
