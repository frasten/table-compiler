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
  
  return concode(makecode1(T_TCODE, body.size + 2),
		 body,
		 makecode(T_HALT),
		 endcode());
}

Code stat_list(Pnode p)
{
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
	s = appcode(s, makecode1(T_POP, numobj_in_current_env()));
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
	Code c_temp, c_ret;
    int num_id = 0;
    Pname names;
    Pnode nodo_type = p->child;

	switch (nodo_type->type)
	{
		case N_ATOMIC_TYPE:
		{
			int size = 0;
		    switch (p->child->value.ival)
		    {
		        case INTEGER:
		        case BOOLEAN:
		            size = sizeof(int);
		        case STRING:
		            size = sizeof(char *);
			}

			names = id_list(p->child->brother, &num_id);

			// Genero il codice per allocare ognuna delle variabili
			c_ret.head = NULL;
			for (; names != NULL; names = names->next)
			{
				Pschema schema;
				c_temp = makecode1(T_NEWATOM, size);
				c_ret = (c_ret.head == NULL ? c_temp : appcode(c_ret, c_temp));

				// Aggiungo il nome alla Symbol Table
			    schema = atomic_type(nodo_type);
			    schema->name = names->name;
			    insert(*schema);
			}
			return c_ret;
		}
		case N_TABLE_TYPE:
			// TODO
			break;
		default:
			noderror(p->child);
	}
	return endcode();
}

Pname id_list(Pnode p, int* quanti)
{
	if (p != NULL)
	{
		Pname lista = (Pname)newmem(sizeof(Name));
		lista->name = p->value.sval;

		// Controllo definizione, inserisco nell'environment
		if (name_in_environment(lista->name))
	    	semerror(p, "Variable redeclaration");
	    // Aggiungo la nuova variabile nell'environment
	    insert_name_into_environment(lista->name);

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
    if (!name_in_environment(p->child->value.sval))
    	semerror(p->child, "Undefined identifier in assignment");

    // Carico gli schemi di ID e expr
    symbol = lookup(p->child->value.sval);
	exprschema = clone_schema(&symbol->schema);
    exprcode = expr(p->child->brother, exprschema);

    // Type checking:
    if (!type_equal(symbol->schema, *exprschema))
    	semerror(p->child->brother, "Incompatible types in assignment");

	return concode(
		exprcode,
		makecode1(T_STO, symbol->oid),
		endcode());
}

Code attr_code(Pnode p)
{
	// TODO
	return endcode();
}

Code if_stat(Pnode p)
{
	// TODO
	return endcode();
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
			printf("IF_STAT\n");
			return if_stat(p);
		case N_WHILE_STAT:
			printf("WHILE_STAT\n");
			return while_stat(p);
		case N_READ_STAT:
			printf("READ_STAT\n");
			return read_stat(p);
		case N_WRITE_STAT:
			printf("WRITE_STAT\n");
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
	// TODO
	return endcode();
}

Code write_stat(Pnode p)
{
	// TODO
	return endcode();
}
