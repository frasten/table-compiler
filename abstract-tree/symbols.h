#ifndef __SYMBOLS_H
#define __SYMBOLS_H

int linenumber;

char* newstring(char* str);

typedef union {
	int ival;
	char *sval;
} Value;

#else
#endif