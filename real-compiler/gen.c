#include "parser.h"
#include "def.h"

static Opdescr tabop[] =
{
        {T_TCODE,   "TCODE",     0,  "i"  },
        {T_NEWATOM, "NEWATOM",   0,  "i"  },
        {T_NEWTAB,  "NEWTAB",    0,  "i"  },
        {T_POP,     "POP",       0,  "i"  },
        {T_LDINT,   "LDINT",     0,  "i"  },
        {T_LDSTR,   "LDSTR",     0,  "s"  },
        {T_LDTAB,   "LDTAB",     1,  "ii" },
        {T_IATTR,   "IATTR",     0,  "i"  },
        {T_SATTR,   "SATTR",     0,  "s"  },
        {T_ENDTAB,  "ENDTAB",   -1,  ""   },
        {T_LOB,     "LOB",       0,  "i"  },
        {T_LAT,     "LAT",       0,  "iii"},
        {T_STO,     "STO",       0,  "i"  },
        {T_SKIP,    "SKIP",      0,  "i"  },
        {T_SKIPF,   "SKIPF",     0,  "i"  },
        {T_EQU,     "EQU",       0,  ""   },
        {T_NEQ,     "NEQ",       0,  ""   },
        {T_IGT,     "IGT",       0,  ""   },
        {T_IGE,     "IGE",       0,  ""   },
        {T_ILT,     "ILT",       0,  ""   },
        {T_ILE,     "ILE",       0,  ""   },
        {T_SGT,     "SGT",       0,  ""   },
        {T_SGE,     "SGE",       0,  ""   },
        {T_SLT,     "SLT",       0,  ""   },
        {T_SLE,     "SLE",       0,  ""   },
        {T_PLUS,    "PLUS",      0,  ""   },
        {T_MINUS,   "MINUS",     0,  ""   },
        {T_TIMES,   "TIMES",     0,  ""   },
        {T_DIV,     "DIV",       0,  ""   },
        {T_JOIN,    "JOIN",      1,  "i"  },
        {T_ENDJOIN, "ENDJOIN",  -1,  "i"  },
        {T_UMI,     "UMI",       0,  ""   },
        {T_NEG,     "NEG",       0,  ""   },
        {T_PROJ,    "PROJ",      1,  "i"  },
        {T_ATTR,    "ATTR",      0,  "ii" },
        {T_ENDPROJ, "ENDPROJ",  -1,  ""   },
        {T_SEL,     "SEL",       1,  "i"  },
        {T_ENDSEL,  "ENDSEL",   -1,  "i"  },
        {T_EXS,     "EXS",       1,  "i"  },
        {T_ENDEXS,  "ENDEXS",   -1,  "i"  },
        {T_ALL,     "ALL",       1,  "i"  },
        {T_ENDALL,  "ENDALL",   -1,  "i"  },
        {T_UPD,     "UPD",       1,  "iii"},
        {T_ENDUPD,  "ENDUPD",   -1,  "i"  },
        {T_REMDUP,  "REMDUP",    0,  ""   },
        {T_EXT,     "EXT",       1,  "ii" },
        {T_ENDEXT,  "ENDEXT",   -1,  "i"  },
        {T_GET,     "GET",       0,  "is" },
        {T_FGET,    "FGET",      0,  "is" },
        {T_PRINT,   "PRINT",     0,  "s"  },
        {T_FPRINT,  "FPRINT",    0,  "s"  },
        {T_HALT,    "HALT",      0,  ""   }
};

#define TOTOP (sizeof(tabop)/sizeof(Opdescr))

Opdescr *get_descr(Operator op)
{
    unsigned int i;
    for (i = 0; i < TOTOP; i++)
    {
        if (tabop[i].op == op)
            return(&tabop[i]);
    }
    syserror("get_descr()");
    return NULL;
}

char *nameop(Operator op)
{
    unsigned int i;

    for (i = 0; i < TOTOP; i++)
    {
        if (tabop[i].op == op)
            return(tabop[i].name);
    }
    syserror("nameop()");
    return NULL;
}
 
void relocate_address(Code code, int offset)
{
    Tstat *pt = code.head;
    int i;

    for (i = 1; i <= code.size; i++)
    {
        pt->address += offset;
        pt = pt->next;
    }
}
 
Code appcode(Code code1, Code code2)
{
    Code rescode;

    if (code1.head == NULL)
        return (code2);
    else if (code2.head == NULL)
        return (code1);
    relocate_address(code2, code1.size);
    rescode.head = code1.head;
    rescode.tail = code2.tail;
    code1.tail->next = code2.head;
    rescode.size = code1.size + code2.size;
    return(rescode);
}

Code endcode()
{
    static Code code = {NULL, 0, NULL};

    return code;
}

Code concode(Code code1, Code code2, ...)
{
    Code rescode = code1, *pcode = &code2;

    while (pcode->head != NULL)
    {
        rescode = appcode(rescode, *pcode);
        pcode++;
    }
    return rescode;
}

Tstat *newstat(Operator op)
{
    Tstat *pstat;

    pstat = (Tstat*) newmem(sizeof(Tstat));
    pstat->address = 0;
    pstat->op = op;
    pstat->next = NULL;
    return pstat;
}

Code makecode(Operator op)
{
    Code code;

    code.head = code.tail = newstat(op);
    code.size = 1;
    return code;
}

Code makecode1(Operator op, Value arg)
{
    Code code;

    code = makecode(op);
    code.head->args[0] = arg;
    return code;
}

Code makecode2(Operator op, Value arg1, Value arg2)
{
    Code code;

    code = makecode1(op, arg1);
    code.head->args[1] = arg2;
    return code;
}

Code makecode3(Operator op, Value arg1, Value arg2, Value arg3)
{
    Code code;

    code = makecode2(op, arg1, arg2);
    code.head->args[2] = arg3;
    return code;
}

Code make_ldint(int i)
{
    Code code;

    code = makecode(T_LDINT);
    Value v;
    v.ival = i;
    code.head->args[0] = v;
    return code;
}

Code make_ldstr(char *s)
{
    Code code;

    code = makecode(T_LDSTR);
    Value v;
    v.sval = s;
    code.head->args[0] = v;
    return code;
}

Code make_sattr(char *s)
{
    Code code;

    code = makecode(T_SATTR);
    Value v;
    v.sval = s;
    code.head->args[0] = v;
    return code;
}

Code make_get_fget(Operator op, int oid, char *format)
{
    Code code;

    Value v1; v1.ival = oid;
    Value v2; v2.sval = format;
    code = makecode1(op, v1);
    code.head->args[1] = v2;
    return code;
}

Code make_print_fprint(Operator op, char *format)
{
    Code code;

    code = makecode(op);
    Value v;
    v.sval = format;
    code.head->args[0] = v;
    return code;
}
