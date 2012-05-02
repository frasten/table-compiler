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
                      $$ = pseudotermnode(T_ATOMIC_TYPE, INT);
                  }
                | STRING {
                      $$ = pseudotermnode(T_ATOMIC_TYPE, STRING);
                  }
                | BOOL {
                      $$ = pseudotermnode(T_ATOMIC_TYPE, BOOL);
                  }
                ;

    table_type : TABLE '(' attr_list ')' {
                     $$ = nontermnode(NTABLE_TYPE);
                     $$->child = $3;
                 }
               ;

    attr_list : attr_decl ',' attr_list {
                    $$ = $1;
                    $$->brother = $3;
                }
              | attr_decl
              ;

    attr_decl : atomic_type ID {
                    $$ = nontermnode(NATTR_DECL);
                    $$->child = $1;
                    $$->child->brother = idnode();
                }
              ;

    assign_stat : ID {$$ = idnode();} '=' expr {
                      $$ = nontermnode(NASSIGN_STAT);
                      $$->child = $2;
                      $$->child->brother = $4;
                  }
                ;

    expr : expr bool_op bool_term {
               $$ = $2;
               $$->child = $1;
               $1->brother = $3;
           }
         | bool_term {
               $$ = $1;
           }
         ;

    bool_op : AND {$$ = pseudotermnode(T_LOGIC_EXPR, AND);}
            | OR  {$$ = pseudotermnode(T_LOGIC_EXPR, OR);}
            ;

    bool_term : comp_term comp_op comp_term {
                    $$ = $2;
                    $$->child = $1;
                    $1->brother = $3;
                }
              | comp_term
              ;

    comp_op : COMPARISON {$$ = pseudotermnode(T_COMP_EXPR, COMPARISON);}
            | DIFFER     {$$ = pseudotermnode(T_COMP_EXPR, DIFFER);}
            | '>'        {$$ = pseudotermnode(T_COMP_EXPR, '>');}
            | GTE        {$$ = pseudotermnode(T_COMP_EXPR, GTE);}
            | '<'        {$$ = pseudotermnode(T_COMP_EXPR, '<');}
            | LTE        {$$ = pseudotermnode(T_COMP_EXPR, LTE);}
            ;

    comp_term : comp_term low_bin_op low_term {
                    $$ = $2;
                    $$->child = $1;
                    $1->brother = $3;
                }
              | low_term
              ;

    low_bin_op : '+' {$$ = pseudotermnode(T_MATH_EXPR, '+');}
               | '-' {$$ = pseudotermnode(T_MATH_EXPR, '-');}
               ;

    low_term : low_term high_bin_op factor {
                   $$ = $2;
                   $$->child = $1;
                   $1->brother = $3;
               }
             | factor
             ;

    high_bin_op : '*' {$$ = pseudotermnode(T_MATH_EXPR, '*');}
                | '/' {$$ = pseudotermnode(T_MATH_EXPR, '/');}
                | join_op {
                    $$ = nontermnode(NJOIN_EXPR);
                    $$->brother = $1;
                }
                ;

    factor : unary_op factor {
                 $$ = $1;
                 $1->child = $2;
             }
           | '(' expr ')' {$$ = $2;}
           | ID {$$ = idnode();}
           | constant
           ;

    unary_op : '-' {$$ = pseudotermnode(T_NEG_EXPR, '-');}
             | NOT {$$ = pseudotermnode(T_NEG_EXPR, NOT);}
             | project_op
             | select_op
             | exists_op
             | all_op
             | update_op
             | extend_op
             | rename_op
             ;

    join_op : JOIN '[' expr ']' {
                  $$ = $3;
              }
            ;

    project_op : PROJECT '[' id_list ']' {
                     $$ = nontermnode(NPROJECT_EXPR);
                     $$->brother = $3;
                 }
               ;

    select_op : SELECT '[' expr ']' {
                    $$ = pseudotermnode(T_SELECT_EXPR, SELECT);
                    $$->brother = $3;
                }
              ;

    exists_op : EXISTS '[' expr ']' {
                    $$ = pseudotermnode(T_SELECT_EXPR, EXISTS);
                    $$->brother = $3;
                }
              ;

    all_op : ALL '[' expr ']' {
                    $$ = pseudotermnode(T_SELECT_EXPR, ALL);
                    $$->brother = $3;
                }
           ;

    extend_op : EXTEND '[' atomic_type ID {$$ = idnode();} '=' expr ']' {
                    $$ = nontermnode(NEXTEND_EXPR);
                    $$->brother = $3;
                    $3->brother = $5;
                    $5->brother = $7;
                }
              ;

    update_op : UPDATE '[' ID {$$ = idnode();} '=' expr ']' {
                    $$ = nontermnode(NUPDATE_EXPR);
                    $$->brother = $4;
                    $4->brother = $6;
                }
              ;

    rename_op : RENAME '[' id_list ']' {
                    $$ = nontermnode(NRENAME_EXPR);
                    $$->brother = $3;
                }
              ;

    constant : atomic_const
             | table_const
             ;

    atomic_const : INTCONST {$$ = intconstnode();}
                 | STRCONST {$$ = strconstnode();}
                 | BOOLCONST {$$ = boolconstnode();}
                 ;

    table_const : '{' table_instance '}' {
                      $$ = nontermnode(NTABLE_INSTANCE);
                      $$->child = $2;
                  }
                ;

    table_instance : tuple_list
                   | atomic_type_list
                   ;

    tuple_list : tuple_const ',' tuple_list {
                     $$ = $1;
                     $$->brother = $3;
                 }
               | tuple_const
               ;

    tuple_const : '(' atomic_const_list ')' {
                      $$ = nontermnode(NTUPLE_CONST);
                      $$->child = $2;
                  }
                ;

    atomic_const_list : atomic_const ',' atomic_const_list {
                            $$ = $1;
                            $$->brother = $3;
                        }
                      | atomic_const
                      ;

    atomic_type_list : atomic_type ',' atomic_type_list {
                           $$ = $1;
                           $$->brother = $3;
                       }
                     | atomic_type
                     ;

    if_stat : IF expr THEN stat_list else_part END {
                  $$ = nontermnode(NIF_STAT);
                  $$->child = $2;
                  $2->brother = nontermnode(NSTAT_LIST);
                  $2->brother->child = $4;
                  $2->brother->brother = $5;
              }
            ;

    else_part : ELSE stat_list {
                    $$ = nontermnode(NSTAT_LIST);
                    $$->child = $2;
                }
              | {$$ = NULL;}
              ;

    while_stat : WHILE expr DO stat_list END {
                     $$ = nontermnode(NWHILE_STAT);
                     $$->child = $2;
                     $2->brother = nontermnode(NSTAT_LIST);
                     $2->brother->child = $4;
                 }
               ;

    read_stat : READ specifier ID {
                    $$ = nontermnode(NREAD_STAT);
                    $$->child = idnode();
                    $$->child->brother = $2;
                }
              ;

    specifier : '[' expr ']' {$$ = $2;}
              | {$$ = NULL;}
              ;

    write_stat : WRITE specifier expr {
                     $$ = nontermnode(NWRITE_STAT);
                     $$->child = $3;
                     $3->brother = $2;
                 }
               ;

%%
void yyerror(char* message) {
    fprintf(stderr, "%s Riga: %d near %s\n", message, linenumber, yytext);
}

int main() {
    int result;

    linenumber = 1;
    result = yyparse();
    if (result != 0) {
      // Parse error
      return result;
    }
    print_tree(root, 0);
    print_tree_graphviz(root);
    return 0;
}