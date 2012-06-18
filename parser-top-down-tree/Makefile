WFLAGS=-Wall -Wextra
CFLAGS=-g $(WFLAGS)

GCC=$(shell which gcc)

SOURCES=$(shell ls *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=compiler

#V=1

# from Lauri Leukkunen's build system
ifdef V
  Q = 
  P = @printf "" # <- space before hash is important!!!
else
  P = @printf "[%s] $@\n" # <- space before hash is important!!!
  Q = @
endif

all: $(EXECUTABLE)

$(EXECUTABLE): table-lex.o $(OBJECTS)
	$(P)LD
	$(Q)$(GCC) $(LIBS) $(CFLAGS) -o $@ ${OBJECTS}

%.o:: %.c
	$(P)CC
	$(Q)$(GCC) $(CFLAGS) -c -o $@ $<

table-lex.c:
	lex -o table-lex.c --header-file=lex.h table.lex

clean:
	rm -f *.o $(EXECUTABLE) table-lex.c lex.h