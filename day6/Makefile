.PHONY = test

CFLAGS = -Wall -g -I../lib

all : advent

advent.l.c : advent.l
	flex -o advent.l.c advent.l

advent.y.c : advent.y advent.l.c
	bison -o advent.y.c advent.y

advent : advent.l.c advent.y.c
	$(CC) $(CFLAGS) advent.l.c advent.y.c ../lib/file.o -o advent

clean :
	rm -f advent.l.* advent.y.* advent
	rm -rf *.dSYM

test : advent
	echo 'turn on 499,499 through 500,500' | ./advent
	echo 'turn off 499,499 through 500,500' | ./advent
	echo 'toggle 499,499 through 500,500' | ./advent
