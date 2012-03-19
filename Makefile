all: parser-bnf

table-lex.o: table-lex.c
	gcc table-lex.c -c -o table-lex.o

table-lex.c:
	lex -o table-lex.c --header-file=lex.h table.lex

parser-bnf: table-lex.o
	gcc table-lex.o parser-bnf.c -o parser-bnf

clean:
	rm -f table-lex table-lex.c parser-bnf lex.h