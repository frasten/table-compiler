%{
#include "def.h"
#define YYSTYPE Pnode
#define YYDEBUG 1

extern char *yytext;
extern int line;
extern Value lexval;

int yydebug = 0;
Pnode root = NULL;
%}

%token ALL AND BOOLCONST BOOLEAN DO ELSE END EQ ERROR EXISTS EXTEND GE ID IF 
%token INTCONST INTEGER JOIN LE NE NOT OR PROGRAM PROJECT READ RENAME 
%token SELECT STRCONST STRING TABLE THEN UPDATE WHILE WRITE

%%

program : PROGRAM stat_list END {root = $$ = newnode(N_PROGRAM); $$->child = newnode(N_STAT_LIST); $$->child->child = $2;}
        ;

stat_list : stat ';' stat_list {$$ = $1; $1->brother = $3;}
          | stat
          ;

stat : def_stat
     | assign_stat
     | if_stat
     | while_stat
     | read_stat
     |  write_stat
     ;

def_stat : type id_list {$$ = newnode(N_DEF_STAT); $$->child = $1; $1->brother = $2;}
         ;

id_list : ID {$$ = idnode(lexval.sval);} ',' id_list {$$ = $2; $2->brother = $4;}
        | ID {$$ = idnode(lexval.sval);} 
        ;

type : atomic_type {$$ = $1;}
     | table_type {$$ = $1;}
     ;

atomic_type : INTEGER {$$ = qualnode(N_ATOMIC_TYPE, INTEGER);}
            | STRING {$$ = qualnode(N_ATOMIC_TYPE, STRING);}
            | BOOLEAN {$$ = qualnode(N_ATOMIC_TYPE, BOOLEAN);}
            ;

table_type :  TABLE '(' attr_list ')' {$$ = qualnode(N_TABLE_TYPE, TABLE); $$->child = $3;}
           ;

attr_list : attr_decl ',' attr_list {$$ = $1; $1->brother = $3;}
          | attr_decl
          ;
          
attr_decl : atomic_type ID {$$ = newnode(N_ATTR_DECL); $$->child = $1; $1->brother = idnode(lexval.sval);}
          ;
          
assign_stat : ID {$$ = idnode(lexval.sval);} '=' expr {$$ = newnode(N_ASSIGN_STAT); $$->child = $2; $2->brother = $4;}
            ;

expr : expr bool_op bool_term {$$ = $2; $$->child = $1; $1->brother = $3;}
     | bool_term
     ;
     
bool_op : AND {$$ = qualnode(N_LOGIC_EXPR, AND);}
        | OR {$$ = qualnode(N_LOGIC_EXPR, OR);}
        ;

bool_term : comp_term comp_op comp_term {$$ = $2; $$->child = $1; $1->brother = $3;}
          | comp_term
          ;

comp_op : EQ {$$ = qualnode(N_COMP_EXPR, EQ);}
        | NE {$$ = qualnode(N_COMP_EXPR, NE);}
        | '>' {$$ = qualnode(N_COMP_EXPR, '>');}
        | GE {$$ = qualnode(N_COMP_EXPR, GE);}
        | '<' {$$ = qualnode(N_COMP_EXPR, '<');}
        | LE {$$ = qualnode(N_COMP_EXPR, LE);}
        ;
        
comp_term : comp_term low_bin_op low_term {$$ = $2; $$->child = $1; $1->brother = $3;}
          | low_term
          ;

low_bin_op : '+' {$$ = qualnode(N_MATH_EXPR, '+');}
           | '-' {$$ = qualnode(N_MATH_EXPR, '-');}
           ;
           
low_term : low_term high_bin_op factor {$$ = $2; 
                                        if($$->type == N_JOIN_EXPR) 
                                        {
                                          $1->brother = $$->child;
                                          $1->brother->brother = $3;
                                          $$->child = $1; 
                                        }
                                        else
                                        {
                                          $$->child = $1; 
                                          $1->brother = $3;
                                        }}
         | factor
         ;
         
high_bin_op : '*' {$$ = qualnode(N_MATH_EXPR, '*');}
            | '/' {$$ = qualnode(N_MATH_EXPR, '/');}
            | join_op 
            ;
            
factor : unary_op factor {$$ = $1; if($1->type == N_SELECT_EXPR) 
                                     $1->child->brother = $2;
                                    else if($$->type == N_PROJECT_EXPR || $$->type == N_RENAME_EXPR || $$->type == N_UPDATE_EXPR || $$->type == N_EXTEND_EXPR)
                                    {
				      $$->child = $2; 
                                      $$->child->brother = $$->brother;
                                      $$->brother = NULL;
                                    }
                                    else 
                                      $$->child = $2;}
       | '(' expr ')' {$$ = $2;}
       | ID {$$ = idnode(lexval.sval);}
       | const
       ;

unary_op : '-' {$$ = qualnode(N_NEG_EXPR, '-');}
         | NOT {$$ = qualnode(N_NEG_EXPR, NOT);}
         | project_op
         | select_op
         | exists_op
         | all_op
         | update_op
         | extend_op
         | rename_op
         ;

join_op : JOIN '[' expr ']' {$$ = newnode(N_JOIN_EXPR); $$->child = $3;}
        ;

project_op : PROJECT '[' id_list ']' {$$ = newnode(N_PROJECT_EXPR); $$->brother = $3;}
           ;

select_op : SELECT '[' expr ']' {$$ = qualnode(N_SELECT_EXPR, SELECT); $$->child = $3;}
          ;

exists_op : EXISTS '[' expr ']' {$$ = qualnode(N_SELECT_EXPR, EXISTS); $$->child = $3;}
          ;

all_op : ALL '[' expr ']' {$$ = qualnode(N_SELECT_EXPR, ALL); $$->child = $3;}
          ;
          
update_op : UPDATE '[' ID {$$ = idnode(lexval.sval);} '=' expr ']' {$$ = newnode(N_UPDATE_EXPR); $$->brother = $4; $4->brother = $6;}
          ;

extend_op : EXTEND '[' atomic_type ID {$$ = idnode(lexval.sval);} '=' expr ']' {$$ = newnode(N_EXTEND_EXPR); $$->brother = $3; $3->brother = $5; $5->brother = $7;}
          ;
          
rename_op : RENAME '[' id_list ']' {$$ = newnode(N_RENAME_EXPR); $$->brother = $3;}
           ;

const : atomic_const
      | table_const
      ;
      
atomic_const : INTCONST {$$ = intconstnode(lexval.ival);}
             | STRCONST {$$ = strconstnode(lexval.sval);}
             | BOOLCONST {$$ = boolconstnode(lexval.ival);}
             ;

table_const : '{' table_instance '}' {$$ = newnode(N_TABLE_CONST); $$->child = $2;}
            ;

table_instance : tuple_list
               | atomic_type_list
               ;
            
tuple_list : tuple_const ',' tuple_list {$$ = $1; $1->brother = $3;}
           | tuple_const
           ;
           
tuple_const : '(' atomic_const_list ')' {$$ = newnode(N_TUPLE_CONST); $$->child = $2;}
            ;
            
atomic_const_list : atomic_const ',' atomic_const_list {$$ = $1; $1->brother = $3;}
                  | atomic_const
                  ;

atomic_type_list : atomic_type ',' atomic_type_list {$$ = $1; $1->brother = $3;}
                 | atomic_type
                 ;
                  
if_stat : IF expr THEN stat_list else_part END {$$ = newnode(N_IF_STAT); $$->child = $2; $2->brother = newnode(N_STAT_LIST); $2->brother->child = $4; $2->brother->brother = $5;}
        ;
        
else_part : ELSE stat_list {$$ = newnode(N_STAT_LIST); $$->child = $2;}
          | {$$ = NULL;}
          ;
          
while_stat : WHILE expr DO stat_list END {$$ = newnode(N_WHILE_STAT); $$->child = $2; $2->brother = newnode(N_STAT_LIST); $2->brother->child = $4;}
           ;
            
read_stat : READ specifier ID {$$ = newnode(N_READ_STAT); $$->child = $2; $2->brother = idnode(lexval.sval);}
          ;

specifier : '[' expr ']' {$$ = newnode(N_SPECIFIER); $$->child = $2;}
          | {$$ = newnode(N_SPECIFIER);}
          ;
          
write_stat : WRITE specifier expr {$$ = newnode(N_WRITE_STAT); $$->child = $2; $2->brother = $3;}
           ;

%%

Pnode newnode(Typenode tn)
{
    Pnode p = malloc(sizeof(Node));
    p->type = tn;
    p->line = line;
    p->child = p->brother = NULL;
    return(p);
}

Pnode qualnode(Typenode tn, int qual)
{
    Pnode p = newnode(tn);
    p->value.ival = qual;
    return(p);
}

Pnode intconstnode(int i)
{
    Pnode p = newnode(N_INTCONST);
    p->value.ival = i;
    return(p);
}

Pnode strconstnode(char *s)
{
    Pnode p = newnode(N_STRCONST);
    p->value.sval = s;
    return(p);
}

Pnode boolconstnode(int b)
{
    Pnode p = newnode(N_BOOLCONST);
    p->value.ival = b;
    return(p);
}

Pnode idnode(char *s)
{
    Pnode p = newnode(N_ID);
    p->value.sval = s;
    return(p);
}

int yyerror()
{
  printf("Line %d: syntax error on symbol \"%s\"\n", line, yytext);
  exit(-1);
}

