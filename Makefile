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

echo :
	@echo OBJS $(OBJS)
	@echo HEADERS $(HEADERS)
	@echo DAYS $(DAYS)
	@echo ADVENTS $(ADVENTS)

$(OBJS) : $(HEADERS)

$(ADVENTS) : $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f $(ADVENTS)
	find . -name '*.dSYM' | xargs rm -rf

try : $(ADVENTS)
	@for day in $(DAYS); do echo $$day -----; ./$$day/advent $$day/input; done
