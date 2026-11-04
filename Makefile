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

build/parse: tests/parse.c $(filter-out build/main.o,$(OBJ)) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

build/records: tests/records.c build/record.o | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

test: build/parse build/records autologin
	sh tests/run.sh build/parse
	sh tests/check.sh ./autologin
	build/records build

install: autologin
	DESTDIR="$(DESTDIR)" SBINDIR="$(PREFIX)/sbin" MANDIR="$(PREFIX)/share/man" INIT="$(INIT)" SU="$(SU)" sh init/install.sh install

uninstall:
	DESTDIR="$(DESTDIR)" SBINDIR="$(PREFIX)/sbin" MANDIR="$(PREFIX)/share/man" SU="$(SU)" sh init/install.sh uninstall

clean:
	rm -rf build autologin

-include $(DEP)

.PHONY: install uninstall clean test
