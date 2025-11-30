/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#ifndef SVC_ARR_H
#define SVC_ARR_H

#include <stdlib.h>
#include <string.h>

/**
 * A generic fat pointer array of T*
 *
 * 0: size_t length
 * 1: size_t capacity
 * 2: T*
 */
#define arr_of(T) T *
#define arr_ptr   arr_of(void *)

/**
 * Get the length of the given array.
 */
#define arr_len(a) (((size_t *)(a))[-2])

/**
 * Get the capacity of the given array.
 */
#define arr_cap(a) (((size_t *)(a))[-1])

/**
 * Get the beginning of the fat pointer array.
 */
#define arr_head(a) ((size_t *)(a) - 2)

/**
 * Wrapper to call free on every array elements and then call arr_free on the
 * array.
 */
#define arr_free_free(a, free)                    \
    do {                                          \
        for (size_t i = 0; i < arr_len(a); ++i) { \
            free((a)[i]);                         \
        }                                         \
        arr_free(a);                              \
    } while (0);

/**
 * Allocate of grow an existing array by l size, if a is NULL, allocate a new
 * array of capacity l, otherwise expand a by l.
 *
 * Returns the array or NULL and set last_error.
 */
static inline arr_ptr
arr_alloc(arr_ptr a, size_t l)
{
    size_t *head = a == NULL ? NULL : arr_head(a);
    size_t cap   = a == NULL ? l : (arr_cap(a) + l);
    size_t *p    = realloc(head, sizeof(size_t) * 2 + sizeof(void *) * cap);

    if (p == NULL) {
        return NULL;
    }

    // set the length manually if it's a new array
    if (a == NULL) {
        p[0] = 0;
    }

    arr_ptr b  = (arr_ptr)(p + 2);
    arr_cap(b) = cap;
    memset(b + arr_len(b), 0, sizeof(*b) * (l - arr_len(b)));
    return b;
}

/**
 * Call arr_alloc on the given *a if needed.
 *
 * Returns -1 on error and set errno.
 */
static inline int
arr_alloc_maybe(arr_ptr *a, size_t l)
{
    if (*a == NULL) {
        if ((*a = arr_alloc(NULL, l)) == NULL) {
            return -1;
        }
        return 0;
    }

    size_t len = arr_len(*a) + l;
    if (len > arr_cap(*a)) {
        if ((*a = arr_alloc(*a, len + (len << 1))) == NULL) {
            return -1;
        }
    }

    return 0;
}

/**
 * Free the given array.
 */
static inline void
arr_free(arr_ptr a)
{
    if (a != NULL) {
        free(arr_head(a));
    }
}

/**
 * Append an element to the end of the array, automatically grows it if
 * required.
 *
 * Returns -1 on error and set errno.
 */
static inline int
arr_append(arr_ptr *a, void *e)
{
    if (arr_alloc_maybe(a, 1) < 0) {
        return -1;
    }

    (*a)[arr_len(*a)++] = e;
    return 0;
}

#endif
