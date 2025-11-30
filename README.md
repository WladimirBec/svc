```
./bld/svc [command] [args]...

    SVC is a small and simple wrapper around sv.

Environments:

    SVDIR: running services directory (default: /var/service/)
    AVDIR: available services directory (default: /etc/sv/)

Commands:

    L, list-availables    list the available services
    s, start [service]    start a service
    S, stop [service]     stop a service
    o, once [service]     start a service once
    d, down [service]     down a service
    u, up [service]       up a service
    l, link [service]     link a service
    r, unlink [service]   unlink a service
    v, view               show the services' statuses
    h, help               show this helper

Signals related commands:

    sig-stop [service]    send a STOP signal to a service
    sig-cont [service]    send a CONT signal to a service
    sig-hup [service]     send a HUP signal to a service
    sig-alrm [service]    send a ALRM signal to a service
    sig-int [service]     send a INT signal to a service
    sig-quit [service]    send a QUIT signal to a service
    sig-usr1 [service]    send a USR1 signal to a service
    sig-usr2 [service]    send a USR2 signal to a service
    sig-term [service]    send a TERM signal to a service
    sig-kill [service]    send a KILL signal to a service

The default command is view.
```

## Why this tool even exists ?

One day when unlinking a service I accidently removed my whole /var/service directory, so from the angryness this tool is born.

## Building

```
git clone https://github.com/WladimirBec/svc.git --depth 1
cd svc
make
make PREFIX=~/.local install # to install it locally
make install # to install it globally (in /usr/bin)
./bld/svc # or to just run it without installing
```

## TODO:

- [ ] Talk about how the envs can be used.
- [ ] Make the README shinny

## Thanks to

- @archlinux.btw who unfortunately passed away too soon, we miss you...
