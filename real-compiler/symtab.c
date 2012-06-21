#include "def.h"
#include "parser.h"
#include <string.h>

#define SHIFT   5
#define MAXFORMAT 1000

extern int oid_counter;

static Pname lextab[TOT_BUCKETS];

Psymbol symtab[TOT_BUCKETS];

void syserror(char *message)
{
    printf("System error: %s\n", message);
    exit(-1);
}

void noderror(Pnode p)
{
    printf("Inconsistent node (%d) in parse tree\n", p->type);
    printf("Node:\n");
    treeprint(p, 0);
    exit(-1);
}

void *newmem(int size)
{
    char *p;
    static long size_allocated = 0;
        
    if((p = malloc(size)) == NULL)
        syserror("Failure in memory allocation");
    size_allocated += size;
    return(p);
}

void freemem(void *p, int size)
{
    static long size_deallocated = 0;
        
    free(p);
    size_deallocated += size;
}

int hash_function(char *s)
{
    int i, h=0;

    for(i=0; s[i] != '\0'; i++)
    {
        h = ((h << SHIFT) + s[i]) % TOT_BUCKETS;
    }
    return(h);
}

void init_lextab()
{
    int i;

    for(i = 0; i < TOT_BUCKETS; i++)
    {
        lextab[i] = NULL;
    }
}

void init_symtab()
{
    int i;

    for(i = 0; i < TOT_BUCKETS; i++)
    {
        symtab[i] = NULL;
    }
}

char *update_lextab(char *s)
{
    int index;
    Pname p;
    char *ps;

    index = hash_function(s);
    for(p = lextab[index]; p != NULL; p = p->next)
    {
        if(strcmp(p->name, s) == 0)
            return(p->name);
    }
    ps = newmem(strlen(s)+1);
    strcpy(ps, s);
    p = lextab[index];
    lextab[index] = (Pname) newmem(sizeof(Name));
    lextab[index]->name = ps;
    lextab[index]->next = p;
    return(lextab[index]->name);
}

Psymbol lookup(char *name)
{
    int index;
    Psymbol psymbol;

    index = hash_function(name);
    for(psymbol = symtab[index]; psymbol != NULL; psymbol = psymbol->next)
    {
        if(psymbol->schema.name == name)
            return(psymbol);
    }
    return(NULL);
}

Psymbol insert(Schema schema)
{
    int index;
    Psymbol psymbol;

    index = hash_function(schema.name);
    psymbol = symtab[index];
    symtab[index] = (Psymbol) newmem(sizeof(Symbol));
    symtab[index]->oid = oid_counter++;
    symtab[index]->size = get_size(&schema);
    symtab[index]->schema = schema;
    symtab[index]->next = psymbol;
    return(symtab[index]);
}

int get_size(Pschema pschema)
{
    Pschema psch;
    int tupsize = 0;
    
    switch (pschema->type)
    {
        case INTEGER: 
        case BOOLEAN: 
            return sizeof(int);
        case STRING: 
            return sizeof(char *);
        case TABLE: 
            for(psch = pschema->next; psch; psch = psch->next)
            {
                if(psch->type == STRING)
                    tupsize += sizeof(char *);
                else
                    tupsize += sizeof(int);
            }
            return (tupsize);
    }
    return -1;
}

int get_attribute_offset(Pschema pschema, char *attrname)
{
    int attroffset;
    
    for(attroffset = 0; pschema->name != attrname && pschema != NULL; pschema = pschema->next)
    {
        attroffset += get_size(pschema);
    }
    if(pschema != NULL)
        return(attroffset);
    syserror("get_attribute_offset()");
    return -1;
}

char *get_format(Schema schema)
{
    char *format;
    Pschema pschema;
    char *attr_name, *atomic_type;
    Boolean first = TRUE;
    
    format = (char*) newmem(MAXFORMAT);
    switch(schema.type)
    {
        case INTEGER: 
            sprintf(format, "i"); 
            break;
        case STRING: 
            sprintf(format, "s"); 
            break;
        case BOOLEAN: 
            sprintf(format, "b"); 
            break;
        case TABLE: 
            sprintf(format, "(");
            for(pschema = schema.next; pschema; pschema = pschema->next)
            {
                attr_name = (pschema->name ? pschema->name : "?");
                atomic_type = (pschema->type == INTEGER ? "i" : (pschema->type == STRING ? "s" : "b"));
                if(first == FALSE)
                    strcat(format, ",");
                sprintf(&format[strlen(format)], "%s:%s", attr_name, atomic_type);
                first = FALSE;
            }
            strcat(format, ")");
            break;
        default:
            syserror("get_format()");
    }
    return(format);
}

void eliminate(char *name)
{
    int index;
    Psymbol psymb, prec;

    index = hash_function(name);
    prec = psymb = symtab[index];
    while(psymb != NULL)
    {
        if(psymb->schema.name == name)
        {
            if(psymb == prec)
                symtab[index] = psymb->next;
            else
                prec->next = psymb->next;
            freemem(psymb, (int)sizeof(Symbol));
            return;
        }
        prec = psymb;
        psymb = psymb->next;
    }
    syserror("No name to be removed from symbol table");
}
