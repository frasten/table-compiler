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

$(EXECUTABLE): abs_tree.o lexer.o $(OBJECTS)
	$(P)LD
	$(Q)$(GCC) $(LIBS) $(CFLAGS) -o $@ ${OBJECTS}

%.o:: %.c
	$(P)CC
	$(Q)$(GCC) $(CFLAGS) -c -o $@ $<

abs_tree.c: abs_tree.y lexer.c
	bison -vd -o abs_tree.c abs_tree.y

lexer.c: lexer.lex
	lex -o lexer.c --header-file=lexer.h lexer.lex

clean:
	rm -f *.o $(EXECUTABLE) *.output abs_tree.c lexer.c lexer.h abs_tree.h *.dot *.png