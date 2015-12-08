OPTIMIZE = -g
WARNINGS = -Wall
INCLUDE  = -Ilib
CFLAGS = $(OPTIMIZE) $(WARNINGS) $(INCLUDE)
CFLAGS  += `pkg-config --cflags glib-2.0`
LDFLAGS += `pkg-config --libs glib-2.0`
OBJS=$(patsubst %.c, %.o, $(wildcard lib/*.c))
HEADERS=$(wildcard lib/*.h)
DAYS=$(wildcard day*)
ADVENTS=$(addsuffix /advent, $(DAYS))

all : Makefile $(ADVENTS)

force-look :
	true

echo :
	@echo OBJS $(OBJS)
	@echo HEADERS $(HEADERS)
	@echo DAYS $(DAYS)
	@echo ADVENTS $(ADVENTS)

$(OBJS) : $(HEADERS)

$(ADVENTS) : $(OBJS)
day6/advent : force-look
	cd day6; $(MAKE) advent

clean:
	rm -f $(OBJS)
	rm -f $(ADVENTS)
	find . -name '*.dSYM' | xargs rm -rf
	cd day6; $(MAKE) clean

try : $(ADVENTS)
	@for day in $(DAYS); do echo $$day -----; ./$$day/advent $$day/input; done
