CC       ?= cc
CFLAGS   ?= -O2 -Wall -Wextra
CPPFLAGS += -Iinclude
PREFIX   ?= /usr/local
INIT     ?=

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)
DEP = $(OBJ:.o=.d)

autologin: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

build/%.o: src/%.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c -o $@ $<

build:
	mkdir -p build

install: autologin
	install -Dm755 autologin $(DESTDIR)$(PREFIX)/sbin/autologin
	DESTDIR="$(DESTDIR)" SBINDIR="$(PREFIX)/sbin" INIT="$(INIT)" sh init/install.sh install
	[ -e $(DESTDIR)/etc/autologin.conf ] || install -Dm644 etc/autologin.conf $(DESTDIR)/etc/autologin.conf

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/sbin/autologin
	DESTDIR="$(DESTDIR)" sh init/install.sh uninstall

clean:
	rm -rf build autologin

-include $(DEP)

.PHONY: install uninstall clean
