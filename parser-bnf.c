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

int main() {
	linenumber = 1;
	parse();
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
	debug("Sono in parse program\n");
	match(PROGRAM);
	parse_stat_list();
	match(END);
}

void parse_stat_list() {
	debug("Sono in parse stat list\n");
	parse_stat();
	if (lookahead == ';') {
		match(';');
		parse_stat_list();
	}
}

void parse_stat() {
	debug("Sono in parse stat\n");
	// def stat: ...
	// assign stat: ID
	// if stat: IF
	// while: WHILE
	// read: READ
	// write: WRITE
	if (lookahead == ID) {
		// Assegnamento o definizione.
		next();
		if (lookahead == '=') {
			// Dovrei fare backtracking, ma facciamo invece iniziare
			// le due funzioni come se avessero gia' mangiato un token.
			// => SOLUZIONE SPORCHISSIMA
			// NO, non fattibile.
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
		parse_def_stat();
	}
}

void parse_def_stat() {
	debug("Sono in parse def stat\n");
	parse_id_list(1);
	match(':');
	parse_type();
}

void parse_id_list(char skip_first_id) {
	debug("Sono in parse id list\n");
	if (lookahead == ID) {
		match(ID);
		if (lookahead == ',') {
			next();
			parse_id_list(0);
		}
	}
	else if (skip_first_id && lookahead == ',') {
		next();
		parse_id_list(0);
	}
	else {
		parseerror();
	}
}

void parse_type() {
	debug("Sono in parse type\n");
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
	debug("Sono in parse atomic type\n");
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
	debug("Sono in parse table type\n");
	match(TABLE);
	match('(');
	parse_attr_list();
	match(')');
}

void parse_attr_list() {
	debug("Sono in parse attr list\n");
	parse_attr_decl();
	if (lookahead == ',') {
		next();
		parse_attr_list();
	}
}

void parse_attr_decl() {
	debug("Sono in parse attr decl\n");
	match(ID);
	match(':');
	parse_atomic_type();
}

void parse_assign_stat() {
	debug("Sono in parse assign stat\n");
	//match(ID);
	match('=');
	parse_expr();
}

void parse_expr() {
	debug("Sono in parse expr\n");
	parse_bool_term();
	if (lookahead == AND || lookahead == OR) {
		parse_bool_op();
		parse_expr();
	}
}

void parse_bool_op() {
	debug("Sono in parse bool op\n");
	if (lookahead == AND ||
		lookahead == OR) {
		next();
	}
	else {
		parseerror();
	}
}

void parse_bool_term() {
	debug("Sono in parse bool term\n");
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
	debug("Sono in parse comp op\n");
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
	debug("Sono in parse comp term\n");
	parse_low_term();
	if (lookahead == PLUS || lookahead == MINUS) {
		parse_low_bin_op();
		parse_low_term();
	}
}

void parse_low_bin_op() {
	debug("Sono in parse low bin op\n");
	if (lookahead == PLUS || lookahead == MINUS) {
		next();
	}
	else {
		parseerror();
	}
}

void parse_low_term() {
	debug("Sono in parse low term\n");
	parse_factor();
	if (lookahead == MULTIPLY ||
		lookahead == DIVIDE ||
		lookahead == JOIN) {
		parse_high_bin_op();
		parse_factor();
	}
}

void parse_high_bin_op() {
	debug("Sono in parse high bin op\n");
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
	debug("Sono in parse factor\n");
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
	debug("Sono in parse unary op\n");
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
	debug("Sono in parse join op\n");
	match(JOIN);
	match('[');
	parse_expr();
	match(']');
}

void parse_project_op() {
	debug("Sono in parse project op\n");
	match(PROJECT);
	match('[');
	parse_id_list(0);
	match(']');
}

void parse_select_op() {
	debug("Sono in parse select op\n");
	match(SELECT);
	match('[');
	parse_expr();
	match(']');
}

void parse_exists_op() {
	debug("Sono in parse exists op\n");
	match(EXISTS);
	match('[');
	parse_expr();
	match(']');
}

void parse_all_op() {
	debug("Sono in parse all op\n");
	match(ALL);
	match('[');
	parse_expr();
	match(']');
}

void parse_extend_op() {
	debug("Sono in parse extend op\n");
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
	debug("Sono in parse update op\n");
	match(UPDATE);
	match('[');
	match(ID);
	match('=');
	parse_expr();
	match(']');
}

void parse_rename_op() {
	debug("Sono in parse rename op\n");
	match(RENAME);
	match('[');
	parse_id_list(0);
	match(']');
}

void parse_const() {
	debug("Sono in parse const\n");
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
	debug("Sono in parse atomic const\n");
	if (lookahead == INTCONST ||
		lookahead == STRCONST ||
		lookahead == BOOLCONST) {
		next();
	}
}

void parse_table_const() {
	debug("Sono in parse table const\n");
	match('{');
	parse_tuple_list();
	match('}');
}

void parse_tuple_list() {
	debug("Sono in parse tuple list\n");
	if (lookahead == '}') {
		return; // Senza consumarlo
	}
	else {
		parse_tuple_const();
		match(',');
		parse_tuple_list();
	}
}

void parse_tuple_const() {
	debug("Sono in parse tuple const\n");
	match('(');
	parse_atomic_const_list();
	match(')');
}

void parse_atomic_const_list() {
	debug("Sono in parse atomic const list\n");
	parse_atomic_const();
	if (lookahead == ',') {
		next();
		parse_atomic_const_list();
	}
}

void parse_if_stat() {
	debug("Sono in parse if stat\n");
	match(IF);
	parse_expr();
	match(THEN);
	parse_stat_list();
	parse_else_part();
	match(END);
}

void parse_else_part() {
	debug("Sono in parse else part\n");
	if (lookahead == ELSE) {
		next();
		parse_stat_list();
	}
}

void parse_while_stat() {
	debug("Sono in parse while stat\n");
	match(WHILE);
	parse_expr();
	match(DO);
	parse_stat_list();
	match(END);
}

void parse_read_stat() {
	debug("Sono in parse read stat\n");
	match(READ);
	parse_specifier();
	match(ID);
}

void parse_specifier() {
	debug("Sono in parse specifier\n");
	if (lookahead == '[') {
		next();
		parse_expr();
		match(']');
	}
}

void parse_write_stat() {
	debug("Sono in parse write stat\n");
	match(WRITE);
	parse_specifier();
	parse_expr();
}
