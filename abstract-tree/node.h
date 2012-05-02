#ifndef __NODE_H
#define __NODE_H

#include "symbols.h"

typedef enum {
	T_BINARY_OP,
	T_BOOLOP,
	T_INTCONST,
	T_BOOLCONST,
	T_STRCONST,
	T_ID,
	T_NONTERMINAL,
	T_NEG_EXPR,
	T_ATOMIC_TYPE,
	T_LOGIC_EXPR,
	T_COMP_EXPR,
	T_MATH_EXPR,
	T_SELECT_EXPR
} Typenode;

typedef enum {
	NPROGRAM,
	NSTAT,
	NDEF_STAT,
	NDEF_LIST,
	NSTAT_LIST,
	NATTR_DECL,
	NBOOL_TERM,
	NCOMP_TERM,
	NLOW_TERM,
	NEXPR,
	NFACTOR,
	NJOIN_EXPR,
	NTYPE,
	NASSIGN_STAT,
	NIF_STAT,
	NWHILE_STAT,
	NREAD_STAT,
	NWRITE_STAT,
	NCONST,
	NTABLE_INSTANCE,
	NTUPLE_CONST,
	NTABLE_TYPE,
	NPROJECT_EXPR,
	NEXTEND_EXPR,
	NUPDATE_EXPR,
	NRENAME_EXPR,
	NTUPLE_LIST
} Nonterminal;

typedef struct snode {
	Typenode type;
	Value value;
	int line;
	struct snode *child, *brother;
	char* yytext;
} Node;

typedef Node *Pnode;

#define CHILDREN_TREE_LIMIT 100
unsigned char children[CHILDREN_TREE_LIMIT];


/* Function prototypes */
void print_tree(Pnode p, int level);
void print_tree_graphviz(Pnode p);
void print_node_graphviz(Pnode p, Pnode parent);

void graphviz_print_edge(Pnode p, Pnode q);
void graphviz_print_node(Pnode p);

char* get_node_string(Pnode p);

Pnode idnode();
Pnode keynode(Typenode keyword);
Pnode intconstnode();
Pnode strconstnode();
Pnode boolconstnode();
Pnode nontermnode(Nonterminal nonterm);
Pnode pseudotermnode(Typenode type, int value);

#else
#endif