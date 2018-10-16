CC=gcc
DEPS = linkedlist.h
.phony all:
all: pman

pman: pman.c
	gcc -Wall pman.c -lreadline -lhistory -o PMan
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
.PHONY clean:
clean:
	-rm -rf *.o *.exe
