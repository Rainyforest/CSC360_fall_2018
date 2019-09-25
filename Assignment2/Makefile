.phony all:
all: acs

acs: acs.c
	gcc acs.c -lreadline -pthread -lhistory -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe