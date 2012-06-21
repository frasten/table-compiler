#include "def.h"
#include "parser.h"

void semerror(Pnode p, char *message)
{
  printf("Line %d: %s\n", p->line, message);
  exit(-1);
}

int qualifier(Pnode p)
{
  return (p->value.ival);
}

char *valname(Pnode p)
{
  return (p->value.sval);
}

Code program(Pnode root)
{
  Code body = stat_list(root->child);
  
  Value v1; v1.ival = body.size + 2;
  return concode(makecode1(T_TCODE, v1),
         body,
         makecode(T_HALT),
         endcode());
}

Code stat_list(Pnode p)
{
    int env_num_vars = 0;
    push_environment();
/*
    stat_list
      /
     /
    stat --> stat --> stat
*/
    Code s = stat(p->child);
    // Scorro gli stats, dal secondo in poi
    for (p = p->child->brother; p != NULL; p = p->brother)
    {
        s = appcode(s, stat(p));
    }

    // Tolgo le variabili dichiarate in questo environment:
    if ((env_num_vars = numobj_in_current_env()) > 0)
    {
        Value v1; v1.ival = env_num_vars;
        s = appcode(s, makecode1(T_POP, v1));
    }
    pop_environment();
    return s;
}

Code def_stat(Pnode p)
{
/*
    def_stat
      /
     /
   type ---> ID ---> ID
*/
    int num_id = 0, op;
    Code c_temp, code_ret;
    Pname names;
    Pnode nodo_type = p->child;
    Schema schema;

    switch (nodo_type->type)
    {
        case N_ATOMIC_TYPE:
            op = T_NEWATOM;
            break;
        case N_TABLE_TYPE:
            op = T_NEWTAB;
            break;
        default:
            noderror(p->child);
    }

    /* Operazioni comuni tra dati atomici e table */

    // Carico la lista di ID delle variabili
    names = id_list(p->child->brother, &num_id);

    // Vincoli semantici
    if (repeated_names(names))
        semerror(p, "Variable redeclaration");

    // Controlliamo che nessuna variabile sia nell'environment
    for (Pname n = names; n != NULL; n = n->next)
    {
        if (name_in_environment(n->name))
            semerror(p, "Variable redeclaration");
        // Aggiungo la nuova variabile nell'environment
        insert_name_into_environment(n->name);
    }

    // Genero il codice per allocare ognuna delle variabili
    code_ret.head = NULL;
    for (; names != NULL; names = names->next)
    {
        // Aggiungo il nome alla Symbol Table
        schema = type(nodo_type);
        schema.name = names->name;
        insert(schema);

        Value v1; v1.ival = get_size(&schema);
        c_temp = makecode1(op, v1);
        code_ret = (code_ret.head == NULL ? c_temp : appcode(code_ret, c_temp));
    }

    return code_ret;
}

Pname id_list(Pnode p, int* quanti)
{
    if (p != NULL)
    {
        Pname lista = (Pname)newmem(sizeof(Name));
        lista->name = p->value.sval;

        (*quanti)++;
        lista->next = id_list(p->brother, quanti);
        return lista;
    }
    else return NULL;
}


Code assign_stat(Pnode p)
{
    Psymbol symbol;
    Code exprcode;
    Pschema exprschema;
/*
    assign_stat
        /
       /
      ID ---> expr
 */

    // Semantica: Carico gli schemi di ID e expr
    symbol = lookup(valname(p->child));
    if (symbol == NULL)
        semerror(p->child, "Undefined identifier in assignment");
    exprschema = clone_schema(&symbol->schema);
    exprcode = expr(p->child->brother, exprschema);

    // Type checking:
    if (!type_equal(symbol->schema, *exprschema))
        semerror(p->child->brother, "Incompatible types in assignment");

    Value v1; v1.ival = symbol->oid;
    return concode(
        exprcode,
        makecode1(T_STO, v1),
        endcode());
}

Code if_stat(Pnode p)
{
    /*
        if
        /
       /
      expr ---> stat_list [---> stat_list]
     */
    Schema exprschema;
    Code exprcode = expr(p->child, &exprschema);
    Code bodythen = stat_list(p->child->brother);

    // Vincoli semantici
    if (exprschema.type != BOOLEAN)
        semerror(p->child, "Boolean expression required");

    if (p->child->brother->brother == NULL)
    {
        // IF-THEN
        Value v1; v1.ival = bodythen.size + 1;
        return concode(exprcode,
                       makecode1(T_SKIPF, v1),
                       bodythen,
                       endcode());
    }
    else
    {
        // IF-THEN-ELSE
        Code bodyelse = stat_list(p->child->brother->brother);
        Value v1; v1.ival = bodythen.size + 2;
        Value v2; v2.ival = bodyelse.size + 1;
        return concode(exprcode,
                       makecode1(T_SKIPF, v1),
                       bodythen,
                       makecode1(T_SKIP, v2),
                       bodyelse,
                       endcode()
            );
    }
}

Code read_stat(Pnode p)
{
    // TODO
    return endcode();
}

Code specifier(Pnode p)
{
    // TODO
    return endcode();
}

Code stat(Pnode p)
{
    // TODO
    switch (p->type)
    {
        case N_DEF_STAT:
            return def_stat(p);
        case N_ASSIGN_STAT:
            return assign_stat(p);
        case N_IF_STAT:
            return if_stat(p);
        case N_WHILE_STAT:
            return while_stat(p);
        case N_READ_STAT:
            return read_stat(p);
        case N_WRITE_STAT:
            return write_stat(p);
        default:
            noderror(p);
    }
    return endcode();
}

Code tuple_const(Pnode p, Pschema s)
{
    // TODO
    return endcode();
}

Code while_stat(Pnode p)
{
    /*
        while
        /
       /
      expr ---> stat_list
     */
    Schema exprschema;
    Code exprcode = expr(p->child, &exprschema);
    Code body = stat_list(p->child->brother);

    // Vincoli semantici
    if (exprschema.type != BOOLEAN)
        semerror(p->child, "Boolean expression required");

    Value v1; v1.ival = body.size + 2;
    Value v2; v2.ival = -(exprcode.size + body.size + 1);
    return concode(exprcode,
                   makecode1(T_SKIPF, v1),
                   body,
                   makecode1(T_SKIP, v2),
                   endcode());
}

Code write_stat(Pnode p)
{
    /*
         WRITE_STAT
            /
           /
        N_SPECIFIER --> expr
             /
            /
        [expr-filename]
     */
    Value format;
    int op;
    Schema exprschema, specifierschema;
    Code exprcode, specifiercode;

    exprcode = expr(p->child->brother, &exprschema);
    if (p->child->child != NULL)
    {
        // Con specifier
        specifiercode = expr(p->child->child, &specifierschema);
        // Vincoli semantici
        if (specifierschema.type != STRING)
            semerror(p->child->child, "String type required for specifier");
        exprcode = appcode(exprcode, specifiercode);
        op = T_FPRINT;
    }
    else
    {
        // Senza specifier
        op = T_PRINT;
    }
    format.sval = get_format(exprschema);
    return concode(
        exprcode,
        makecode1(op, format),
        endcode()
        );
}
