table: lexer.o parser.o symtab.o print.o sem.o stat.o expr.o gen.o main.o
	cc -g -o table lexer.o parser.o symtab.o print.o sem.o stat.o expr.o gen.o main.o
lexer.o: lexer.c def.h parser.h
	cc -g -c lexer.c
lexer.c: lexer.lex def.h parser.h
	flex -o lexer.c lexer.lex
parser.o: parser.c def.h
	cc -g -c parser.c
parser.h: parser.y def.h
	bison -vd -o parser.c parser.y
symtab.o: symtab.c def.h parser.h
	cc -g -c symtab.c
print.o: print.c def.h parser.h
	cc -g -c print.c
sem.o: sem.c def.h parser.h
	cc -g -c sem.c
stat.o: stat.c def.h parser.h
	cc -g -c stat.c
expr.o: expr.c def.h parser.h
	cc -g -c expr.c
gen.o: gen.c def.h parser.h
	cc -g -c gen.c
main.o: main.c def.h parser.h
	cc -g -c main.c