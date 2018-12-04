.phony all:
all: diskops

diskops:
	gcc -Wall diskinfo.c -lm -o  diskinfo
	gcc -Wall disklist.c -lm -o disklist
	gcc -Wall diskget.c -lm -o diskget
	gcc -Wall diskput.c -lm -o diskput

.PHONY clean:
clean:
	-rm -rf *.o *.exe
