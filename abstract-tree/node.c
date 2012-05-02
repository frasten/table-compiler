#include <stdlib.h>
#include <stdio.h>

#include "node.h"
#include "lexer.h"

#define GRAPHVIZ_FILENAME "tree.dot"

extern Value lexval;
extern int linenumber;
FILE* file_graphviz;



Pnode newnode(Typenode tnode) {
	Pnode p;
	p = (Pnode) malloc(sizeof(Node));
	p->line = linenumber;
	p->type = tnode;
	p->yytext = newstring(yytext);
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
	int i;

	p = newnode(T_STRCONST);

	// Un finto escape delle virgolette
	for (i = 0; ; i++) {
		char c = lexval.sval[i];
		if (c == '\0')
			break;
		if (c == '"') {
			lexval.sval[i] = '`';
		}
	}

	p->value.sval = lexval.sval;
	return(p);
}


Pnode boolconstnode() {
	Pnode p;
	p = newnode(T_BOOLCONST);
	p->value.ival = lexval.ival;
	return(p);
}


Pnode pseudotermnode(Typenode type, int value) {
	Pnode p;
	p = newnode(type);
	p->value.ival = value;
	return p;
}


void print_tree(Pnode p, int level) {
	char connector;
	int i;
	char* msg;

	// Print the lines
	for (i = 0; i < level; i++) {
		if (children[i] == 1) {
			printf("| ");
		}
		else {
			printf("  ");
		}
	}

	if (p->brother != NULL) {
		children[level] = 1;
	}
	else {
		children[level] = 0;
	}

	// Print this node
	if (p->brother == NULL)
		connector = '`';
	else if (children[level] == 1)
		connector = '+';
	else
		connector = '|';

	printf("%c-> ", connector);
	msg = get_node_string(p);
	printf("%s\n", msg);
	free(msg);


	// Children
	if (p->child != NULL) {
		print_tree(p->child, level + 1);
	}

	// Right siblings
	if (p->brother != NULL) {
		print_tree(p->brother, level);
	}

	children[level] = 0;
}


void print_tree_graphviz(Pnode p) {
	file_graphviz = fopen(GRAPHVIZ_FILENAME, "w");
	fprintf(file_graphviz, "%s\n", "graph G {");
	print_node_graphviz(p, NULL);

	fprintf(file_graphviz, "%s\n", "}");
	fclose(file_graphviz);
}


void print_node_graphviz(Pnode p, Pnode parent) {
	if (p->child != NULL) {
		graphviz_print_node(p);
		graphviz_print_node(p->child);
		graphviz_print_edge(p, p->child);

		print_node_graphviz(p->child, p);
	}

	if (p->brother != NULL) {
		// Stesso padre
		graphviz_print_node(p->brother);
		graphviz_print_edge(parent, p->brother);

		print_node_graphviz(p->brother, parent);
	}
}

void graphviz_print_edge(Pnode p, Pnode q) {
	fprintf(file_graphviz, "\"%p\" -- \"%p\";\n", p, q);
}


void graphviz_print_node(Pnode p) {
	char* msg;
	msg = get_node_string(p);
	// TODO: per le stringhe, se voglio stampare le "" non posso.
	fprintf(file_graphviz, "\"%p\" [label=\"%s\"];\n", p, msg);
	free(msg);
}


char* get_node_string(Pnode p) {
	char* msg = (char*) calloc(30, sizeof(char));

	// ID (a)
	switch (p->type) {
		case T_INTCONST:    sprintf(msg, "INT (%d)",         p->value.ival); break;
		case T_STRCONST:    sprintf(msg, "STR (%s)",         p->value.sval); break;
		case T_BOOLCONST:   sprintf(msg, "BOOL (%s)",        p->yytext);     break;
		case T_BOOLOP:      sprintf(msg, "BOOLOP (%s)",      p->yytext);     break;
		case T_BINARY_OP:   sprintf(msg, "%c",               p->value.ival); break;
		case T_ID:          sprintf(msg, "ID (%s)",          p->value.sval); break;
		case T_ATOMIC_TYPE: sprintf(msg, "ATOMIC_TYPE (%s)", p->yytext);     break;
		case T_LOGIC_EXPR:  sprintf(msg, "LOGIC_EXPR (%s)",  p->yytext);     break;
		case T_COMP_EXPR:   sprintf(msg, "COMP_EXPR (%s)",   p->yytext);     break;
		case T_MATH_EXPR:   sprintf(msg, "MATH_EXPR (%s)",   p->yytext);     break;
		case T_NEG_EXPR:    sprintf(msg, "NEG_EXPR (%s)",    p->yytext);     break;
		case T_SELECT_EXPR: sprintf(msg, "SELECT_EXPR (%s)", p->yytext);     break;
		case T_NONTERMINAL:
			switch(p->value.ival) {
				case NPROGRAM:          sprintf(msg, "%s", "PROGRAM");           break;
				case NSTAT:             sprintf(msg, "%s", "STAT");              break;
				case NEXPR:             sprintf(msg, "%s", "EXPR");              break;
				case NDEF_STAT:         sprintf(msg, "%s", "DEF_STAT");          break;
				case NDEF_LIST:         sprintf(msg, "%s", "DEF_LIST");          break;
				case NATTR_DECL:        sprintf(msg, "%s", "ATTR_DECL");         break;
				case NBOOL_TERM:        sprintf(msg, "%s", "BOOL_TERM");         break;
				case NCOMP_TERM:        sprintf(msg, "%s", "COMP_TERM");         break;
				case NLOW_TERM:         sprintf(msg, "%s", "LOW_TERM");          break;
				case NPROJECT_EXPR:     sprintf(msg, "%s", "PROJECT_EXPR");      break;
				case NEXTEND_EXPR:      sprintf(msg, "%s", "EXTEND_EXPR");       break;
				case NUPDATE_EXPR:      sprintf(msg, "%s", "UPDATE_EXPR");       break;
				case NRENAME_EXPR:      sprintf(msg, "%s", "RENAME_EXPR");       break;
				case NFACTOR:           sprintf(msg, "%s", "FACTOR");            break;
				case NJOIN_EXPR:        sprintf(msg, "%s", "JOIN_EXPR");         break;
				case NTYPE:             sprintf(msg, "%s", "TYPE");              break;
				case NCONST:            sprintf(msg, "%s", "CONST");             break;
				case NTABLE_INSTANCE:   sprintf(msg, "%s", "TABLE INSTANCE");    break;
				case NTUPLE_LIST:       sprintf(msg, "%s", "TUPLE LIST");        break;
				case NTUPLE_CONST:      sprintf(msg, "%s", "TUPLE CONST");       break;
				case NASSIGN_STAT:      sprintf(msg, "%s", "ASSIGN_STAT");       break;
				case NWHILE_STAT:       sprintf(msg, "%s", "WHILE_STAT");        break;
				case NIF_STAT:          sprintf(msg, "%s", "IF_STAT");           break;
				case NREAD_STAT:        sprintf(msg, "%s", "READ_STAT");         break;
				case NWRITE_STAT:       sprintf(msg, "%s", "WRITE_STAT");        break;
				case NTABLE_TYPE:       sprintf(msg, "%s", "TABLE_TYPE");        break;
				case NSTAT_LIST:        sprintf(msg, "%s", "STAT_LIST");         break;
				default: sprintf(msg, "%s", "xxxNONTERMINALxxx"); break;
			}
			break;
		default: sprintf(msg, "%s", "xxxNODOxxx"); break;
	}
	return msg;
}
