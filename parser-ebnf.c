#include <stdio.h>

#include "node.h"
#include "parser-ebnf.h"
#include "table-lex.h"
#include "lex.h"

int lookahead;
Pnode root = NULL;

void debug(const char* str) {
	printf("%s", str);
	printf("yytext = %s\n", yytext);
	printf("lookahead = %d\n", lookahead);
}

void whereami(const char* funcname) {
	return;
	const char* pattern = "Entering %s()\n";
	char* tmp = malloc(strlen(funcname) + strlen(pattern) - 2);
	sprintf(tmp, pattern, funcname);
	debug(tmp);
	free(tmp);
}

int main() {
	int i;
	// Reset tree data
	for (i = 0; i < CHILDREN_TREE_LIMIT; i++) {
		children[i] = 0;
	}

	printf("\nCompiling, please wait..\n");
	linenumber = 1;
	errors = 0;
	parse();

	print_tree(root, 0);

	if (lookahead != MY_EOF) {
		printf("Line %d: Syntax error. Found garbage data at the end of the file.\n", linenumber);
		errors++;
	}

	if (errors == 0) {
		printf("Compilation successful.\n\n");
		return 0;
	}
	else {
		printf("Compilation unsuccessful, %i errors.\n\n", errors);
		return 1;
	}
}

void parse() {
	next();
	root = nontermnode(NPROGRAM);
	root->child = parse_program();
}

void next() {
	lookahead = yylex();
	if (lookahead == ERROR) {
		printf("Line %d: %s <-- ERROR!!!\n", linenumber, yytext);
		errors++;
	}
	//printf("Line %d: %d (%s)\n", linenumber, result, yytext);
}

void parseerror() {
	errors++;
	printf("Syntax error @line %d: %s\n", linenumber, yytext);
	next();
}

void match(int what) {
	if (lookahead == what) {
		next();
	}
	else {
		printf("Syntax error @line %d: %s. Expected %d - %c\n", linenumber, yytext, what, what);
		errors++;
	}
}

Pnode parse_program() {
	Pnode p;
	whereami(__func__);
	match(PROGRAM);
	p = nontermnode(NSTAT_LIST);
	p->child = parse_stat_list();
	match(END);
	return p;
}

Pnode parse_stat_list() {
	Pnode head, p;
	whereami(__func__);
	head = p = nontermnode(NSTAT);
	head->child = parse_stat();
	while (lookahead == ';') {
		next();
		p->brother = nontermnode(NSTAT);
		p->brother->child = parse_stat();
		p = p->brother;
	}
	return head;
}

Pnode parse_stat() {
	Pnode p = NULL;
	whereami(__func__);
	switch (lookahead) {
		case INT:
		case STRING:
		case BOOL:
		case TABLE:
			/* def-stat */
			p = nontermnode(NDEF_STAT);
			p->child = parse_def_stat();
			break;
		case ID:
			p = nontermnode(NASSIGN_STAT);
			p->child = parse_assign_stat();
			break;
		case IF:
			p = nontermnode(NIF_STAT);
			p->child = parse_if_stat();
			break;
		case WHILE:
			p = nontermnode(NWHILE_STAT);
			p->child = parse_while_stat();
			break;
		case READ:
			p = nontermnode(NREAD_STAT);
			p->child = parse_read_stat();
			break;
		case WRITE:
			p = nontermnode(NWRITE_STAT);
			p->child = parse_write_stat();
			break;
		default:
			parseerror();
	}
	return p;
}

Pnode parse_def_stat() {
	Pnode head;
	// type -> idlist
	whereami(__func__);
	head = nontermnode(NTYPE);
	head->child = parse_type();
	head->brother = nontermnode(NID_LIST);
	head->brother->child = parse_id_list();
	return head;
}

Pnode parse_id_list() {
	Pnode head, p;
	/* id_list
	     |
	     id->id->id
	*/
	whereami(__func__);
	match(ID);
	head = p = idnode();
	while (lookahead == ',') {
		next();
		match(ID);
		p->brother = idnode();
		p = p->brother;
	}
	return head;
}

Pnode parse_type() {
	// If atomic: terminal node representing the key
	// Otherwise: a NTABLE_TYPE
	//               `->attr ->attr ->attr...
	Pnode p = NULL;
	whereami(__func__);
	switch (lookahead) {
		case INT:
		case STRING:
		case BOOL:	
			p = nontermnode(NATOMIC_TYPE);
			p->child = parse_atomic_type();
			break;
		case TABLE:
			p = nontermnode(NTABLE_TYPE);
			p->child = parse_table_type();
			break;
		default:
			parseerror();
	}
	return p;
}

Pnode parse_atomic_type() {
	// terminal node representing the key
	Pnode p = NULL;
	whereami(__func__);
	switch (lookahead) {
		case INT:
			next();
			p = keynode(T_INTEGER);
			break;
		case STRING:
			next();
			p = keynode(T_STRING);
			break;
		case BOOL:
			next();
			p = keynode(T_BOOLEAN);
			break;
		default:
			parseerror();
	}
	return p;
}

Pnode parse_table_type() {
	// An attributes list
	Pnode p;
	whereami(__func__);
	match(TABLE);
	match('(');
	p = nontermnode(NATTR_LIST);
	p->child = parse_attr_list();
	match(')');
	return p;
}

Pnode parse_attr_list() {
	Pnode head, p;
	whereami(__func__);
	head = p = nontermnode(NATTR_DECL);
	head->child = parse_attr_decl();
	while (lookahead == ',') {
		next();
		p->brother = nontermnode(NATTR_DECL);
		p->brother->child = parse_attr_decl();
		p = p->brother;
	}
	return head;
}

Pnode parse_attr_decl() {
	Pnode head;
	// type id
	whereami(__func__);
	head = nontermnode(NATOMIC_TYPE);
	head->child = parse_atomic_type();
	match(ID);
	head->brother = idnode();
	return head;
}

Pnode parse_assign_stat() {
	Pnode p;
	whereami(__func__);
	match(ID);
	p = idnode();
	match('=');
	p->brother = nontermnode(NEXPR);
	p->brother->child = parse_expr();
	return p;
}

Pnode parse_expr() {
	Pnode head, p;
	whereami(__func__);
	head = p = nontermnode(NBOOL_TERM);
	head->child = parse_bool_term();
	while (lookahead == AND || lookahead == OR) {
		next();
		// TODO: BUG ho un and, e invece mi appare un nodo OR.
		p->brother = keynode(lookahead == AND ? T_AND : T_OR);
		p = p->brother;
		p->brother = nontermnode(NBOOL_TERM);
		p->brother->child = parse_bool_term();
		p = p->brother;
	}
	return head;
}

Pnode parse_bool_term() {
	Pnode head, p;
	whereami(__func__);
	head = p = nontermnode(NCOMP_TERM);
	p->child = parse_comp_term();
	if (lookahead == COMPARISON ||
		lookahead == DIFFER ||
		lookahead == GT ||
		lookahead == GTE ||
		lookahead == LT ||
		lookahead == LTE ||
		lookahead == GT) {
		p->brother = keynode(T_BOOLOP);
		next();
		// TODO: probabilmente bug.
		p = p->brother;
		p->value.ival = lookahead;
		p->brother = nontermnode(NCOMP_TERM);
		p = p->brother;
		p->child = parse_comp_term();
	}
	return head;
}

Pnode parse_comp_term() {
	Pnode head, p;
	whereami(__func__);
	head = p = nontermnode(NLOW_TERM);
	head->child = parse_low_term();
	while (lookahead == PLUS || lookahead == MINUS) {
		// TODO: non appare nell'albero, e' vuoto.
		next();
		p->brother = keynode(T_BINARY_OP);
		p = p->brother;
		p->value.ival = lookahead;
		p->brother = nontermnode(NLOW_TERM);
		p = p->brother;
		p->child = parse_low_term();
	}
	return head;
}

Pnode parse_low_term() {
	Pnode head, p;
	whereami(__func__);
	head = p = nontermnode(NFACTOR);
	head->child = parse_factor();
	while (lookahead == MULTIPLY ||
		lookahead == DIVIDE ||
		lookahead == JOIN) {
		if (lookahead == JOIN) {
			p->brother = nontermnode(NJOIN_OP);
			p = p->brother;
			p->child = parse_join_op();
		}
		else {
			p->brother = keynode(T_BINARY_OP);
			p = p->brother;
			p->value.ival = lookahead;
			next();
		}
		p->brother = nontermnode(NFACTOR);
		p = p->brother;
		p->child = parse_factor();
	}
	return head;
}

Pnode parse_factor() {
	Pnode head, p;
	whereami(__func__);
	switch (lookahead) {
		case ID:
			head = p = idnode();
			next();
			break;
		case '(':
			next();
			head = p = nontermnode(NEXPR);
			p->child = parse_expr();
			match(')');
			break;
		case INTCONST:
		case STRCONST:
		case BOOLCONST:
		case '{':
			head = p = nontermnode(NCONST);
			p->child = parse_const();
			break;
		default:
			head = p = nontermnode(NUNARY_OP);
			p->child = parse_unary_op();
			p->brother = nontermnode(NFACTOR);
			p = p->brother;
			p->child = parse_factor();
	}
	return head;
}

Pnode parse_unary_op() {
	Pnode p = NULL;
	whereami(__func__);
	switch (lookahead) {
		case MINUS:
		case NOT:
			// TODO
			next();
			break;
		case PROJECT:
			p = nontermnode(NPROJECT_OP);
			p->child = parse_project_op();
			break;
		case SELECT:
			p = nontermnode(NSELECT_OP);
			p->child = parse_select_op();
			break;
		case EXISTS:
			p = nontermnode(NEXISTS_OP);
			p->child = parse_exists_op();
			break;
		case ALL:
			p = nontermnode(NALL_OP);
			p->child = parse_all_op();
			break;
		case EXTEND:
			p = nontermnode(NEXTEND_OP);
			p->child = parse_extend_op();
			break;
		case UPDATE:
			p = nontermnode(NUPDATE_OP);
			p->child = parse_update_op();
			break;
		case RENAME:
			p = nontermnode(NRENAME_OP);
			p->child = parse_rename_op();
			break;
		default:
			parseerror();
	}
	return p;
}

Pnode parse_join_op() {
	Pnode p;
	whereami(__func__);
	match(JOIN);
	match('[');
	p = nontermnode(NEXPR);
	p->child = parse_expr();
	match(']');
	return p;
}

Pnode parse_project_op() {
	Pnode p;
	whereami(__func__);
	match(PROJECT);
	match('[');
	p = nontermnode(NID_LIST);
	p->child = parse_id_list();
	match(']');
	return p;
}

Pnode parse_select_op() {
	Pnode p;
	whereami(__func__);
	match(SELECT);
	match('[');
	p = nontermnode(NEXPR);
	p->child = parse_expr();
	match(']');
	return p;
}

Pnode parse_exists_op() {
	Pnode p;
	whereami(__func__);
	match(EXISTS);
	match('[');
	p = nontermnode(NEXPR);
	p->child = parse_expr();
	match(']');
	return p;
}

Pnode parse_all_op() {
	Pnode p;
	whereami(__func__);
	match(ALL);
	match('[');
	p = nontermnode(NEXPR);
	p->child = parse_expr();
	match(']');
	return p;
}

Pnode parse_extend_op() {
	Pnode p;
	whereami(__func__);
	match(EXTEND);
	match('[');
	p = nontermnode(NATOMIC_TYPE);
	p->child = parse_atomic_type();
	match(ID);
	match('=');
	p->brother = nontermnode(NEXPR);
	p->brother->child = parse_expr();
	match(']');
	return p;
}

Pnode parse_update_op() {
	Pnode p;
	whereami(__func__);
	match(UPDATE);
	match('[');
	p = idnode();
	match(ID);
	match('=');
	p->brother = nontermnode(NEXPR);
	p->brother->child = parse_expr();
	match(']');
	return p;
}

Pnode parse_rename_op() {
	Pnode p;
	whereami(__func__);
	match(RENAME);
	match('[');
	p = nontermnode(NID_LIST);
	p->child = parse_id_list();
	match(']');
	return p;
}

Pnode parse_const() {
	Pnode p = NULL;
	whereami(__func__);
	switch (lookahead) {
		case INTCONST:
		case STRCONST:
		case BOOLCONST:
			p = nontermnode(NSIMPLE_CONST);
			p->child = parse_atomic_const();
			break;
		case '{':
			p = nontermnode(NTABLE_CONST);
			p->child = parse_table_const();
			break;
		default:
			parseerror();
	}
	return p;
}

Pnode parse_atomic_const() {
	Pnode p = NULL;
	whereami(__func__);
	switch (lookahead) {
		case INTCONST:
			p = intconstnode();
			next();
			break;
		case STRCONST:
			p = strconstnode();
			next();
			break;
		case BOOLCONST:
			p = boolconstnode();
			next();
			break;
		default:
			parseerror();
	}
	return p;
}

Pnode parse_table_const() {
	Pnode p, q, head;
	whereami(__func__);
	match('{');
	p = head = NULL;
	// TODO: Potrei iniziare con un "{,}", che non va bene!!!!!
	while (lookahead == '(' || lookahead == ',') {
		if (lookahead == ',') {
			next();
			continue;
		}
		q = nontermnode(NTUPLE_CONST);
		if (p == NULL) {
			head = p = q;
		}
		else {
			p->brother = q;
			p = p->brother;
		}
		p->child = parse_tuple_const();
	}
	match('}');
	return head;
}

Pnode parse_tuple_const() {
	Pnode p, head;
	whereami(__func__);
	match('(');
	head = p = nontermnode(NSIMPLE_CONST);
	p->child = parse_atomic_const();
	while (lookahead == ',') {
		next();
		p->brother = nontermnode(NSIMPLE_CONST);
		p = p->brother;
		p->child = parse_atomic_const();
	}
	match(')');
	return head;
}

Pnode parse_if_stat() {
	Pnode p, head;
	whereami(__func__);
	match(IF);
	head = p = nontermnode(NEXPR);
	p->child = parse_expr();
	match(THEN);
	p->brother = nontermnode(NSTAT_LIST);
	p = p->brother;
	p->child = parse_stat_list();
	if (lookahead == ELSE) {
		next();
		p->brother = nontermnode(NSTAT_LIST);
		p = p->brother;
		p->child = parse_stat_list();
	}
	match(END);
	return head;
}

Pnode parse_while_stat() {
	Pnode p;
	whereami(__func__);
	match(WHILE);
	p = nontermnode(NEXPR);
	p->child = parse_expr();
	match(DO);
	p->brother = nontermnode(NSTAT_LIST);
	p->brother->child = parse_stat_list();
	match(END);
	return p;
}

Pnode parse_read_stat() {
	Pnode p;
	whereami(__func__);
	match(READ);
	p = nontermnode(NSPECIFIER);
	p->child = parse_specifier();
	match(ID);
	p->brother = idnode();
	return p;
}

Pnode parse_specifier() {
	Pnode p = NULL;
	whereami(__func__);
	if (lookahead == '[') {
		next();
		p = nontermnode(NEXPR);
		p->child = parse_expr();
		match(']');
	}
	return p;
}

Pnode parse_write_stat() {
	Pnode p;
	whereami(__func__);
	match(WRITE);
	p = nontermnode(NSPECIFIER);
	p->child = parse_specifier();
	p->brother = nontermnode(NEXPR);
	p->brother->child = parse_expr();
	return p;
}
