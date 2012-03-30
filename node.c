#include <stdlib.h>
#include <stdio.h>

#include "node.h"

extern Value lexval;


Pnode newnode(Typenode tnode) {
	Pnode p;
	p = (Pnode) malloc(sizeof(Node));
	p->type = tnode;
	p->child = p->brother = NULL;
	return(p);
}

Pnode nontermnode(Nonterminal nonterm) {
	Pnode p;
	p = newnode(T_NONTERMINAL);
	p->value.ival = nonterm;
	return(p);
}

Pnode keynode(Typenode keyword) {
	return(newnode(keyword));
}

Pnode idnode() {
	Pnode p;
	p = newnode(T_ID);
	p->value.sval = lexval.sval;
	return(p);
}

Pnode intconstnode() {
	Pnode p;
	p = newnode(T_INTCONST);
	p->value.ival = lexval.ival;
	return(p);
}

Pnode strconstnode() {
	Pnode p;
	p = newnode(T_STRCONST);
	p->value.sval = lexval.sval;
	return(p);
}

Pnode boolconstnode() {
	Pnode p;
	p = newnode(T_BOOLCONST);
	p->value.ival = lexval.ival;
	return(p);
}

void print_tree(Pnode p, int level) {
	// Print this node
	print_node(p, level);

	// Children
	if (p->child != NULL) {
		print_tree(p->child, level + 1);
	}

	// Right siblings
	if (p->brother != NULL) {
		print_tree(p->brother, level);
	}
}

void print_node(Pnode p, int level) {
	int i;
	char connector;

	for (i = 0; i < level; i++) {
		printf("  ");
	}

	// ID (a)
	connector = (p->brother == NULL ? '`' : '|');
	printf("%c-> ", connector);

	switch (p->type) {
		case T_INTEGER:   printf("INTEGER"); break;
		case T_STRING:    printf("STRING");  break;
		case T_BOOLEAN:   printf("BOOLEAN"); break;
		case T_AND:       printf("AND"); break;
		case T_OR:        printf("OR"); break;
		case T_INTCONST:  printf("INT (%d)",  p->value.ival); break;
		case T_STRCONST:  printf("STR (%s)",  p->value.sval); break;
		case T_BOOLCONST: printf("BOOL (%d)", p->value.ival); break;
		case T_BOOLOP:    printf("BOOLOP (%i)", p->value.ival); break;
		case T_BINARY_OP: printf("%c", p->value.ival); break;
		case T_ID:        printf("ID (%s)",   p->value.sval); break;
		case T_NONTERMINAL:
			switch(p->value.ival) {
				case NPROGRAM: printf("PROGRAM"); break;
				case NSTAT: printf("STAT"); break;
				case NEXPR: printf("EXPR"); break;
				case NDEF_STAT: printf("DEF_STAT"); break;
				case NDEF_LIST: printf("DEF_LIST"); break;
				case NID_LIST: printf("ID_LIST"); break;
				case NATTR_DECL: printf("ATTR_DECL"); break;
				case NBOOL_TERM: printf("BOOL_TERM"); break;
				case NCOMP_TERM: printf("COMP_TERM"); break;
				case NLOW_TERM: printf("LOW_TERM"); break;
				case NUNARY_OP: printf("UNARY_OP"); break;
				case NPROJECT_OP: printf("PROJECT_OP"); break;
				case NSELECT_OP: printf("SELECT_OP"); break;
				case NEXISTS_OP: printf("EXISTS_OP"); break;
				case NALL_OP: printf("ALL_OP"); break;
				case NEXTEND_OP: printf("EXTEND_OP"); break;
				case NUPDATE_OP: printf("UPDATE_OP"); break;
				case NRENAME_OP: printf("RENAME_OP"); break;
				case NFACTOR: printf("FACTOR"); break;
				case NJOIN_OP: printf("JOIN_OP"); break;
				case NDOMAIN: printf("DOMAIN"); break;
				case NCONST: printf("CONST"); break;
				case NSIMPLE_CONST: printf("ATOMIC_CONST"); break;
				case NTABLE_CONST: printf("TABLE CONST"); break;
				case NTUPLE_CONST: printf("TUPLE CONST"); break;
				case NSPECIFIER: printf("SPECIFIER"); break;
				case NASSIGN_STAT: printf("ASSIGN_STAT"); break;
				case NWHILE_STAT: printf("WHILE_STAT"); break;
				case NIF_STAT: printf("IF_STAT"); break;
				case NREAD_STAT: printf("READ_STAT"); break;
				case NWRITE_STAT: printf("WRITE_STAT"); break;
				case NATOMIC_TYPE: printf("ATOMIC_TYPE"); break;
				case NTABLE_TYPE: printf("TABLE_TYPE"); break;
				case NSTAT_LIST: printf("STAT_LIST"); break;
				default: printf("xxxNONTERMINALxxx"); break;
			}
			break;
		default:          printf("xxxNODOxxx"); break;
	}
	printf("\n");
}