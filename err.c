/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#include "err.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define err_len 512
static char err[err_len];

char const *
get_last_error(void)
{
    return err;
}

static void
vset_last_error(char const *fmt, va_list ap)
{
    vsnprintf(err, err_len, fmt, ap);
}

void
set_last_error(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vset_last_error(fmt, ap);
    va_end(ap);
}

void
wrap_last_error(char const *fmt, ...)
{
    char err_previous[err_len] = {0};
    memcpy(err_previous, err, err_len);

    va_list ap;
    va_start(ap, fmt);
    vset_last_error(fmt, ap);
    va_end(ap);

    if (err_previous[0] != '\0') {
        char err_current[err_len] = {0};
        memcpy(err_current, err, err_len);

        set_last_error("%s: %s", err_current, err_previous);
    }
}

void
clear_last_error(void)
{
    err[0] = '\0';
}

void
print_last_error(char const *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (err[0] != '\0') {
        fprintf(stderr, ": %s", err);
    }

    fputc('\n', stderr);
}
