all: table-lex

table-lex: table-lex.c
	gcc table-lex.c -o table-lex

table-lex.c:
	lex -o table-lex.c table.lex

clean:
	rm -f table-lex table-lex.c