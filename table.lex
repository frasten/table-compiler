/* Generation of lines preceded by their position number */

%{
#include <stdlib.h>
#include <stdio.h>

typedef union {int ival; char *sval; } Lexval;

#define INT 258
#define STRING 259
#define BOOL 260
#define TABLE 261

#define INTCONST 262
#define STRCONST 263
#define BOOLCONST 264

#define PROGRAM 265
#define END 266
#define IF 267
#define THEN 268
#define ELSE 269
#define WHILE 270
#define DO 271

#define READ 272
#define WRITE 273

#define AND 274
#define OR 275
#define NOT 276

#define ASSIGN '='
#define LT '<'
#define LTE 277
#define GT '>'
#define GTE 278
#define COMPARISON 279 /* == */
#define DIFFER 280 /* != */

#define PLUS '+'
#define MINUS '-'
#define MULTIPLY '*'
#define DIVIDE '/'

#define PROJECT 281
#define SELECT 282
#define EXISTS 283
#define ALL 284
#define JOIN 285
#define UPDATE 286
#define EXTEND 287
#define RENAME 288

#define COMMA ','
#define COLON ':'
#define SEMICOLON ';'
#define SQUARE_OPEN '['
#define SQUARE_CLOSE ']'
#define ROUND_OPEN '('
#define ROUND_CLOSE ')'
#define CURLY_OPEN '{'
#define CURLY_CLOSE '}'

#define ID 289

#define MY_EOF 300
#define ERROR 301



char* newstring(char* str) {
	char* new = malloc(strlen(str) + 1);
	strcpy(new, str);
	return new;
}


Lexval lexval;
int linenumber = 1;

%}
%option	noyywrap

delimiter    [ \t\n]
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
