CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -ansi -std=c99 -O3
LDFLAGS = -O3
TARGETS = ungcc

all: $(TARGETS)

ungcc: main.c
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@

clean:
	rm *.o

destroy: clean
	rm -f $(TARGETS)

rebuild: destroy all
