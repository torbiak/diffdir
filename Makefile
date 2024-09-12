CFLAGS += -std=c99 -Wall -Wextra -O2

PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include
PKGCONFIGDIR = $(PREFIX)/lib/pkgconfig

VERSION = 1.0.0

INSTALL = install

.PHONY: build
build: libdiffdir.a diffdir

.PHONY: debug
debug: CFLAGS += -g -fsanitize=leak -fsanitize=address -fsanitize=undefined
debug: build

libdiffdir.a: diffdir.o
	ar rcs $@ $^

diffdir: main.c libdiffdir.a
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o libdiffdir.a libdiffdir.pc diffdir

.PHONY: test
test: diffdir
	./test.py

.PHONY: run
run:
	./diffdir -h

libdiffdir.pc: Makefile
	VERSION=$(VERSION) PREFIX=$(PREFIX) ./gen-pkg-config >$@

.PHONY: install
install: libdiffdir.pc
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 644 $(LIB_NAME) $(DESTDIR)$(LIBDIR)
	$(INSTALL) -d $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 644 diffdir.h $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 644 libdiffdir.pc $(DESTDIR)$(PKGCONFIGDIR)
