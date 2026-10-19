CC       ?= cc
CFLAGS   ?= -O2 -Wall -Wextra
CPPFLAGS += -Iinclude
PREFIX   ?= /usr/local

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
	install -Dm755 etc/init.d/autologin $(DESTDIR)/etc/init.d/autologin
	[ -e $(DESTDIR)/etc/autologin.conf ] || install -Dm644 etc/autologin.conf $(DESTDIR)/etc/autologin.conf

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/sbin/autologin $(DESTDIR)/etc/init.d/autologin

clean:
	rm -rf build autologin

-include $(DEP)

.PHONY: install uninstall clean
