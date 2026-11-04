CC       ?= cc
CFLAGS   ?= -O2 -Wall -Wextra
CPPFLAGS += -Iinclude
PREFIX   ?= /usr/local
INIT     ?=
SU       ?=

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
	DESTDIR="$(DESTDIR)" SBINDIR="$(PREFIX)/sbin" INIT="$(INIT)" SU="$(SU)" sh init/install.sh install

uninstall:
	DESTDIR="$(DESTDIR)" SBINDIR="$(PREFIX)/sbin" SU="$(SU)" sh init/install.sh uninstall

clean:
	rm -rf build autologin

-include $(DEP)

.PHONY: install uninstall clean
