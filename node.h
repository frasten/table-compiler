#ifndef __NODE_H
#define __NODE_H


typedef enum {
	T_INTEGER,
	T_STRING,
	T_BOOLEAN,
	T_AND,
	T_OR,
	T_BINARY_OP,
	T_BOOLOP,
	T_INTCONST,
	T_BOOLCONST,
	T_STRCONST,
	T_ID,
	T_NONTERMINAL,
	T_MINUS,
	T_NOT
} Typenode;

typedef enum {
	NPROGRAM,
	NSTAT,
	NDEF_STAT,
	NDEF_LIST,
	NSTAT_LIST,
	NID_LIST,
	NATTR_DECL,
	NATTR_LIST,
	NBOOL_TERM,
	NCOMP_TERM,
	NLOW_TERM,
	NEXPR,
	NFACTOR,
	NJOIN_OP,
	NTYPE,
	NASSIGN_STAT,
	NIF_STAT,
	NWHILE_STAT,
	NREAD_STAT,
	NWRITE_STAT,
	NCONST,
	NUNARY_OP,
	NTUPLE_CONST,
	NTABLE_CONST,
	NSPECIFIER,
	NSIMPLE_CONST,
	NATOMIC_TYPE,
	NTABLE_TYPE,
	NPROJECT_OP,
	NSELECT_OP,
	NEXISTS_OP,
	NALL_OP,
	NEXTEND_OP,
	NUPDATE_OP,
	NRENAME_OP
} Nonterminal;

typedef union {
	int ival;
	char *sval;
} Value;

typedef struct snode {
	Typenode type;
	Value value;
	struct snode *child, *brother;
	char* yytext;
} Node;

typedef Node *Pnode;

#define CHILDREN_TREE_LIMIT 100
unsigned char children[CHILDREN_TREE_LIMIT];


/* Function prototypes */
void print_tree(Pnode p, int level);
void print_node(Pnode p);

Pnode idnode();
Pnode keynode(Typenode keyword);
Pnode intconstnode();
Pnode strconstnode();
Pnode boolconstnode();
Pnode nontermnode(Nonterminal nonterm);


#else
#endif