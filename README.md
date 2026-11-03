# autologin

A small service (OpenRC, systemd, runit or dinit) that logs a user in on tty1 without a password and runs a `/bin/sh` script as them. Made for people that have disk encryption and want autologin.

## Install

```sh
make
doas make install
```

`make install` detects your init system and installs the matching service from `init/`. If it finds more than one, it asks which to use. To skip the question, pick one yourself:

```sh
doas make install INIT=systemd
```

| Init system | Service file | Enable with |
|-------------|--------------|-------------|
| OpenRC  | `/etc/init.d/autologin` | `rc-update add autologin default` |
| systemd | `/etc/systemd/system/autologin.service` | `systemctl enable autologin` |
| runit   | `/etc/sv/autologin` (`/etc/runit/sv` on Artix) | `ln -s /etc/sv/autologin /var/service` |
| dinit   | `/etc/dinit.d/autologin` | `dinitctl enable autologin` |

It also installs `/usr/local/sbin/autologin` and `/etc/autologin.conf` (the config only if it doesn't exist yet).

tty1 must be free. On systemd the service takes it from `getty@tty1` by itself. On OpenRC with busybox init, comment out its getty in `/etc/inittab`:

```sh
doas sed -i 's|^tty1::|#tty1::|' /etc/inittab
doas kill -HUP 1
```

On runit and dinit, disable the tty1 getty service (`agetty-tty1` / `tty1`).

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

Disable and stop the service with your init system, then:

```sh
doas make uninstall
```

`/etc/autologin.conf` is left in place.
