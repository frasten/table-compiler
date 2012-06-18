#include "def.h"

extern int yydebug;
extern Pnode root;

int main(int argc, char *argv[])
{
    Code code;
    
    if(argc > 1) yydebug = 1;
    init_compiler();
    yyparse();
    code = program(root);
    codeprint(code, 0);
}

void init_compiler()
{
    init_lextab();
    init_symtab();
}
