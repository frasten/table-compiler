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

Code stat_list(Pnode p) {
	// TODO
	return endcode();
}
// TODO
