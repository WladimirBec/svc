```
./bld/svc [command] [args]...

    SVC is a small and simple alternative to sv.

Environments:

    SVDIR: running services directory (default: /var/service/)
    AVDIR: available services directory (default: /etc/sv/)

Commands:

    L, list-availables    list the available services
    s, start [service]    start a service
    S, stop [service]     stop a service
    o, once [service]     start a service once
    R, restart [service]  restart a service
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

## Why not just use `sv` ?

Indeed, `sv` works, but this tool adds some features that are missing from the original `sv`.

First it prints a nice table for a quick view of what's running:
```
$ doas svc
PID   NAME            STATUS   DOWN  TIME    
----  --------------  -------  ----  --------
988   agetty-tty1     running  no    00:46:02
986   agetty-tty2     running  no    00:46:02
984   agetty-tty3     running  no    00:46:02
985   agetty-tty4     running  no    00:46:02
987   agetty-tty5     running  no    00:46:02
990   agetty-tty6     running  no    00:46:02
0     agetty-ttyUSB0  stopped  yes   00:04:11
996   dbus            running  no    00:46:02
1001  iwd             running  no    00:46:02
1000  seatd           running  no    00:46:02
993   udevd           running  no    00:46:02
```

It also shows you what services are available for you to link:
```
$ svc L # or svc list-availables
/etc/sv/acpid
/etc/sv/agetty-console
/etc/sv/agetty-generic
/etc/sv/agetty-hvc0
/etc/sv/agetty-hvsi0
/etc/sv/agetty-serial
/etc/sv/agetty-tty1
/etc/sv/agetty-tty2
/etc/sv/agetty-tty3
/etc/sv/agetty-tty4
/etc/sv/agetty-tty5
/etc/sv/agetty-tty6
/etc/sv/agetty-ttyAMA0
/etc/sv/agetty-ttyS0
/etc/sv/agetty-ttyUSB0
/etc/sv/dbus
/etc/sv/dhcpcd
/etc/sv/dhcpcd-eth0
/etc/sv/sshd
...
```

It simplify the way of enabling/disabling services:
```
$ doas svc l sshd # or doas svc link sshd
linked sshd
$ doas svc r sshd # or doas svc unlink sshd
unlinked sshd
```

It offers a nice workflow to down/up services:
```
$ doas svc d sshd # or doas svc down sshd
downed sshd
$ doas svc u sshd # or doas svc up sshd
upped sshd
```

Another feature of `svc` is the native support for sending signals to your services:
```
$ doas svc sig-hup sshd
sent HUP signal to sshd
```

## What the environment variables do ?

`svc` uses two environment variables to modify its behavior.

The first one is `SVDIR` which has the same utility as when used with `sv`, it specifies where the services directory is located, by default just like with `sv` it's set to `/var/service`.
If you use user-level runit services and you desire to manage them with `svc` you can set `SVDIR` to your services directory, like so: `SVDIR=~/services svc view` .

The second one is `AVDIR` and it specifies where to find a collection of predefined runit services, by default it's set to `/etc/sv`.

## Building

```
git clone https://github.com/WladimirBec/svc.git --depth 1
cd svc
make
make PREFIX=~/.local install # to install it locally
make install # to install it globally (in /usr/bin)
./bld/svc # or to just run it without installing
```

## Thanks to

- @archlinux.btw who unfortunately passed away too soon, we miss you...
- @runit25
- Haris
