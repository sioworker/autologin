# autologin

A small OpenRC service that logs a user in on tty1 without a password and runs a `/bin/sh` script as them. Made for people that have disk encryption and want autologin.

## Install

```sh
make
doas make install
doas rc-update add autologin default
doas rc-service autologin start
```

This installs:

- `/usr/local/sbin/autologin`
- `/etc/init.d/autologin`
- `/etc/autologin.conf` (only if it doesn't exist yet)

tty1 must be free, so comment out its getty in `/etc/inittab`:

```sh
doas sed -i 's|^tty1::|#tty1::|' /etc/inittab
doas kill -HUP 1
```

The service needs `seatd`.

## Config

`/etc/autologin.conf`:

```sh
user=USER # Set your user
# Possible Values:
#  - yes    After the script exits, it'll be ran again
#  - no     After the script exits, a getty will open
relogin=yes
# Everything after script= is ran with /bin/sh, for example:
script=
exec sh
```

## What it does

- creates `/run/user/<uid>`
- takes tty1 as the controlling tty and switches to it
- drops to the user and their groups
- sets `HOME`, `USER`, `LOGNAME`, `SHELL`, `PATH`, `TERM`, `XDG_RUNTIME_DIR`, `XDG_SEAT`, `XDG_VTNR`
- runs the script as a login shell

Stopping the service kills the whole session. Errors before the tty is taken go to `/var/log/autologin.log`.

## Uninstall

```sh
doas rc-service autologin stop
doas rc-update del autologin default
doas make uninstall
```

`/etc/autologin.conf` is left in place.
