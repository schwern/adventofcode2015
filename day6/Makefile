.PHONY = test

CFLAGS = -Wall -g -I../lib
CFLAGS  += `pkg-config --cflags glib-2.0`
LDFLAGS += `pkg-config --libs glib-2.0`

all : advent

advent2 : CFLAGS  += `pkg-config --cflags glib-2.0`
advent2 : LDFLAGS += `pkg-config --libs glib-2.0`
advent2 : ../lib/common.o advent2.c

advent.l.c : advent.l
	flex -o advent.l.c advent.l

advent.y.c : advent.y advent.l.c
	bison -o advent.y.c advent.y

advent : advent.l.c advent.y.c
	$(CC) $(CFLAGS) $(LDFLAGS) advent.l.c advent.y.c ../lib/common.o -o advent

clean :
	rm -f advent.l.* advent.y.* advent advent2
	rm -rf *.dSYM

test : advent
	echo 'turn on 499,499 through 500,500' | ./advent
	echo 'turn off 499,499 through 500,500' | ./advent
	echo 'toggle 499,499 through 500,500' | ./advent
