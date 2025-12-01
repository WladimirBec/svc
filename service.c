/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#include "service.h"
#include "err.h"
#include "io.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

char const *
svc_status_str(svc_status status)
{
    switch (status) {
    case SVC_RUNNING:   return "running";
    case SVC_STOPPED:   return "stopped";
    case SVC_FINISHING: return "finishing";
    default:            return "unknown";
    }
}

static svc_status
svc_status_from_str(char const *s)
{
    if (strncmp(s, "finish", 6) == 0) {
        return SVC_FINISHING;
    } else if (strncmp(s, "down", 4) == 0) {
        return SVC_STOPPED;
    } else if (strncmp(s, "run", 3) == 0) {
        return SVC_RUNNING;
    }

    return SVC_UNKNOWN;
}

static int
get_status(int fd)
{
    char buf[6 + 1] = {0}; // "finish" == 6 + 1
    if (io_readat(fd, "supervise/stat", buf, 6) < 0) {
        return -1;
    }

    return svc_status_from_str(buf);
}

static pid_t
get_pid(int fd)
{
    // usually /proc/sys/kernel/pid_max is set at 32768, our buf is 16 char
    // wide which should be more than enough to store any pid even if pid_max
    // has been changed to a greater value
    char buf[16 + 1] = {0};

    if (io_readat(fd, "supervise/pid", buf, 16) < 0) {
        return -1;
    }

    pid_t pid = 0;
    if ((errno = 0, pid = strtoimax(buf, NULL, 10)) == 0 || errno != 0) {
        // the supervise/pid can for a very short time be empty, for example if
        // we check the pid file right after starting a service.
        pid = 0;
    }

    return pid;
}

static int
get_time(int fd, svc_time *t)
{
    struct stat sb = {0};
    if (fstatat(fd, "supervise/stat", &sb, 0) == -1) {
        set_last_error("stat failed: %s", strerror(errno));
        return -1;
    }

    time_t now = time(NULL);
    if (now == ((time_t)-1)) {
        set_last_error("failed to get time: %s", strerror(errno));
        return -1;
    }

    unsigned long mtime = difftime(now, sb.st_mtime);
    unsigned int h      = mtime / 3600;
    mtime %= 3600;
    unsigned int m = mtime / 60;
    unsigned int s = mtime % 60;

    if (io_snprintf(*t, sizeof(*t) / sizeof(**t), "%02d:%02d:%02d", h, m, s) ==
        -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    return 0;
}

static svc *
svc_new(int fd, char const *name)
{
    int f = openat(fd, name, O_RDONLY);
    if (f == -1) {
        set_last_error("failed to open %s: %s", name, strerror(errno));
        return NULL;
    }

    svc *s = NULL;

    int status = get_status(f);
    if (status == -1) {
        wrap_last_error("failed to get status of %s", name);
        goto end;
    }

    int is_down = io_existsat(f, "down");
    if (is_down == -1) {
        wrap_last_error("failed to check if %s is down", name);
        goto end;
    }

    pid_t pid = get_pid(f);
    if (pid == -1) {
        wrap_last_error("failed to get pid of %s", name);
        goto end;
    }

    svc_time time = {0};
    if (get_time(f, &time) == -1) {
        wrap_last_error("failed to get pid of %s", name);
        goto end;
    }

    s = calloc(1, sizeof(*s) + (sizeof(*s->name) * strlen(name) + 1));
    if (s == NULL) {
        set_last_error("calloc failed: %s", strerror(errno));
    } else {
        s->status  = status;
        s->is_down = is_down;
        s->pid     = pid;
        memcpy(s->time, time, sizeof(time));
        strcpy(s->name, name);
    }

end:
    close(f);
    return s;
}

arr_of(svc *) list_services(char const *svdir, arr_of(char *) entries)
{
    int fd = open(svdir, O_RDONLY | O_RDONLY);
    if (fd == -1) {
        set_last_error("failed to open dir '%s': %s", svdir, strerror(errno));
        return NULL;
    }

    arr_of(svc *) list = (arr_of(svc *))arr_alloc(NULL, 8);
    if (list == NULL) {
        set_last_error("failed to allocate array: %s", strerror(errno));
    } else {
        for (size_t i = 0; i < arr_len(entries); ++i) {
            svc *service = svc_new(fd, entries[i]);
            if (service == NULL) {
                wrap_last_error("failed to create svc '%s'", entries[i]);
            err:
                arr_free_free((arr_ptr)list, free);
                list = NULL;
                break;
            }

            if (arr_append((arr_ptr *)&list, service) < 0) {
                set_last_error("append to array failed: %s", strerror(errno));
                free(service);
                goto err;
            }
        }
    }

    close(fd);
    return list;
}

arr_of(svc *) svc_list(cfg *config)
{
    arr_of(char *) entries = io_list_dirs(config->svdir);
    if (entries == NULL) {
        wrap_last_error("failed to list dirs in '%s'", config->svdir);
        return NULL;
    }

    arr_of(svc *) list = list_services(config->svdir, entries);
    arr_free_free((arr_ptr)entries, free);
    return list;
}

int
svc_linked(cfg *config, char const *name)
{
    char path[512] = {0};
    if (io_snprintf(path, 512, "%s/%s", config->svdir, name) == -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    return io_exists(path);
}

int
svc_link(cfg *config, char const *name)
{
    char from[512] = {0};
    if (io_snprintf(from, 512, "%s/%s", config->available, name) == -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    char to[512] = {0};
    if (io_snprintf(to, 512, "%s/%s", config->svdir, name) == -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    if (symlink(from, to) == -1) {
        set_last_error("symlink failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int
svc_unlink(cfg *config, char const *name)
{
    char path[512] = {0};
    if (io_snprintf(path, 512, "%s/%s", config->svdir, name) == -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    if (unlink(path) == -1) {
        set_last_error("unlink failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int
svc_control(cfg *config, char const *name, char command)
{
    char path[512] = {0};
    if (io_snprintf(
            path, 512, "%s/%s/supervise/control", config->svdir, name) == -1) {
        set_last_error("io_snprintf failed");
        return -1;
    }

    if (io_writeat(-1, path, (char[]){command}, 1) == -1) {
        wrap_last_error("failed to write supervise/control of %s", name);
        return -1;
    }

    return 0;
}

int
svc_running(cfg *config, char const *name)
{
    char path[512] = {0};
    if (io_snprintf(path, 512, "%s/%s/supervise/stat", config->svdir, name) ==
        -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    char buf[3 + 1] = {0}; // "run"
    if (io_readat(-1, path, buf, 3) == -1) {
        wrap_last_error("failed to read supervise/stat of %s", name);
        return -1;
    };

    return strcmp(buf, "run") == 0 ? 1 : 0;
}

int
svc_is_down(cfg *config, char const *name)
{
    char path[512] = {0};
    if (io_snprintf(path, 512, "%s/%s/down", config->svdir, name) == -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    return io_exists(path);
}

int
svc_down(cfg *config, char const *name)
{
    char path[512] = {0};
    if (io_snprintf(path, 512, "%s/%s/down", config->svdir, name) == -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    int fd = creat(path, 0644);
    if (fd == -1) {
        set_last_error("creat failed: %s", strerror(errno));
        return -1;
    }

    close(fd);
    return 0;
}

int
svc_up(cfg *config, char const *name)
{
    char path[512] = {0};
    if (io_snprintf(path, 512, "%s/%s/down", config->svdir, name) == -1) {
        wrap_last_error("io_snprintf failed");
        return -1;
    }

    if (unlink(path) == -1) {
        set_last_error("unlink failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}
