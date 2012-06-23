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
    while (schema != NULL) {
        if (schema->name == name)
            return TRUE;
        schema = schema->next;
    }
    return FALSE;
}

// Controlla se i due schemi in ingresso hanno nomi di attributo in comune
Boolean homonyms(Pschema schema1, Pschema schema2)
{
    Pschema ptr1, ptr2;
    for (ptr1 = schema1; ptr1 != NULL; ptr1 = ptr1->next)
    {
        for (ptr2 = schema2; ptr2 != NULL; ptr2 = ptr2->next)
        {
            if (ptr1->name == ptr2->name)
                return TRUE;
        }
    }
    return FALSE;
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
    Pname head = list;
    Pname new_element = (Pname) newmem(sizeof(Name));
    new_element->name = name;
    new_element->next = NULL;

    if (list == NULL)
        return new_element;
    while (list->next)
        list = list->next;
    list->next = new_element;

    return head;
}


void free_name_list(Pname list)
{
    Pname nextptr;
    while (list)
    {
        nextptr = list->next;
        freemem(list, sizeof(Name));
        list = nextptr;
    }
}


void free_schema(Pschema schema)
{
    Pschema nextptr;
    while (schema)
    {
        nextptr = schema->next;
        freemem(schema, sizeof(Schema));
        schema = nextptr;
    }
}


Code expr(Pnode root, Pschema pschema)
{
    Code code1, code2, code3;
    Schema schema1, schema2, schema3;
    int op, op2, offset, context;
    Psymbol symbol;

    pschema->name = NULL;
    pschema->next = NULL;
    switch(root->type) {
        case N_ID :
        {
            Pschema tmp;
            if ((tmp = name_in_constack(valname(root), &offset, &context)) != NULL)
            {
                // Proviamo a vedere se e' un attributo
                pschema->name = valname(root);
                pschema->type = tmp->type;
                Value v1; v1.ival = offset;
                Value v2; v2.ival = context;
                Value v3; v3.ival = get_size(tmp);
                return makecode3(T_LAT, v1, v2, v3);
            }
            else if ((symbol = lookup(valname(root))) != NULL)
            {
                /* Variabile */
                // Assegno il tipo
                *pschema = symbol->schema;
                Value v; v.ival = symbol->oid;
                return makecode1(T_LOB, v);
            }
            else
                semerror(root, "Undefined identifier");
        }
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
                {
                    Value v1; v1.ival = code2.size + 2;
                    Value v2; v2.ival = 2;
                    Value v3; v3.ival = 0;
                    return concode(
                        code1,
                        makecode1(T_SKIPF, v1),
                        code2,
                        makecode1(T_SKIP, v2),
                        makecode1(T_LDINT, v3),
                        endcode()
                        );
                }
                case OR:
                {
                    Value v1; v1.ival = 3;
                    Value v2; v2.ival = 1;
                    Value v3; v3.ival = code2.size + 1;
                    return concode(
                        code1,
                        makecode1(T_SKIPF, v1),
                        makecode1(T_LDINT, v2),
                        makecode1(T_SKIP, v3),
                        code2,
                        endcode()
                        );
                }
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
        case N_TABLE_CONST:
        {
            // Vediamo il primo figlio e da li' decidiamo come procedere:
            int numtuples = 0;
            Pschema tuple_schema;
            Code tuples_code;

            if (root->child->type == N_TUPLE_CONST)
            {
                /*
                N_TABLE_CONST
                   N_TUPLE_CONST
                      N_INTCONST (1)
                      N_STRCONST (alpha)
                   N_TUPLE_CONST
                      N_INTCONST (2)
                      N_STRCONST (beta)
                */
                // Prendiamo lo Schema dal primo figlio, poi confronteremo le tuple con esso.
                tuple_schema = tuple_to_schema(root->child);

                // Ciclo dei figli
                tuples_code = endcode();
                Pnode tup;
                for (tup = root->child; tup != NULL; tup = tup->brother)
                {
                    Code singlecode = tuple_const(tup, tuple_schema);
                    if (tuples_code.head == NULL)
                        tuples_code = singlecode;
                    else
                        tuples_code = appcode(tuples_code, singlecode);
                    numtuples++;
                }
            }
            else if (root->child->type == N_ATOMIC_TYPE)
            {
                /*
                N_TABLE_CONST
                   N_ATOMIC_TYPE (integer)
                   N_ATOMIC_TYPE (string)
                   N_ATOMIC_TYPE (boolean)
                */

                tuple_schema = tuple_to_schema(root);
            }
            else noderror(root->child);

            // Impostiamo lo schema.
            pschema->type = TABLE;
            pschema->next = tuple_schema;

            // Calcolo la dimensione della tupla in memoria
            Value v1; v1.ival = get_size(pschema);
            Value v2; v2.ival = numtuples;

            code2 = makecode2(T_LDTAB, v1, v2);
            if (numtuples > 0)
                code2 = appcode(code2, tuples_code);
            return appcode(code2, makecode(T_ENDTAB));
        }
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
            Pname n;
            for (n = names; n != NULL; n = n->next)
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

            Value v1; v1.ival = num_id;
            return concode(code1,
                           makecode1(T_PROJ, v1),
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
            Pschema s;
            for (s = schema1.next; s != NULL; s = s->next)
            {
                old_num++;
            }
            if (old_num != num_id)
                semerror(root->child, "Wrong number of attributes in rename");

            // Rinomino le variabili nello schema
            pschema->next = clone_schema(schema1.next);
            Pschema schemaptr = pschema->next;
            Pname n;
            for (n = names; n != NULL; n = n->next)
            {
                schemaptr->name = n->name;

                schemaptr = schemaptr->next;
            }

            return code1;
        }
        case N_SELECT_EXPR:
        {
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
                    op2 = T_ENDEXS;
                    break;
                case ALL:
                    pschema->type = BOOLEAN;
                    op = T_ALL;
                    op2 = T_ENDALL;
                    break;
                default: noderror(root);
            }

            Value v1; v1.ival = code1.size;
            return concode(
                code2,
                makecode1(op, v1),
                code1,
                makecode1(op2, v1),
                endcode()
                );
        }
        case N_JOIN_EXPR:
        {
            code1 = expr(root->child, &schema1);
            code3 = expr(root->child->brother->brother, &schema3);

            // Vincoli semantici
            if (schema1.type != TABLE || schema3.type != TABLE)
                semerror(root, "Join requires table operands");

            // Controlliamo gli attributi in comune
            if (homonyms(schema1.next, schema3.next))
                semerror(root, "Common attribute names in join operands");

            // Creiamo lo Schema di uscita
            Pschema leftschema = clone_schema(schema1.next);
            Pschema rightschema = clone_schema(schema3.next);

            pschema->type = TABLE;
            pschema->next = append_schemas(leftschema, rightschema);

            // Andiamo a valutare l'espressione di condizione nello schema unione.
            push_context(pschema->next);
            code2 = expr(root->child->brother, &schema2);
            pop_context();

            if (schema2.type != BOOLEAN)
                semerror(root, "Join requires boolean predicate");

            Value v1; v1.ival = code2.size;
            return concode(
                code1,
                code3,
                makecode1(T_JOIN, v1),
                code2,
                makecode1(T_ENDJOIN, v1),
                endcode()
                );
        }
        case N_UPDATE_EXPR:
        {
            code1 = expr(root->child, &schema1);
            push_context(schema1.next);
            code2 = expr(root->child->brother->brother, &schema2);
            pop_context();

            // Vincoli semantici
            if (schema1.type != TABLE)
                semerror(root, "Update requires table operand");

            Pschema attr = name_in_schema(valname(root->child->brother), schema1.next);
            if (attr == NULL)
                semerror(root->child->brother, "Unknown attribute name");

            if (!type_equal(schema2, *attr))
                semerror(root->child->brother->brother, "Attribute type and expression type must be equal in update");

            pschema->type = TABLE;
            pschema->next = clone_schema(schema1.next);

            Value v1; v1.ival = get_attribute_offset(schema1.next, attr->name);
            Value v2; v2.ival = get_size(attr);
            Value v3; v3.ival = code2.size;
            return concode(
                code1,
                makecode3(T_UPD, v1, v2, v3),
                code2,
                makecode1(T_ENDUPD, v3),
                makecode(T_REMDUP),
                endcode()
                );
        }
        case N_EXTEND_EXPR:
        {
            char *attrname;
            Pschema newattr, schema_tipo;

            code1 = expr(root->child, &schema1);

            // Vincoli semantici
            if (schema1.type != TABLE)
                semerror(root->child, "Extend requires table operand");

            push_context(schema1.next);
            code2 = expr(root->child->brother->brother->brother, &schema2);
            pop_context();

            schema_tipo = atomic_type(root->child->brother);

            attrname = valname(root->child->brother->brother);
            if (name_in_schema(attrname, schema1.next))
                semerror(root->child->brother->brother, "Attribute name in extend must be new");

            if (!type_equal(*schema_tipo, schema2))
                semerror(root->child->brother, "Attribute type and expression type must be equal in extend");


            // Unione tra gli schemi
            newattr = schemanode(attrname, schema_tipo->type);

            pschema->type = TABLE;
            pschema->next = append_schemas(clone_schema(schema1.next), newattr);

            Value v1; v1.ival = get_size(newattr);
            Value v2; v2.ival = code2.size;
            return concode(
                code1,
                makecode2(T_EXT, v1, v2),
                code2,
                makecode1(T_ENDEXT, v2),
                endcode()
                );
        }
        default: noderror(root);
    }
    return endcode();
}

Schema type(Pnode p)
{
    Pschema presult;
    Schema result;
    if (p->type == N_ATOMIC_TYPE)
    {
        presult = atomic_type(p);
        result = *presult;
        freemem(presult, sizeof(Schema));
        return result;
    }
    else if (p->type == N_TABLE_TYPE)
    {
        presult = table_type(p);
        result = *presult;
        freemem(presult, sizeof(Schema));
        return result;
    }
    else noderror(p);

    return *schemanode(NULL, 0); // Solo per eliminare il warning del compilatore
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
    Code retcode = endcode();
    // Utilizzata all'interno di Project
    for (;p != NULL; p = p->brother)
    {
        Value v1; v1.ival = get_attribute_offset(schema->next, valname(p));
        Value v2; v2.ival = get_size(name_in_schema(valname(p), schema));
        Code tmpcode = makecode2(T_ATTR, v1, v2);
        if (retcode.head == NULL)
            retcode = tmpcode;
        else
            retcode = appcode(retcode, tmpcode);
    }
    return retcode;
}

Pschema tuple_to_schema(Pnode p)
{
    Pschema tmp, result = NULL;
    Pnode attr;
    for (attr = p->child; attr != NULL; attr = attr->brother)
    {
        Pschema nuovo = (Pschema) newmem(sizeof(Schema));
        nuovo->next = NULL;
        if (attr->type == N_ATOMIC_TYPE)
        {
            // Per tuple di tipi
            nuovo->type = attr->value.ival;
        }
        else
        {
            Schema tipo;
            expr(attr, &tipo);
            nuovo->type = tipo.type;
        }
        if (result == NULL)
            result = tmp = nuovo;
        else
        {
            tmp->next = nuovo;
            tmp = tmp->next;
        }
    }
    return result;
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

    free_name_list(attr_name_list);

    return tableschema;
}
