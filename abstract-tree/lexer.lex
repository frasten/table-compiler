/* Generation of lines preceded by their position number */
%{
#include <stdlib.h>
#include <stdio.h>
#include "abs_tree.h"
#include "symbols.h"

//#include "node.h"


char* newstring(char* str) {
	char* new = malloc(strlen(str) + 1);
	strcpy(new, str);
	return new;
}

Value lexval;

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

             /* Default action */
.            {return ERROR;}

%%
