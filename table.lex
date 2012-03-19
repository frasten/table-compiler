/* Generation of lines preceded by their position number */

%{
#include <stdlib.h>
#include <stdio.h>
#include "table-lex.h"


char* newstring(char* str) {
	char* new = malloc(strlen(str) + 1);
	strcpy(new, str);
	return new;
}


Lexval lexval;
int linenumber = 1;

%}
%option	noyywrap

delimiter    [ \t]
spacing      {delimiter}+
letter       [A-Za-z]
digit        [0-9]
intconst     {digit}+
strconst     \"([^\"])*\"
/*"/* per il mio editor */
boolconst    true|false
id           {letter}({letter}|{digit})*
newline      \n
sugar        [=<>+/\-\*,:;\[\]\(\)\{\}]

%%

{newline}    {linenumber++;}
{spacing}    ;
program      {return PROGRAM;}
end          {return END;}
if           {return IF;}
then         {return THEN;}
else         {return ELSE;}
while        {return WHILE;}
do           {return DO;}
read         {return READ;}
write        {return WRITE;}
and          {return AND;}
or           {return OR;}
not          {return NOT;}

"<="         {return LTE;}
">="         {return GTE;}
"=="         {return COMPARISON;}
"!="         {return DIFFER;}

{sugar}      {return yytext[0];}

project      {return PROJECT;}
select       {return SELECT;}
exists       {return EXISTS;}
all          {return ALL;}
join         {return JOIN;}
update       {return UPDATE;}
extend       {return EXTEND;}
rename       {return RENAME;}


integer      {return INT;}
string       {return STRING;}
boolean      {return BOOL;}
table	     {return TABLE;}
{intconst}   {lexval.ival = atoi(yytext); return INTCONST;}
{strconst}   {lexval.sval = newstring(yytext); return STRCONST;}
{boolconst}  {lexval.ival = (yytext[0] == 'f' ? 0 : 1); return BOOLCONST;}

{id}         {lexval.sval = newstring(yytext); return ID;}

             /* End of file */
<<EOF>>      {return MY_EOF;}

             /* Default action */
.            {return ERROR;}

%%


main()
{
	printf("##################################\n");
	int result;
	while ((result = yylex()) != MY_EOF) {
		if (result == ERROR) {
			printf("Line %d: %s <-- ERROR!!!\n", linenumber, yytext);
		}
		else {
			printf("Line %d: %d (%s)\n", linenumber, result, yytext);
		}
	}
	printf("Fine.\n");
}
