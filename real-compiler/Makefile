WFLAGS=-Wall -Wextra
CFLAGS=-g $(WFLAGS)

GCC=$(shell which gcc)

SOURCES=$(shell ls *.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=table

#V=1

ifdef V
  Q = 
  P = @printf "" # <- space before hash is important!!!
else
  P = @printf "[%s] $@\n" # <- space before hash is important!!!
  Q = @
endif

all: $(EXECUTABLE)

$(EXECUTABLE): parser.o lexer.o $(OBJECTS)
	$(P)LD
	$(Q)$(GCC) $(LIBS) $(CFLAGS) -o $@ ${OBJECTS}

%.o:: %.c parser.h
	$(P)CC
	$(Q)$(GCC) $(CFLAGS) -c -o $@ $<

parser.h: parser.c

parser.c: parser.y lexer.c
	$(P)BS
	$(Q)bison -vd -o $@ parser.y

lexer.c: lexer.lex
	$(P)LX
	$(Q)flex -o $@ --header-file=lexer.h lexer.lex

clean:
	rm -f *.o $(EXECUTABLE) *.output parser.c lexer.c parser.h lexer.h
