CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -ansi -std=c99 -O3 -D_XOPEN_SOURCE=700 -g
LDFLAGS = -O3
TARGETS = ungcc

all: $(TARGETS)

ungcc: main.o asm.o toasm.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

destroy: clean
	rm -f $(TARGETS)

rebuild: destroy all
