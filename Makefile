all: parser-ebnf

table-lex.o: table-lex.c
	gcc table-lex.c -g -c -o table-lex.o

table-lex.c:
	lex -o table-lex.c --header-file=lex.h table.lex

parser-bnf: table-lex.o
	gcc table-lex.o parser-bnf.c -g -o parser-bnf

parser-ebnf: table-lex.o
	gcc table-lex.o parser-ebnf.c -g -o parser-ebnf

clean:
	rm -f table-lex table-lex.c parser-bnf lex.h parser-bnf parser-ebnf