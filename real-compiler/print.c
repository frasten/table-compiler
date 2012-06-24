#include "def.h"
#include "parser.h"

#define MAXPFORMAT 100

extern Psymbol symtab[];

char* tabtypes[] =
{
        "N_ASSIGN_STAT",
        "N_ATOMIC_TYPE",
        "N_ATTR_DECL",
        "N_BOOLCONST",
        "N_COMP_EXPR",
        "N_DEF_STAT",
        "N_EXTEND_EXPR",
        "N_ID",
        "N_IF_STAT",
        "N_INTCONST",
        "N_JOIN_EXPR",
        "N_LOGIC_EXPR",
        "N_MATH_EXPR",
        "N_NEG_EXPR",
        "N_PROGRAM",
        "N_PROJECT_EXPR",
        "N_READ_STAT",
        "N_RENAME_EXPR",
        "N_SELECT_EXPR",
        "N_SPECIFIER",
        "N_STAT_LIST",
        "N_STRCONST",
        "N_TABLE_CONST",
        "N_TABLE_TYPE",
        "N_TUPLE_CONST",
        "N_TYPE",
        "N_UPDATE_EXPR",
        "N_WHILE_STAT",
        "N_WRITE_STAT"
};

#define TOTOP 19

struct {int symbol; char* operator;} tabop[TOTOP] =
{
    {INTEGER, "integer"},
    {STRING, "string"},
    {BOOLEAN, "boolean"},
    {SELECT, "select"},
    {EXISTS, "exists"},
    {ALL, "all"},
    {NOT, "not"},
    {'+', "+"},
    {'-', "-"},
    {'*', "*"},
    {'/', "/"},
    {EQ, "=="},
    {NE, "!="},
    {'>', ">"},
    {GE, ">="},
    {'<', "<"},
    {LE, "<="},
    {AND, "and"},
    {OR, "or"}
};

char *operator(int symbol)
{
    int i;
    
    for(i=0; i<TOTOP; i++)
        if(tabop[i].symbol == symbol)
            return(tabop[i].operator);
    return(NULL);
}

void treeprint(Pnode root, int indent)
{
    int i;
    Pnode p;  
    
    for(i=0; i<indent; i++)
        printf("   ");
    printf("%s", tabtypes[root->type]);
    if(root->type == N_ATOMIC_TYPE || root->type == N_MATH_EXPR || root->type == N_COMP_EXPR || root->type == N_LOGIC_EXPR || root->type == N_NEG_EXPR || root->type == N_SELECT_EXPR)
        printf(" (%s)", operator(root->value.ival));
    if(root->type == N_ID)
        printf(" (%s)", root->value.sval);
    else if(root->type == N_INTCONST)
        printf(" (%d)", root->value.ival);
    else if(root->type == N_STRCONST)
        printf(" (%s)", root->value.sval);
    else if(root->type == N_BOOLCONST)
        printf(" (%s)", (root->value.ival == 1 ? "true" : "false"));
    printf("\n");
    for(p=root->child; p != NULL; p = p->brother)
        treeprint(p, indent+1);
}

void symprint()
{
    int i;
    Psymbol psymbol;
    
    
    for(i = 0; i < TOT_BUCKETS; i++)
    {
        if(symtab[i])
        {
            printf("Bucket %d:\n", i);
            for(psymbol = symtab[i]; psymbol != NULL; psymbol = psymbol->next)
            {
                printf("\t");
    printf("(oid = %d, size = %d) : ", psymbol->oid, psymbol->size);
                schprint(psymbol->schema);
            }
        }
    }
}

void schprint(Pschema schema)
{
    printf("%s: %s", schema->name, (schema->type == TABLE ? "table" : operator(schema->type)));
    if(schema->type == TABLE)
    {
        printf("(");
        for(schema = schema->next; schema!= NULL; schema = schema->next)
            printf("%s:%s ", schema->name, operator(schema->type));
        printf(")");
    }
    printf("\n");
}

void idlprint(Pname idlist)
{
    while(idlist)
    {
        printf("%s ", idlist->name);
        idlist = idlist->next;
    }
    printf("\n");
}

void codeprint(Code code, int indent)
{
    Tstat *pstat;
    Opdescr *pdescr;
    char pformat[MAXPFORMAT], *pchar;
    int i;


    for (pstat = code.head; pstat; pstat = pstat->next)
    {
        pdescr = get_descr(pstat->op);
        if (pdescr->indent < 0)
            indent += pdescr->indent;
        for (i = 0; i < indent; i++)
        {
            printf("  ");
        }
        printf("%s", pdescr->name);
        int contatore = 0;
        for (*pformat = '\0', pchar = pdescr->format; *pchar != '\0'; pchar++)
        {
            switch(*pchar)
            {
                case 'i':
                    printf(" %d", (pstat->args[contatore]).ival);
                    //sprintf(&pformat[strlen(pformat)], " %%d");
                    break;
                case 's':
                    printf(" \"%s\"", (pstat->args[contatore]).sval);
                    //sprintf(&pformat[strlen(pformat)], " \"%%s\"");
                    break;
                default: syserror("codeprint()");
            }
            contatore++;
        }
        //sprintf(&pformat[strlen(pformat)], "\n");
        //printf(pformat, *pstat->args[0], pstat->args[1], pstat->args[2]);
        printf("\n");
        if (pdescr->indent > 0)
            indent += pdescr->indent;
    }
}
    

