%{
// TODO	EVENTUALI INCLUDES
#include <stdio.h>

#include "lexer.h"
#include "node.h"

#define YYSTYPE Pnode

void yyerror(char* message);

%}
%token PROGRAM END IF THEN ELSE WHILE DO READ WRITE AND OR NOT LTE GTE COMPARISON
%token DIFFER PROJECT SELECT EXISTS ALL JOIN UPDATE EXTEND RENAME INT STRING
%token BOOL TABLE INTCONST STRCONST BOOLCONST ID MY_EOF ERROR
%%
// GRAMMATICA
	gino: ;
%%
void yyerror(char* message) {
	fprintf(stderr, "%s\n", message);
}

int main() {
	return yyparse();
}