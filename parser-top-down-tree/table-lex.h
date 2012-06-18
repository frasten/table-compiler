#ifndef __TABLE_LEX_H
#define __TABLE_LEX_H

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

int linenumber;

char* newstring(char* str);

#else
#endif