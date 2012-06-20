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

Boolean repeated_names(Pname n)
{
    Pname nleft, nright;
    for (nleft = n; nleft != NULL; nleft = nleft->next)
    {
        for (nright = nleft->next; nright != NULL; nright = nright->next)
        {
            if (nleft->name == nright->name)
                return TRUE;
        }
    }
    return FALSE;
}

Boolean duplicated(char* name, Pschema schema)
{
    // TODO: non so se va implementata cosi', dipende a cosa serve.
    while (schema != NULL) {
        if (schema->name == name)
            return TRUE;
        schema = schema->next;
    }
    return FALSE;
}

Boolean homonyms(Pschema schema1, Pschema schema2)
{
    // TODO: non so se va implementata cosi', dipende se l'ordine conta negli schemi.
    while (schema1 != NULL && schema2 != NULL)
    {
        if (schema1->name != schema2->name)
            return FALSE;
        // TODO: non e' che vado a toccare gli oggetti passati in input? teoricamente no.
        schema1 = schema1->next;
        schema2 = schema2->next;
    }
    return TRUE;
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

    if (psch1 == NULL)
        return psch2;
    while(psch1->next)
        psch1 = psch1->next;
    psch1->next = psch2;
    return(head);
}


Pname add_name_to_list(char* name, Pname list)
{
    Pname new_element = (Pname) newmem(sizeof(Name));
    new_element->name = name;
    new_element->next = list;

    list = new_element;
    return list;
}


Code expr(Pnode root, Pschema pschema)
{
    Code code1, code2;
    Schema schema1, schema2;
    int op, op2, offset, context;
    Psymbol symbol;

    // TODO
    // 
    pschema->name = NULL;
    pschema->next = NULL;
    switch(root->type) {
        case N_ID : // TODO
            if ((symbol = lookup(valname(root))) != NULL)
            {
                /* Variabile */
                // Assegno il tipo
                *pschema = symbol->schema;

                return makecode1(T_LOB, symbol->oid);
            }
            else {
                // Proviamo a vedere se e' un attributo
                Pschema tmp = name_in_constack(valname(root), &offset, &context);
                if (tmp != NULL)
                {
                    pschema->name = valname(root);
                    pschema->type = tmp->type;
                    return makecode3(T_LAT, offset, context, get_size(tmp));
                }
                else semerror(root, "Undefined identifier");
            }
            break;
        case N_MATH_EXPR:
            /*
                    '+'
                    /
                   /
                (...) --> (...)
             */
            code1 = expr(root->child, &schema1);
            code2 = expr(root->child->brother, &schema2);

            // Vincoli semantici
            if (schema1.type != INTEGER || schema2.type != INTEGER)
                semerror(root, "Math operation requires integer types");
            pschema->type = INTEGER;

            switch(qualifier(root))
            {
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
        case N_LOGIC_EXPR:
            code1 = expr(root->child, &schema1);
            code2 = expr(root->child->brother, &schema2);

            // Vincoli semantici
            if (schema1.type != BOOLEAN || schema2.type != BOOLEAN)
                semerror(root, "Logic operation requires boolean types");
            pschema->type = BOOLEAN;

            switch (qualifier(root))
            {
                case AND:
                    return concode(
                        code1,
                        makecode1(T_SKIPF, code2.size + 2),
                        code2,
                        makecode1(T_SKIP, 2),
                        makecode1(T_LDINT, 0),
                        endcode()
                        );
                case OR:
                    return concode(
                        code1,
                        makecode1(T_SKIPF, 3),
                        makecode1(T_LDINT, 1),
                        makecode1(T_SKIP, code2.size + 1),
                        code2,
                        endcode()
                        );
                default: noderror(root);
            }
        case N_COMP_EXPR:
            code1 = expr(root->child, &schema1);
            code2 = expr(root->child->brother, &schema2);
            pschema->type = BOOLEAN;

            switch (qualifier(root))
            {
                case EQ:
                case NE:
                    if (!type_equal(schema1, schema2))
                        semerror(root, "Comparison requires same types");
                    return concode(
                        code1,
                        code2,
                        makecode(qualifier(root) == EQ ? T_EQU : T_NEQ),
                        endcode()
                        );
                case '>':
                case GE:
                case '<':
                case LE:
                    if (schema1.type == INTEGER && schema2.type == INTEGER)
                    {
                        switch (qualifier(root))
                        {
                            case '>': op = T_IGT; break;
                            case GE : op = T_IGE; break;
                            case '<': op = T_ILT; break;
                            case LE : op = T_ILE; break;
                        }
                    }
                    else if (schema1.type == STRING && schema2.type == STRING)
                    {
                        switch (qualifier(root))
                        {
                            case '>': op = T_SGT; break;
                            case GE : op = T_SGE; break;
                            case '<': op = T_SLT; break;
                            case LE : op = T_SLE; break;
                        }
                    }
                    else semerror(root, "Comparison requires same types");

                    // Vincoli OK
                    return concode(
                        code1,
                        code2,
                        makecode(op),
                        endcode()
                        );
                default: noderror(root);
            }

        case N_INTCONST:
            pschema->type = INTEGER;
            return make_ldint(root->value.ival);
        case N_STRCONST:
            pschema->type = STRING;
            return make_ldstr(valname(root));
        case N_BOOLCONST:
            pschema->type = BOOLEAN;
            return make_ldint(root->value.ival);
        case N_NEG_EXPR:
            code1 = expr(root->child, &schema1);
            switch (qualifier(root))
            {
                case NOT:
                    if (schema1.type != BOOLEAN)
                        semerror(root->child, "Logical negation requires boolean type");
                    pschema->type = BOOLEAN;
                    return appcode(code1, makecode(T_NEG));
                case '-':
                    if (schema1.type != INTEGER)
                        semerror(root->child, "Unary minus requires integer type");
                    pschema->type = INTEGER;
                    return appcode(code1, makecode(T_UMI));
                    break;
                default: noderror(root);
            }
        case N_PROJECT_EXPR:
        {
            /*
              project_expr
                  /
                 /
                expr --> ID --> ID --> ...
             */
            code1 = expr(root->child, &schema1);

            // Vincoli semantici
            if (schema1.type != TABLE)
                semerror(root->child, "Project requires table operand");

            int num_id = 0;
            Pname names = id_list(root->child->brother, &num_id);
            if (repeated_names(names))
                semerror(root->child, "Duplicated name in list");


            // Costruzione dello Schema in uscita: filtro solo gli ID selezionati,
            // e in quell'ordine.
            pschema->type = TABLE;
            Pschema filtered = NULL, attr, attr_copy;
            for (Pname n = names; n != NULL; n = n->next)
            {
                attr = name_in_schema(n->name, schema1.next);
                if (attr == NULL)
                    semerror(root->child, "Unknown projection attribute name");
                attr_copy = schemanode(attr->name, attr->type);
                if (filtered == NULL)
                {
                    filtered = attr_copy;
                    pschema->next = filtered;
                }
                else
                {
                    filtered->next = attr_copy;
                    filtered = filtered->next;
                }
            }

            return concode(code1,
                           makecode1(T_PROJ, num_id),
                           attr_code(root->child->brother, &schema1),
                           makecode(T_ENDPROJ),
                           makecode(T_REMDUP),
                           endcode()
                           );
        }
        case N_RENAME_EXPR:
        {
            /*
               rename_expr
                   /
                  /
                expr --> id --> id...
             */
            code1 = expr(root->child, &schema1);

            // Vincoli semantici
            if (schema1.type != TABLE)
                semerror(root->child, "Rename requires table operand");

            int num_id = 0;
            Pname names = id_list(root->child->brother, &num_id);
            if (repeated_names(names))
                semerror(root->child, "Duplicated name in list");

            // Conto il numero di attributi attuali della tabella
            int old_num = 0;
            for (Pschema s = schema1.next; s != NULL; s = s->next)
            {
                old_num++;
            }
            if (old_num != num_id)
                semerror(root->child, "Wrong number of attributes in rename");

            // Rinomino le variabili nello schema
            pschema->next = clone_schema(schema1.next);
            Pschema schemaptr = pschema->next;
            for (Pname n = names; n != NULL; n = n->next)
            {
                schemaptr->name = n->name;

                schemaptr = schemaptr->next;
            }

            return code1;
        }
        case N_SELECT_EXPR:
            code2 = expr(root->child->brother, &schema2);
            push_context(schema2.next);
            code1 = expr(root->child, &schema1);            
            pop_context();

            if (schema1.type != BOOLEAN)
                semerror(root->child, "Selection requires boolean predicate");
            if (schema2.type != TABLE)
                semerror(root->child->brother, "Selection requires table operand");

            switch (qualifier(root)) {
                case SELECT:
                    pschema->type = TABLE;
                    pschema->next = clone_schema(schema2.next);
                    op = T_SEL;
                    op2 = T_ENDSEL;
                    break;
                case EXISTS:
                    pschema->type = BOOLEAN;
                    op = T_EXS;
                    op = T_ENDEXS;
                    break;
                case ALL:
                    pschema->type = BOOLEAN;
                    op = T_ALL;
                    op = T_ENDALL;
                    break;
                default: noderror(root);
            }

            return concode(
                code2,
                makecode1(op, code1.size),
                code1,
                makecode1(op2, code1.size),
                endcode()
                );
        default: noderror(root);
    }
    return endcode();
}

Schema type(Pnode p)
{
    if (p->type == N_ATOMIC_TYPE)
        return *atomic_type(p);
    else if (p->type == N_TABLE_TYPE)
        return *table_type(p);
    else noderror(p);
}

Pschema atomic_type(Pnode p)
{
    if (p->type != N_ATOMIC_TYPE)
        noderror(p);
    Pschema schema = schemanode(NULL, p->value.ival);
    return schema;
}

Code attr_code(Pnode p, Pschema schema)
{
    Code retcode = endcode(); // TODO: Memory leak?
    // Utilizzata all'interno di Project
    for (;p != NULL; p = p->brother)
    {
        Code tmpcode = makecode2(T_ATTR,
                                 get_attribute_offset(schema->next, valname(p)),
                                 get_size(name_in_schema(valname(p), schema))
                                 );
        if (retcode.head == NULL)
            retcode = tmpcode;
        else
            retcode = appcode(retcode, tmpcode);
    }
    return retcode;
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
    Pnode decl;
    Pschema tableschema, attrschema;
    /*
        table_type
          /
         /
        attr_decl --> attr_decl --> .....
     */
    if (p->type != N_TABLE_TYPE)
        noderror(p);
    tableschema = schemanode(NULL, TABLE);

    /*
           attr_decl
            /
           /
        atomic_type ---> ID
     */

    Pname attr_name_list = NULL;
    for (decl = p->child; decl != NULL; decl = decl->brother)
    {
        if (decl->child->type != N_ATOMIC_TYPE) noderror(decl->child);
        if (decl->child->brother->type != N_ID) noderror(decl->child->brother);

        attrschema = atomic_type(decl->child);
        attrschema->name = valname(decl->child->brother);
        attr_name_list = add_name_to_list(attrschema->name, attr_name_list);

        tableschema = append_schemas(tableschema, attrschema);
    }
    // Controllo semantico di attributi duplicati
    if (repeated_names(attr_name_list))
        semerror(p, "Duplicate attribute name");

    return tableschema;
}
