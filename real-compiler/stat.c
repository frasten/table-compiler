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
	switch (p->child->type)
	{
		// TODO: boolean c; lo fa crashare
		case N_ATOMIC_TYPE: {
			int size = 0;
		    switch (p->child->value.ival)
		    {
		        case INTEGER:
		        case BOOLEAN:
		            size = sizeof(int);
		        case STRING:
		            size = sizeof(char *);
			}

			c_ret.head = NULL;
			// Genero il codice per allocare ognuna delle variabili
			for (p = p->child->brother; p != NULL; p = p->brother) {
				c_temp = makecode1(T_NEWATOM, size);
				c_ret = (c_ret.head == NULL ? c_temp : appcode(c_ret, c_temp));
			}
			return c_ret;
		}
		case N_TABLE_TYPE:
			// TODO
			break;
	}
	return endcode();
}

Code assign_stat(Pnode p)
{
	// TODO
	return endcode();
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
			printf("DEF_STAT\n");
			return def_stat(p);
		case N_ASSIGN_STAT:
			printf("ASSIGN_STAT\n");
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
			semerror(p, "Unexpected error.");
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
