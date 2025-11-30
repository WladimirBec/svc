/**
 * SPDX-License-Identifier: AGPL-3.0-only
 * Copyright (C) 2025 Wladimir Bec
 */
#include "availables.h"
#include "config.h"
#include "err.h"
#include "service.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

#define UNUSED __attribute__((unused))

#define IMPL_CONTROL_CMD(name, command, error, success)              \
    static int cmd_##name(cfg *config, UNUSED int argc, char **argv) \
    {                                                                \
        if (svc_control(config, argv[2], command) < 0) {             \
            print_last_error(error, argv[2]);                        \
            return 1;                                                \
        }                                                            \
                                                                     \
        printf(success "\n", argv[2]);                               \
        return 0;                                                    \
    }

typedef int (*cmd)(cfg *config, int argc, char **argv);

typedef enum {
    CMD_REQ_SVC              = 1 << 0,
    CMD_REQ_SVC_LINKED       = 1 << 1,
    CMD_REQ_SVC_NOT_LINKED   = 1 << 2,
    CMD_REQ_SVC_RUNNING      = 1 << 3,
    CMD_REQ_SVC_NOT_RUNNING  = 1 << 4,
    CMD_REQ_AVAILABLE_EXISTS = 1 << 5,
    CMD_REQ_SVC_DOWN         = 1 << 6,
    CMD_REQ_SVC_NOT_DOWN     = 1 << 7,
} cmd_req;

static int
nofdigits(int n)
{
    int r = 1;

    if (n >= 10000) {
        r += 4;
        n /= 10000;
    }

    if (n >= 100) {
        r += 2;
        n /= 100;
    }

    if (n >= 10) {
        ++r;
    }

    return r;
}

static int
cmd_list_availables(cfg *config, UNUSED int argc, UNUSED char **argv)
{
    arr_of(char *) list = availables_get(config);
    if (list == NULL) {
        print_last_error("failed to get availables list");
        return 1;
    }

    for (size_t i = 0; i < arr_len(list); ++i) {
        printf("%s/%s\n", config->available, list[i]);
    }

    arr_free_free((arr_ptr)list, free);
    return 0;
}

IMPL_CONTROL_CMD(start, 'u', "failed to start %s", "started %s")
IMPL_CONTROL_CMD(stop, 'd', "failed to stop %s", "stopped %s")
IMPL_CONTROL_CMD(once, 'o', "failed to start once %s", "started once %s")

static int
cmd_down(cfg *config, UNUSED int argc, char **argv)
{
    if (svc_down(config, argv[2]) == -1) {
        print_last_error("failed to down %s", argv[2]);
        return 1;
    }

    printf("downed %s\n", argv[2]);
    return 0;
}

static int
cmd_up(cfg *config, UNUSED int argc, char **argv)
{
    if (svc_up(config, argv[2]) == -1) {
        print_last_error("failed to up %s", argv[2]);
        return 1;
    }

    printf("upped %s\n", argv[2]);
    return 0;
}

IMPL_CONTROL_CMD(sig_stop,
                 'p',
                 "failed to send STOP signal to %s",
                 "sent STOP signal to %s")
IMPL_CONTROL_CMD(sig_cont,
                 'c',
                 "failed to send CONT signal to %s",
                 "sent CONT signal to %s")
IMPL_CONTROL_CMD(sig_hup,
                 'h',
                 "failed to send HUP signal to %s",
                 "sent HUP signal to %s")
IMPL_CONTROL_CMD(sig_alrm,
                 'a',
                 "failed to send ALRM signal to %s",
                 "sent ALRM signal to %s")
IMPL_CONTROL_CMD(sig_int,
                 'i',
                 "failed to send INT signal to %s",
                 "sent INT signal to %s")
IMPL_CONTROL_CMD(sig_quit,
                 'q',
                 "failed to send QUIT signal to %s",
                 "sent QUIT signal to %s")
IMPL_CONTROL_CMD(sig_usr1,
                 '1',
                 "failed to send USR1 signal to %s",
                 "sent USR1 signal to %s")
IMPL_CONTROL_CMD(sig_usr2,
                 '2',
                 "failed to send USR2 signal to %s",
                 "sent USR2 signal to %s")
IMPL_CONTROL_CMD(sig_term,
                 't',
                 "failed to send TERM signal to %s",
                 "sent TERM signal to %s")
IMPL_CONTROL_CMD(sig_kill,
                 'k',
                 "failed to send KILL signal to %s",
                 "sent KILL signal to %s")

static int
cmd_link(cfg *config, UNUSED int argc, char **argv)
{
    if (svc_link(config, argv[2]) == -1) {
        print_last_error("failed to link %s", argv[2]);
        return 1;
    }

    printf("linked %s\n", argv[2]);
    return 0;
}

static int
cmd_unlink(cfg *config, UNUSED int argc, char **argv)
{
    if (svc_unlink(config, argv[2]) == -1) {
        print_last_error("failed to unlink %s", argv[2]);
        return 1;
    }

    printf("unlinked %s\n", argv[2]);
    return 0;
}

static int
cmd_view(cfg *config, UNUSED int argc, UNUSED char **argv)
{
    arr_of(svc *) list = svc_list(config);
    if (list == NULL) {
        print_last_error("failed to get services list");
        return 1;
    }

    int widths[5] = {0, 0, 0, 4, 0};
    char *cols[5] = {"PID", "NAME", "STATUS", "DOWN", "TIME"};
    for (size_t i = 0; i < arr_len(list); ++i) {
        svc *service = list[i];

        int n = 0;
        if ((n = nofdigits(service->pid)) > widths[0]) {
            widths[0] = n;
        }
        if ((n = strlen(service->name)) > widths[1]) {
            widths[1] = n;
        }
        if ((n = strlen(svc_status_str(service->status))) > widths[2]) {
            widths[2] = n;
        }
        if ((n = strlen(service->time)) > widths[4]) {
            widths[4] = n;
        }
    }

    printf("%-*s  %-*s  %-*s  %-*s  %-*s\n",
           widths[0],
           cols[0],
           widths[1],
           cols[1],
           widths[2],
           cols[2],
           widths[3],
           cols[3],
           widths[4],
           cols[4]);

    for (size_t i = 0; i < 5; ++i) {
        if (i > 0) {
            printf("  ");
        }
        for (int j = 0; j < widths[i]; ++j) {
            putc('-', stdout);
        }
    }
    fputc('\n', stdout);

    for (size_t i = 0; i < arr_len(list); ++i) {
        svc *service = list[i];
        printf("%-*d  %-*s  %-*s  %-*s  %-*s\n",
               widths[0],
               service->pid,
               widths[1],
               service->name,
               widths[2],
               svc_status_str(service->status),
               widths[3],
               service->is_down == 1 ? "yes" : "no",
               widths[4],
               service->time);
    }

    arr_free_free((arr_ptr)list, free);
    return 0;
}

static int
cmd_help(UNUSED cfg *config, UNUSED int argc, char **argv)
{
    printf("%s [command] [args]...\n\n", argv[0]);
    puts("    SVC is a small and simple wrapper around sv.\n");
    puts("Environments:\n");
    puts("    SVDIR: running services directory (default: /var/service/)");
    puts("    AVDIR: available services directory (default: /etc/sv/)\n");
    puts("Commands:\n");
    puts("    L, list-availables    list the available services");
    puts("    s, start [service]    start a service");
    puts("    S, stop [service]     stop a service");
    puts("    o, once [service]     start a service once");
    puts("    d, down [service]     down a service");
    puts("    u, up [service]       up a service");
    puts("    l, link [service]     link a service");
    puts("    r, unlink [service]   unlink a service");
    puts("    v, view               show the services' statuses");
    puts("    h, help               show this helper\n");
    puts("Signals related commands:\n");
    puts("    sig-stop [service]    send a STOP signal to a service");
    puts("    sig-cont [service]    send a CONT signal to a service");
    puts("    sig-hup [service]     send a HUP signal to a service");
    puts("    sig-alrm [service]    send a ALRM signal to a service");
    puts("    sig-int [service]     send a INT signal to a service");
    puts("    sig-quit [service]    send a QUIT signal to a service");
    puts("    sig-usr1 [service]    send a USR1 signal to a service");
    puts("    sig-usr2 [service]    send a USR2 signal to a service");
    puts("    sig-term [service]    send a TERM signal to a service");
    puts("    sig-kill [service]    send a KILL signal to a service\n");
    puts("The default command is view.");
    return 0;
}

static cmd
find_cmd(int argc, char **argv)
{
    if (argc < 2) {
        return cmd_view;
    }

    char const *cmd = argv[1];
    if (strlen(cmd) == 1) {
        switch (cmd[0]) {
        case 'L': return cmd_list_availables;
        case 's': return cmd_start;
        case 'S': return cmd_stop;
        case 'o': return cmd_once;
        case 'd': return cmd_down;
        case 'u': return cmd_up;
        case 'l': return cmd_link;
        case 'r': return cmd_unlink;
        case 'v': return cmd_view;
        case 'h': return cmd_help;
        }
    } else if (strcasecmp(cmd, "list-availables") == 0) {
        return cmd_list_availables;
    } else if (strcasecmp(cmd, "start") == 0) {
        return cmd_start;
    } else if (strcasecmp(cmd, "stop") == 0) {
        return cmd_stop;
    } else if (strcasecmp(cmd, "once") == 0) {
        return cmd_once;
    } else if (strcasecmp(cmd, "down") == 0) {
        return cmd_down;
    } else if (strcasecmp(cmd, "up") == 0) {
        return cmd_up;
    } else if (strcasecmp(cmd, "link") == 0) {
        return cmd_link;
    } else if (strcasecmp(cmd, "unlink") == 0) {
        return cmd_unlink;
    } else if (strcasecmp(cmd, "view") == 0) {
        return cmd_help;
    } else if (strcasecmp(cmd, "help") == 0) {
        return cmd_help;
    } else if (strcasecmp(cmd, "sig-stop") == 0) {
        return cmd_sig_stop;
    } else if (strcasecmp(cmd, "sig-cont") == 0) {
        return cmd_sig_cont;
    } else if (strcasecmp(cmd, "sig-hup") == 0) {
        return cmd_sig_hup;
    } else if (strcasecmp(cmd, "sig-alrm") == 0) {
        return cmd_sig_alrm;
    } else if (strcasecmp(cmd, "sig-int") == 0) {
        return cmd_sig_int;
    } else if (strcasecmp(cmd, "sig-quit") == 0) {
        return cmd_sig_quit;
    } else if (strcasecmp(cmd, "sig-usr1") == 0) {
        return cmd_sig_usr1;
    } else if (strcasecmp(cmd, "sig-usr2") == 0) {
        return cmd_sig_usr2;
    } else if (strcasecmp(cmd, "sig-term") == 0) {
        return cmd_sig_term;
    } else if (strcasecmp(cmd, "sig-kill") == 0) {
        return cmd_sig_kill;
    }

    print_last_error("unknown command %s", cmd);
    return NULL;
}

static int
do_requirements(cmd_req reqs, cfg *config, int argc, char **argv)
{
    if (reqs & CMD_REQ_SVC && argc < 3) {
        print_last_error("[service] expected");
        return -1;
    }

    if (reqs & CMD_REQ_SVC_LINKED || reqs & CMD_REQ_SVC_NOT_LINKED) {
        assert(reqs & CMD_REQ_SVC);

        int r = svc_linked(config, argv[2]);
        if (r == -1) {
            print_last_error("failed to check service %s", argv[2]);
            return -1;
        }

        if (reqs & CMD_REQ_SVC_LINKED && r == 0) {
            print_last_error("service %s is already not linked", argv[2]);
            return -1;
        }

        if (reqs & CMD_REQ_SVC_NOT_LINKED && r == 1) {
            print_last_error("service %s is already linked", argv[2]);
            return -1;
        }
    }

    if (reqs & CMD_REQ_SVC_RUNNING || reqs & CMD_REQ_SVC_NOT_RUNNING) {
        assert(reqs & CMD_REQ_SVC);

        int r = svc_running(config, argv[2]);
        if (r == -1) {
            print_last_error("failed to get service %s status", argv[2]);
            return -1;
        }

        if (reqs & CMD_REQ_SVC_RUNNING && r == 0) {
            print_last_error("service %s is already not running", argv[2]);
            return -1;
        }

        if (reqs & CMD_REQ_SVC_NOT_RUNNING && r == 1) {
            print_last_error("service %s is already running", argv[2]);
            return -1;
        }
    }

    if (reqs & CMD_REQ_AVAILABLE_EXISTS) {
        assert(reqs & CMD_REQ_SVC);

        int r = availables_exist(config, argv[2]);
        if (r == -1) {
            print_last_error("cannot check if service %s exists", argv[2]);
            return -1;
        } else if (r == 0) {
            print_last_error("service %s doesn't exist", argv[2]);
            return -1;
        }
    }

    if (reqs & CMD_REQ_SVC_DOWN || reqs & CMD_REQ_SVC_NOT_DOWN) {
        assert(reqs & CMD_REQ_SVC);

        int r = svc_is_down(config, argv[2]);
        if (r == -1) {
            print_last_error("failed to get service %s downess", argv[2]);
            return -1;
        }

        if (reqs & CMD_REQ_SVC_DOWN && r == 0) {
            print_last_error("service %s is already up", argv[2]);
            return -1;
        }

        if (reqs & CMD_REQ_SVC_NOT_DOWN && r == 1) {
            print_last_error("service %s is already down", argv[2]);
            return -1;
        }
    }

    return 0;
}

int
main(int argc, char **argv)
{
    cmd c = find_cmd(argc, argv);
    if (c == NULL) {
        return 1;
    }

    cmd_req reqs = 0;
    if (c == cmd_list_availables) {
        reqs = 0;
    } else if (c == cmd_start) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_NOT_RUNNING;
    } else if (c == cmd_stop) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_once) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_NOT_RUNNING;
    } else if (c == cmd_down) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_NOT_DOWN;
    } else if (c == cmd_up) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_DOWN;
    } else if (c == cmd_link) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_NOT_LINKED;
    } else if (c == cmd_unlink) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED;
    } else if (c == cmd_view) {
        reqs = 0;
    } else if (c == cmd_help) {
        reqs = 0;
    } else if (c == cmd_sig_stop) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_cont) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_hup) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_alrm) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_int) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_quit) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_usr1) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_usr2) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_term) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    } else if (c == cmd_sig_kill) {
        reqs = CMD_REQ_SVC | CMD_REQ_SVC_LINKED | CMD_REQ_SVC_RUNNING;
    }

    cfg config = cfg_get();
    if (do_requirements(reqs, &config, argc, argv) < 0) {
        return 1;
    }

    return c(&config, argc, argv);
}
