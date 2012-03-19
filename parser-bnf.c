#include <stdio.h>

#include "parser-bnf.h"
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

int parse() {
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
	if (lookahead == ';') {
		match(';');
		parse_stat_list();
	}
}

void parse_stat() {
	whereami(__func__);
	if (lookahead == ID) {
		// Assegnamento o definizione.
		next();
		if (lookahead == '=') {
			// Dovrei fare backtracking, ma facciamo invece iniziare
			// le due funzioni come se avessero gia' mangiato un token.
			// => SOLUZIONE SPORCHISSIMA
			parse_assign_stat();
		}
		else {
			parse_def_stat();
		}
	}
	else if (lookahead == IF) {
		parse_if_stat();
	}
	else if (lookahead == WHILE) {
		parse_while_stat();
	}
	else if (lookahead == READ) {
		parse_read_stat();
	}
	else if (lookahead == WRITE) {
		parse_write_stat();
	}
	else {
		parseerror();
	}
}

void parse_def_stat() {
	whereami(__func__);
	parse_id_list(1);
	match(':');
	parse_type();
}

void parse_id_list(char skip_first_id) {
	whereami(__func__);
	if (lookahead == ID) {
		match(ID);
		if (lookahead == ',') {
			next();
			parse_id_list(0);
		}
	}
	// WARNING: questo skip e' un bruttissimo hack.
	else if (skip_first_id && lookahead == ',') {
		next();
		parse_id_list(0);
	}
	else {
		parseerror();
	}
}

void parse_type() {
	whereami(__func__);
	if (lookahead == INT ||
		lookahead == STRING ||
		lookahead == BOOL) {
		parse_atomic_type();
	}
	else {
		parse_table_type();
	}
}

void parse_atomic_type() {
	whereami(__func__);
	if (lookahead == INT ||
		lookahead == STRING ||
		lookahead == BOOL) {
		next();
	}
	else {
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
	if (lookahead == ',') {
		next();
		parse_attr_list();
	}
}

void parse_attr_decl() {
	whereami(__func__);
	match(ID);
	match(':');
	parse_atomic_type();
}

void parse_assign_stat() {
	whereami(__func__);
	//match(ID);
	match('=');
	parse_expr();
}

void parse_expr() {
	whereami(__func__);
	parse_bool_term();
	if (lookahead == AND || lookahead == OR) {
		parse_bool_op();
		parse_expr();
	}
}

void parse_bool_op() {
	whereami(__func__);
	if (lookahead == AND ||
		lookahead == OR) {
		next();
	}
	else {
		parseerror();
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
		parse_comp_op();
		parse_comp_term();
	}
}

void parse_comp_op() {
	whereami(__func__);
	if (lookahead == COMPARISON ||
		lookahead == DIFFER ||
		lookahead == GT |
		lookahead == GTE ||
		lookahead == LT ||
		lookahead == LTE ||
		lookahead == GT) {
		next();
	}
	else {
		parseerror();
	}
}

void parse_comp_term() {
	whereami(__func__);
	parse_low_term();
	if (lookahead == PLUS || lookahead == MINUS) {
		parse_low_bin_op();
		parse_low_term();
	}
}

void parse_low_bin_op() {
	whereami(__func__);
	if (lookahead == PLUS || lookahead == MINUS) {
		next();
	}
	else {
		parseerror();
	}
}

void parse_low_term() {
	whereami(__func__);
	parse_factor();
	if (lookahead == MULTIPLY ||
		lookahead == DIVIDE ||
		lookahead == JOIN) {
		parse_high_bin_op();
		parse_factor();
	}
}

void parse_high_bin_op() {
	whereami(__func__);
	if (lookahead == MULTIPLY || lookahead == DIVIDE) {
		next();
	}
	else if (lookahead == JOIN) {
		parse_join_op();
	}
	else {
		parseerror();
	}
}

void parse_factor() {
	whereami(__func__);
	if (lookahead == ID) {
		next();
	}
	else if (lookahead == '(') {
		match('(');
		parse_expr();
		match(')');
	}
	else if (lookahead == INTCONST ||
		lookahead == STRCONST ||
		lookahead == BOOLCONST ||
		lookahead == '{') {
		parse_const();
	}
	else {
		parse_unary_op();
		parse_factor();
	}
}

void parse_unary_op() {
	whereami(__func__);
	if (lookahead == MINUS || lookahead == NOT) {
		next();
	}
	else if (lookahead == PROJECT) {
		parse_project_op();
	}
	else if (lookahead == SELECT) {
		parse_select_op();
	}
	else if (lookahead == EXISTS) {
		parse_exists_op();
	}
	else if (lookahead == ALL) {
		parse_all_op();
	}
	else if (lookahead == EXTEND) {
		parse_extend_op();
	}
	else if (lookahead == UPDATE) {
		parse_update_op();
	}
	else if (lookahead == RENAME) {
		parse_rename_op();
	}
	else {
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
	parse_id_list(0);
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
	match(ID);
	match(':');
	parse_atomic_type();
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
	parse_id_list(0);
	match(']');
}

void parse_const() {
	whereami(__func__);
	if (lookahead == INTCONST ||
		lookahead == STRCONST ||
		lookahead == BOOLCONST) {
		parse_atomic_const();
	}
	else if (lookahead == '{') {
		parse_table_const();
	}
	else {
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
}

void parse_table_const() {
	whereami(__func__);
	match('{');
	parse_tuple_list();
	match('}');
}

void parse_tuple_list() {
	whereami(__func__);
	if (lookahead == '(') {
		parse_tuple_const();
		parse_tuple_list();
	}
}

void parse_tuple_const() {
	whereami(__func__);
	match('(');
	parse_atomic_const_list();
	match(')');
}

void parse_atomic_const_list() {
	whereami(__func__);
	parse_atomic_const();
	if (lookahead == ',') {
		next();
		parse_atomic_const_list();
	}
}

void parse_if_stat() {
	whereami(__func__);
	match(IF);
	parse_expr();
	match(THEN);
	parse_stat_list();
	parse_else_part();
	match(END);
}

void parse_else_part() {
	whereami(__func__);
	if (lookahead == ELSE) {
		next();
		parse_stat_list();
	}
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
