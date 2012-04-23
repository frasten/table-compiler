#include <stdio.h>

#include "parser-ebnf.h"
#include "table-lex.h"
#include "lex.h"

int lookahead;

void debug(const char* str) {
	printf("%s", str);
	printf("yytext = %s\n", yytext);
	printf("lookahead = %d\n", lookahead);
}

void whereami(const char* funcname) {
	const char* pattern = "Entering %s()\n";
	char* tmp = malloc(strlen(funcname) + strlen(pattern) - 2);
	sprintf(tmp, pattern, funcname);
	debug(tmp);
	free(tmp);
}

int main() {
	linenumber = 1;
	parse();
	if (lookahead != MY_EOF) {
		printf("Line %d: Syntax error. Found garbage data at the end of the file.\n", linenumber);
		return 1;
	}
	return 0;
}

void parse() {
	next();
	parse_program();
}

void next() {
	int result = yylex();
	// MY_EOF
	if (result == ERROR) {
		printf("Line %d: %s <-- ERROR!!!\n", linenumber, yytext);
	}
	else {
		//printf("Line %d: %d (%s)\n", linenumber, result, yytext);
		lookahead = result;
	}
}

void parseerror() {
	printf("Syntax error @line %d: %s\n", linenumber, yytext);
	next();
}

void match(int what) {
	if (lookahead == what) {
		next();
	}
	else {
		printf("Syntax error @line %d: %s. Expected %d - %c\n", linenumber, yytext, what, what);
	}
}

void parse_program() {
	whereami(__func__);
	match(PROGRAM);
	parse_stat_list();
	match(END);
}

void parse_stat_list() {
	whereami(__func__);
	parse_stat();
	while (lookahead == ';') {
		next();
		parse_stat();
	}
}

void parse_stat() {
	whereami(__func__);
	switch (lookahead) {
		case INT:
		case STRING:
		case BOOL:
		case TABLE:
			/* def-stat */
			parse_def_stat();
			break;
		case ID:
			parse_assign_stat();
			break;
		case IF:
			parse_if_stat();
			break;
		case WHILE:
			parse_while_stat();
			break;
		case READ:
			parse_read_stat();
			break;
		case WRITE:
			parse_write_stat();
			break;
		default:
			parseerror();
	}
}

void parse_def_stat() {
	whereami(__func__);
	parse_type();
	parse_id_list();
}

void parse_id_list() {
	whereami(__func__);
	match(ID);
	while (lookahead == ',') {
		next();
		match(ID);
	}
}

void parse_type() {
	whereami(__func__);
	switch (lookahead) {
		case INT:
		case STRING:
		case BOOL:
			parse_atomic_type();
			break;
		case TABLE:
			parse_table_type();
			break;
		default:
			parseerror();
	}
}

void parse_atomic_type() {
	whereami(__func__);
	switch (lookahead) {
		case INT:
		case STRING:
		case BOOL:
			next();
			break;
		default:
			parseerror();
	}
}

void parse_table_type() {
	whereami(__func__);
	match(TABLE);
	match('(');
	parse_attr_list();
	match(')');
}

void parse_attr_list() {
	whereami(__func__);
	parse_attr_decl();
	while (lookahead == ',') {
		next();
		parse_attr_decl();
	}
}

void parse_attr_decl() {
	whereami(__func__);
	parse_atomic_type();
	match(ID);
}

void parse_assign_stat() {
	whereami(__func__);
	match(ID);
	match('=');
	parse_expr();
}

void parse_expr() {
	whereami(__func__);
	parse_bool_term();
	while (lookahead == AND || lookahead == OR) {
		next();
		parse_bool_term();
	}
}

void parse_bool_term() {
	whereami(__func__);
	parse_comp_term();
	if (lookahead == COMPARISON ||
		lookahead == DIFFER ||
		lookahead == GT ||
		lookahead == GTE ||
		lookahead == LT ||
		lookahead == LTE ||
		lookahead == GT) {
		next();
		parse_comp_term();
	}
}

void parse_comp_term() {
	whereami(__func__);
	parse_low_term();
	while (lookahead == PLUS || lookahead == MINUS) {
		next();
		parse_low_term();
	}
}

void parse_low_term() {
	whereami(__func__);
	parse_factor();
	while (lookahead == MULTIPLY ||
		lookahead == DIVIDE ||
		lookahead == JOIN) {
		if (lookahead == JOIN) {
			parse_join_op();
		}
		else {
			next();
		}
		parse_factor();
	}
}

void parse_factor() {
	whereami(__func__);
	switch (lookahead) {
		case ID:
			next();
			break;
		case '(':
			next();
			parse_expr();
			match(')');
			break;
		case INTCONST:
		case STRCONST:
		case BOOLCONST:
		case '{':
			parse_const();
			break;
		default:
			parse_unary_op();
			parse_factor();
	}
}

void parse_unary_op() {
	whereami(__func__);
	switch (lookahead) {
		case MINUS:
		case NOT:
			next();
			break;
		case PROJECT:
			parse_project_op();
			break;
		case SELECT:
			parse_select_op();
			break;
		case EXISTS:
			parse_exists_op();
			break;
		case ALL:
			parse_all_op();
			break;
		case EXTEND:
			parse_extend_op();
			break;
		case UPDATE:
			parse_update_op();
			break;
		case RENAME:
			parse_rename_op();
			break;
		default:
			parseerror();
	}
}

void parse_join_op() {
	whereami(__func__);
	match(JOIN);
	match('[');
	parse_expr();
	match(']');
}

void parse_project_op() {
	whereami(__func__);
	match(PROJECT);
	match('[');
	parse_id_list();
	match(']');
}

void parse_select_op() {
	whereami(__func__);
	match(SELECT);
	match('[');
	parse_expr();
	match(']');
}

void parse_exists_op() {
	whereami(__func__);
	match(EXISTS);
	match('[');
	parse_expr();
	match(']');
}

void parse_all_op() {
	whereami(__func__);
	match(ALL);
	match('[');
	parse_expr();
	match(']');
}

void parse_extend_op() {
	whereami(__func__);
	match(EXTEND);
	match('[');
	parse_atomic_type();
	match(ID);
	match('=');
	parse_expr();
	match(']');
}

void parse_update_op() {
	whereami(__func__);
	match(UPDATE);
	match('[');
	match(ID);
	match('=');
	parse_expr();
	match(']');
}

void parse_rename_op() {
	whereami(__func__);
	match(RENAME);
	match('[');
	parse_id_list();
	match(']');
}

void parse_const() {
	whereami(__func__);
	switch (lookahead) {
		case INTCONST:
		case STRCONST:
		case BOOLCONST:
			parse_atomic_const();
			break;
		case '{':
			parse_table_const();
			break;
		default:
			parseerror();
	}
}

void parse_atomic_const() {
	whereami(__func__);
	if (lookahead == INTCONST ||
		lookahead == STRCONST ||
		lookahead == BOOLCONST) {
		next();
	}
	else {
		parseerror();
	}
}


void parse_table_const() {
	whereami(__func__);
	match('{');
	if (lookahead == '(') {
		parse_tuple_list();
	}
	else {
		parse_atomic_type_list();
	}
	match('}');
}

void parse_tuple_list() {
	whereami(__func__);
	parse_tuple_const();
	while (lookahead == ',') {
		next();
		parse_tuple_const();
	}
}

void parse_atomic_type_list() {
	whereami(__func__);
	parse_atomic_type();
	while (lookahead == ',') {
		next();
		parse_atomic_type();
	}
}

void parse_tuple_const() {
	whereami(__func__);
	match('(');
	parse_atomic_const();
	while (lookahead == ',') {
		next();
		parse_atomic_const();
	}
	match(')');
}

void parse_if_stat() {
	whereami(__func__);
	match(IF);
	parse_expr();
	match(THEN);
	parse_stat_list();
	if (lookahead == ELSE) {
		next();
		parse_stat_list();
	}
	match(END);
}

void parse_while_stat() {
	whereami(__func__);
	match(WHILE);
	parse_expr();
	match(DO);
	parse_stat_list();
	match(END);
}

void parse_read_stat() {
	whereami(__func__);
	match(READ);
	parse_specifier();
	match(ID);
}

void parse_specifier() {
	whereami(__func__);
	if (lookahead == '[') {
		next();
		parse_expr();
		match(']');
	}
}

void parse_write_stat() {
	whereami(__func__);
	match(WRITE);
	parse_specifier();
	parse_expr();
}
