/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#include "io.h"
#include "err.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

static int
sort(void const *a, void const *b)
{
    return strcmp(*(arr_of(char *))a, *(arr_of(char *))b);
}

static arr_of(char *) list(DIR *d)
{
    arr_of(char *) arr = (arr_of(char *))arr_alloc(NULL, 8);
    if (arr == NULL) {
        set_last_error("failed to allocate array: %s", strerror(errno));
        return NULL;
    }

    while (1) {
        errno            = 0;
        struct dirent *e = readdir(d);

        if (e == NULL) {
            if (errno != 0) {
                set_last_error("failed to read dir: %s", strerror(errno));
            err:
                arr_free_free((arr_ptr)arr, free);
                arr = NULL;
            }

            break;
        }

        if (e->d_name[0] == '.') {
            continue;
        }

        char *name = strdup(e->d_name);
        if (name == NULL) {
            set_last_error("strdup failed: %s", strerror(errno));
            goto err;
        }
        if (arr_append((arr_ptr *)&arr, name) < 0) {
            set_last_error("failed to append to array: %s", strerror(errno));
            free(name);
            goto err;
        }
    }

    return arr;
}

arr_of(char *) io_list_dirs(char const *path)
{
    DIR *d = opendir(path);
    if (d == NULL) {
        set_last_error("failed to open dir '%s': %s", path, strerror(errno));
        return NULL;
    }

    arr_of(char *) arr = list(d);
    if (arr != NULL) {
        qsort(arr, arr_len(arr), sizeof(*arr), sort);
    }

    closedir(d);
    return arr;
}

int
io_exists(char const *path)
{
    return io_existsat(-1, path);
}

int
io_existsat(int fd, char const *name)
{
    if (faccessat(fd, name, F_OK, 0) == 0) {
        return 1;
    } else if (errno == ENOENT) {
        return 0;
    }

    set_last_error("access failed: %s", strerror(errno));
    return -1;
}

int
io_snprintf(char *buf, size_t buf_len, char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(buf, buf_len, fmt, ap);
    va_end(ap);

    if (r >= (int)buf_len) {
        set_last_error("%s", strerror(EOVERFLOW));
        return -1;
    } else if (r == -1) {
        set_last_error("%s", strerror(errno));
        return -1;
    }

    return 0;
}

int
io_readat(int fd, char const *path, char *buf, size_t buf_len)
{
    int f = openat(fd, path, O_RDONLY);
    if (f == -1) {
        set_last_error("open failed: %s", strerror(errno));
        return -1;
    }

    int n = read(f, buf, buf_len);
    if (n == -1) {
        set_last_error("%s", strerror(errno));
    }

    close(f);
    return n;
}

int
io_writeat(int fd, char const *path, char *buf, size_t buf_len)
{
    int f = openat(fd, path, O_WRONLY);
    if (f == -1) {
        set_last_error("open failed: %s", strerror(errno));
        return -1;
    }

    int n = write(f, buf, buf_len);
    if (n == -1) {
        set_last_error("%s", strerror(errno));
    }

    close(f);
    return n;
}
