CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -pedantic -ansi -std=c99 -O2 -D_XOPEN_SOURCE=700 -g
LDFLAGS = -O2 -lm -lGL -lGLU -lglut
TARGETS = ungcc

all: $(TARGETS)

ungcc: main.o elf.o expr.o elist.o print.o interface.o graph.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o

destroy: clean
	rm -f $(TARGETS)

rebuild: destroy all
