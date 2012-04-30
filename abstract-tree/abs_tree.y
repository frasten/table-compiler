%{
#include <stdio.h>

#include "lexer.h"
#include "node.h"

#define YYSTYPE Pnode

Pnode root = NULL;

void yyerror(char* message);

extern int linenumber;

%}
%token PROGRAM END IF THEN ELSE WHILE DO READ WRITE AND OR NOT LTE GTE COMPARISON
%token DIFFER PROJECT SELECT EXISTS ALL JOIN UPDATE EXTEND RENAME INT STRING
%token BOOL TABLE INTCONST STRCONST BOOLCONST ID ERROR
%%
// GRAMMATICA
    program : PROGRAM stat_list END {
                  $$ = root = nontermnode(NPROGRAM);
                  $$->child = nontermnode(NSTAT_LIST);
                  $$->child->child = $2;
              }
            ;

    stat_list : stat ';' stat_list {
                    $$ = $1;
                    $$->brother = $3;
                }
              | stat
              ;

    stat : def_stat
         | assign_stat
         | if_stat
         | while_stat
         | read_stat
         | write_stat
         ;

    def_stat : type id_list {
                   $$ = nontermnode(NDEF_STAT);
                   $$->child = $1;
                   $1->brother = $2;
               }
             ;

    id_list : ID {$$ = idnode();} ',' id_list {
                  $$ = $2;
                  $$->brother = $4;
              }
            | ID {$$ = idnode();}
            ;

    type : atomic_type
         | table_type
         ;

    atomic_type : INT {
                      $$ = nontermnode(NATOMIC_TYPE);
                      $$->child = keynode(T_INTEGER);
                  }
                | STRING {
                      $$ = nontermnode(NATOMIC_TYPE);
                      $$->child = keynode(T_STRING);
                  }
                | BOOL {
                      $$ = nontermnode(NATOMIC_TYPE);
                      $$->child = keynode(T_BOOLEAN);
                  }
                ;

    table_type : TABLE '(' attr_list ')'
               ;

    attr_list : attr_decl ',' attr_list
              | attr_decl
              ;

    attr_decl : atomic_type ID
              ;

    assign_stat : ID '=' expr
                ;

    expr : expr bool_op bool_term
         | bool_term
         ;

    bool_op : AND
            | OR
            ;

    bool_term : comp_term comp_op comp_term
              | comp_term
              ;

    comp_op : COMPARISON
            | DIFFER
            | '>'
            | GTE
            | '<'
            | LTE
            ;

    comp_term : comp_term low_bin_op low_term
              | low_term
              ;

    low_bin_op : '+'
               | '-'
               ;

    low_term : low_term high_bin_op factor
             | factor
             ;

    high_bin_op : '*'
                | '/'
                | join_op
                ;

    factor : unary_op factor
           | '(' expr ')'
           | ID
           | constant
           ;

    unary_op : '-'
             | NOT
             | project_op
             | select_op
             | exists_op
             | all_op
             | update_op
             | extend_op
             | rename_op
             ;

    join_op : JOIN '[' expr ']'
            ;

    project_op : PROJECT '[' id_list ']'
               ;

    select_op : SELECT '[' expr ']'
              ;

    exists_op : EXISTS '[' expr ']'
              ;

    all_op : ALL '[' expr ']'
           ;

    extend_op : EXTEND '[' atomic_type ID '=' expr ']'
              ;

    update_op : UPDATE '[' ID '=' expr ']'
              ;

    rename_op : RENAME '[' id_list ']'
              ;

    constant : atomic_const
             | table_const
             ;

    atomic_const : INTCONST
                 | STRCONST
                 | BOOLCONST
                 ;

    table_const : '{' table_instance '}'
                ;

    table_instance : tuple_list
                   | atomic_type_list
                   ;

    tuple_list : tuple_const ',' tuple_list
               | tuple_const
               ;

    tuple_const : '(' atomic_const_list ')'
                ;

    atomic_const_list : atomic_const ',' atomic_const_list
                      | atomic_const
                      ;

    atomic_type_list : atomic_type ',' atomic_type_list
                     | atomic_type
                     ;

    if_stat : IF expr THEN stat_list else_part END
            ;

    else_part : ELSE stat_list
              |
              ;

    while_stat : WHILE expr DO stat_list END
               ;

    read_stat : READ specifier ID
              ;

    specifier : '[' expr ']'
              |
              ;

    write_stat : WRITE specifier expr
               ;

%%
void yyerror(char* message) {
    printf("Riga: %d near %s\n", linenumber, yytext);
    fprintf(stderr, "%s\n", message);
}

int main() {
    linenumber = 1;
    yyparse();
    print_tree(root, 0);
    print_tree_graphviz(root);
    return 0;
}