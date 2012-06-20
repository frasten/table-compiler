#include "def.h"
#include "parser.h"

Boolean type_equal(Schema schema1, Schema schema2)
{
    Pschema p1, p2;
    
    if(schema1.type != schema2.type)
        return(FALSE);
    if(schema1.type == TABLE)
    {
        for(p1 = schema1.next, p2 = schema2.next; p1 != NULL && p2 != NULL; p1= p1->next, p2 = p2->next)
            if(p1->type != p2->type || !compatible(p1->name, p2->name))
                return(FALSE);
        return(p1 == NULL && p2 == NULL);
    }
    else
        return(TRUE);
}

Boolean compatible(char *name1, char *name2)
{
    return(name1 == NULL || name2 == NULL || name1 == name2);
}

Pschema clone_schema(Pschema pschema)
{
    Pschema clone, psch;
    
    clone = psch = (Pschema) newmem(sizeof(Schema));
    *psch = *pschema;
    while(pschema->next)
    {
        psch->next = (Pschema) newmem(sizeof(Schema));
        *(psch->next) = *(pschema->next);
        psch = psch->next;
        pschema = pschema->next;
    }
    return (clone);
}

Pschema append_schemas(Pschema psch1, Pschema psch2)
{
    Pschema head = psch1;
    
    while(psch1->next)
        psch1 = psch1->next;
    psch1->next = psch2;
    return(head);
}


Code expr(Pnode root, Pschema pschema) {
    Code code1, code2;
    Schema schema1, schema2;
    int op, offset, context;
    Psymbol symbol;

    // TODO
    // 
    // 
    pschema->name = NULL;
    pschema->next = NULL;
    switch(root->type) {
        case N_ID : // TODO
            if ((symbol = lookup(root->value.sval)) != NULL) {
                // Variabile
                // Assegno il tipo
                *pschema = symbol->schema;

                return makecode1(T_LOB, symbol->oid);
            }
            else if (name_in_constack(root->value.sval, &offset, &context) != NULL) {
                // Attributo nel contesto corrente
                printf("Contesto corrente.\n");
            }
            else {
                // Attributo in un contesto esterno
                printf("Contesto esterno.\n");
            }
            break;
        case N_MATH_EXPR:
            /*
                    '+'
                    /
                   /
                  ID(a) --> INTCONST(5)
             */
            code1 = expr(root->child, &schema1);
            code2 = expr(root->child->brother, &schema2);
            if (schema1.type != INTEGER || schema2.type != INTEGER)
                semerror(root, "Math operation requires integer types");
            pschema->type = INTEGER;
            switch(qualifier(root)) {
                case '+' : op = T_PLUS; break;
                case '-' : op = T_MINUS; break;
                case '*' : op = T_TIMES; break;
                case '/' : op = T_DIV; break;
                default: noderror(root);
            }
            return concode(code1,
                           code2,
                           makecode(op),
                           endcode());
        // TODO
        case N_INTCONST:
            pschema->type = INTEGER;
            return make_ldint(root->value.ival);
        case N_STRCONST:
            pschema->type = STRING;
            return make_ldstr(root->value.sval);
        case N_BOOLCONST:
            pschema->type = BOOLEAN;
            return make_ldint(root->value.ival);
    }
    return endcode();
}

Schema type(Pnode p)
{
    // TODO
}

Pschema atomic_type(Pnode p)
{
    if (p->type != N_ATOMIC_TYPE)
        noderror(p);
    Pschema schema = schemanode(NULL, p->value.ival);
    return schema;
}

/* Crea uno Schema vuoto */
Pschema schemanode(char* name, int type)
{
    Pschema schema = (Pschema) newmem(sizeof(Schema));
    schema->next = NULL;
    schema->name = name;
    schema->type = type;
    return schema;
}

Pschema table_type(Pnode p)
{
    // TODO
}