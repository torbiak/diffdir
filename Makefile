CFLAGS += -Wall -Wextra

build: libdiffdir.a diffdir

libdiffdir.a: diffdir.o
	ar rcs $@ $^

diffdir: main.c libdiffdir.a 
	$(CC) $(CFLAGS) $^

clean:
	rm -f *.o *.a diffdir a_only b_only common

test:
	:  # TODO

run:
	./diffdir -h
