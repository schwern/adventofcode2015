.PHONY = test

CFLAGS = -Wall -g

all : advent

advent.l.c : advent.l
	flex -o advent.l.c advent.l

advent.y.c : advent.y advent.l.c
	bison -o advent.y.c advent.y

advent : advent.l.c advent.y.c
	$(CC) $(CFLAGS) advent.l.c advent.y.c -o advent

clean :
	rm -f advent.l.* advent.y.* advent
	rm -rf *.dSYM

test : advent
	echo 'toggle 660,55 through 986,197' | ./advent
	echo 'turn off 660,55 through 986,197' | ./advent
