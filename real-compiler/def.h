#include <stdio.h>
#include <stdlib.h>

#define TOT_BUCKETS 1009

/* Il seguente e' ridefinito correggendo un warning del compilatore */
#define YY_INPUT(buf,result,max_size) \
    if ( YY_CURRENT_BUFFER_LVALUE->yy_is_interactive ) \
        { \
        int c = '*'; \
        unsigned int n; \
        for ( n = 0; n < max_size && \
                 (c = getc( yyin )) != EOF && c != '\n'; ++n ) \
            buf[n] = (char) c; \
        if ( c == '\n' ) \
            buf[n++] = (char) c; \
        if ( c == EOF && ferror( yyin ) ) \
            YY_FATAL_ERROR( "input in flex scanner failed" ); \
        result = n; \
        } \
    else \
        { \
        errno=0; \
        while ( (result = fread(buf, 1, max_size, yyin))==0 && ferror(yyin)) \
            { \
            if( errno != EINTR) \
                { \
                YY_FATAL_ERROR( "input in flex scanner failed" ); \
                break; \
                } \
            errno=0; \
            clearerr(yyin); \
            } \
        }\
\


typedef enum { FALSE, TRUE } Boolean;

typedef enum 
{
    N_ASSIGN_STAT,
    N_ATOMIC_TYPE,
    N_ATTR_DECL,
    N_BOOLCONST,
    N_COMP_EXPR,
    N_DEF_STAT,
    N_EXTEND_EXPR,
    N_ID,
    N_IF_STAT,
    N_INTCONST,
    N_JOIN_EXPR,
    N_LOGIC_EXPR,
    N_MATH_EXPR,
    N_NEG_EXPR,
    N_PROGRAM,
    N_PROJECT_EXPR,
    N_READ_STAT,
    N_RENAME_EXPR,
    N_SELECT_EXPR,
    N_SPECIFIER,
    N_STAT_LIST,
    N_STRCONST,
    N_TABLE_CONST,
    N_TABLE_TYPE,
    N_TUPLE_CONST,
    N_TYPE,
    N_UPDATE_EXPR,
    N_WHILE_STAT,
    N_WRITE_STAT
} Typenode;

typedef union
{
    int ival;
    char *sval;
} Value;

typedef struct s_name
{
    char *name;
    struct s_name *next;
} Name;

typedef Name *Pname;

typedef struct snode
{
    int type;
    Value value;
    int line;
    struct snode *child, *brother;
} Node;

typedef Node *Pnode;

typedef struct s_schema
{
    char *name;
    int type;
    struct s_schema *next;
} Schema;

typedef Schema *Pschema;

typedef struct s_symbol
{
    int oid;
    int size;
    Schema schema;
    struct s_symbol *next;
} Symbol;

typedef Symbol *Psymbol;

typedef struct s_context
{
    int level;
    Pschema pschema;
    struct s_context *next;
} Context;

typedef Context *Pcontext;

typedef struct s_environment
{
    int level;
    int numobj;
    Pname pname;
    struct s_environment *next;
} Environment;

typedef Environment *Penvironment;

/* --------- For code generation --------- */

#define MAXARGS 3

typedef enum
{
    T_TCODE,
    T_NEWATOM,
    T_NEWTAB,
    T_POP,
    T_LDINT,
    T_LDSTR,
    T_LDTAB,
    T_IATTR,
    T_SATTR,
    T_ENDTAB,
    T_LOB,
    T_LAT,
    T_STO,
    T_SKIP,
    T_SKIPF,
    T_EQU,
    T_NEQ,
    T_IGT,
    T_IGE,
    T_ILT,
    T_ILE,
    T_SGT,
    T_SGE,
    T_SLT,
    T_SLE,
    T_PLUS,
    T_MINUS,
    T_TIMES,
    T_DIV,
    T_JOIN,
    T_ENDJOIN,
    T_UMI,
    T_NEG,
    T_PROJ,
    T_ATTR,
    T_ENDPROJ,
    T_SEL,
    T_ENDSEL,
    T_EXS,
    T_ENDEXS,
    T_ALL,
    T_ENDALL,
    T_UPD,
    T_ENDUPD,
    T_REMDUP,
    T_EXT,
    T_ENDEXT,
    T_GET,
    T_FGET,
    T_PRINT,
    T_FPRINT,
    T_HALT
} Operator;


typedef struct t_stat
{
    int address;
    Operator op;
    Value args[MAXARGS];
    struct t_stat *next;
} Tstat;

typedef struct
{
    Tstat *head;
    int size;
    Tstat *tail;
} Code;

typedef struct
{
    Operator op;
    char *name;
    int indent;
    char *format;
} Opdescr;

/* ------------- Prototypes ------------- */

Boolean compatible(char*, char*),
        duplicated(char*, Pschema),
        homonyms(Pschema, Pschema),
        name_in_environment(char*),
        name_in_list(char*, Pname),
        repeated_names(Pname),
        type_equal(Schema, Schema);

char *clear_string(char *s),
     *get_format(Schema),
     *nameop(Operator),
     *operator(int),
     //*strcat( char*, const char*), TODO
     //*strcpy (char*, const char*), TODO
     *update_lextab(char*),
     *valname(Pnode);
     
Code appcode(Code, Code),
     assign_stat(Pnode),
     attr_code(Pnode, Pschema),
     def_stat(Pnode),
     concode(Code, Code, ...),
     endcode(),
     expr(Pnode, Pschema),
     if_stat(Pnode),
     makecode(Operator),
     makecode1(Operator, Value),
     makecode2(Operator, Value, Value),
     makecode3(Operator, Value, Value, Value),
     make_get_fget(Operator, int, char*),
     make_ldint(int),
     make_ldstr(char *s),
     make_print_fprint(Operator, char*),
     make_sattr(char*),
     program(Pnode),
     read_stat(Pnode),
     specifier(Pnode),
     stat(Pnode),
     stat_list(Pnode),
     tuple_const(Pnode, Pschema),
     while_stat(Pnode),
     write_stat(Pnode);
     
Opdescr *get_descr(Operator);

Operator codop(char*);

Pname id_list(Pnode, int*),
      add_name_to_list(char*, Pname);
     
Pnode boolconstnode(int),
      idnode(char*),
      intconstnode(int),
      newnode(Typenode),
      qualnode(Typenode, int),
      strconstnode(char*);
     
Pschema append_schemas(Pschema, Pschema),
        atomic_type(Pnode),
        clone_schema(Pschema),
        name_in_constack(char*, int*, int*),
        name_in_context(char*),
        name_in_schema(char*, Pschema),
        schemanode(char*, int),
        table_type(Pnode),
        tuple_to_schema(Pnode);
	
Psymbol insert(Schema),
        lookup(char*);
	
Schema type(Pnode);

size_t strlen (const char*);

Tstat *newstat(Operator);

void codeprint(Code, int),
     freemem(void*, int),
     idlprint(Pname),
     init_compiler(),
     init_lextab(),
     init_symtab(),
     insert_name_into_environment(char*),
     *newmem(int),
     noderror(Pnode),
     pop_context(),
     pop_environment(),
     push_context(Pschema),
     push_environment(),
     eliminate(char*),
     relocate_address(Code, int),
     schprint(Schema),
     semerror(Pnode, char*),
     symprint(),
     syserror(char*),
     treeprint(Pnode, int),
     free_name_list(Pname);

int qualifier(Pnode),
    get_size(Pschema),
    get_attribute_offset(Pschema, char*),
    numobj_in_current_env();