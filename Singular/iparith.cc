/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
/* $Id: iparith.cc,v 1.16 1997-04-08 08:43:16 obachman Exp $ */
/*
* ABSTRACT: table driven kernel interface, used by interpreter
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdio.h>
#include <time.h>

#ifndef macintosh
#include <sys/time.h>
#include <unistd.h>
#endif

#include "mod2.h"
#include "tok.h"
#include "ipid.h"
#include "intvec.h"
#include "mmemory.h"
#include "febase.h"
#include "longalg.h"
#include "polys.h"
#include "ideals.h"
#include "matpol.h"
#include "kstd1.h"
#include "timer.h"
#include "ring.h"
#include "subexpr.h"
#include "lists.h"
#include "numbers.h"
#include "stairc.h"
#include "maps.h"
#include "syz.h"
#include "weight.h"
#include "ipconv.h"
#include "ipprint.h"
#include "attrib.h"
#include "silink.h"
#ifdef HAVE_LIBFACTORY
#include "clapsing.h"
#include "kstdfac.h"
#endif
#ifdef HAVE_FGLM
#include "fglm.h"
#endif
#include "ipshell.h"

/*=============== types =====================*/
struct sValCmdTab
{
  short cmd;
  short start;
};

typedef sValCmdTab jjValCmdTab[];

/* ifdef GENTABLE: definitions are in ipshell.h */
#ifndef GENTABLE
typedef char * (*Proc1)(char *);
struct sValCmd1
{
  proc1 p;
  short cmd;
  short res;
  short arg;
};

typedef BOOLEAN (*proc2)(leftv,leftv,leftv);
struct sValCmd2
{
  proc2 p;
  short cmd;
  short res;
  short arg1;
  short arg2;
};

typedef BOOLEAN (*proc3)(leftv,leftv,leftv,leftv);
struct sValCmd3
{
  proc3 p;
  short cmd;
  short res;
  short arg1;
  short arg2;
  short arg3;
};
struct sValCmdM
{
  proc1 p;
  short cmd;
  short res;
  short number_of_args; /* -1: any, -2: any >0, .. */
};
#endif


/*============= proc =======================*/
static int iiTabIndex(const jjValCmdTab dArithTab, const int len, const int op);
#ifdef MDEBUG
#define jjMakeSub(A) jjDBMakeSub(A,__FILE__,__LINE__)
static Subexpr jjDBMakeSub(leftv e,char *f, int l);
#else
static Subexpr jjMakeSub(leftv e);
#endif


/*============= vars ======================*/
extern int cmdtok;
extern BOOLEAN expected_parms;

int iiOp; /* the current operation*/

#ifdef GENTABLE
cmdnames cmds[] =
{  // name-string alias tokval          toktype
  { "$INVALID$",   0, -1,                 0},
  { "and",         0, '&' ,               '&'},
  { "attrib",      0, ATTRIB_CMD ,        CMD_123},
  { "bareiss",     0, BAREISS_CMD ,       CMD_1},
  { "betti",       0, BETTI_CMD ,         CMD_1},
  #ifdef HAVE_DLD
  { "binary",      0, BINARY_CMD ,        ROOT_DECL},
  #endif
  { "break",       0, BREAK_CMD ,         BREAK_CMD},
  { "char",        0, CHARACTERISTIC_CMD ,CMD_1},
  { "char_series", 0, CHAR_SERIES_CMD ,   CMD_1},
  { "charstr",     0, CHARSTR_CMD ,       CMD_1},
  { "close",       0, CLOSE_CMD ,         CMD_1},
  { "coef",        0, COEF_CMD ,          CMD_M},
  { "coeffs",      0, COEFFS_CMD ,        CMD_23},
  { "continue",    0, CONTINUE_CMD ,      CONTINUE_CMD},
  { "content",     0, CONTENT_CMD ,       CMD_1},
  { "contract",    0, CONTRACT_CMD ,      CMD_2},
  { "dbprint",     0, DBPRINT_CMD ,       CMD_M},
  { "def",         0, DEF_CMD ,           ROOT_DECL},
  { "defined",     0, DEFINED_CMD ,       CMD_1},
  { "deg",         0, DEG_CMD ,           CMD_12},
  { "degree",      0, DEGREE_CMD ,        CMD_1},
  { "delete",      0, DELETE_CMD ,        CMD_2},
  { "det",         0, DET_CMD ,           CMD_1},
  { "diff",        0, DIFF_CMD ,          CMD_2},
  { "dim",         0, DIM_CMD ,           CMD_1},
#ifdef DRING
  { "dring",       0, DRING_CMD ,         DRING_CMD},
#endif
  { "dump",        0, DUMP_CMD,           CMD_1},
  { "EXTGCD",      0, EXTGCD_CMD ,        CMD_2},
  { "eliminate",   0, ELIMINATION_CMD,    CMD_23},
  { "else",        0, ELSE_CMD ,          ELSE_CMD},
  { "eval",        0, EVAL ,              EVAL},
  { "execute",     0, EXECUTE_CMD ,       EXECUTE_CMD},
  { "exit",        0, EXIT_CMD ,          EXIT_CMD},
  { "export",      0, EXPORT_CMD ,        EXPORT_CMD},
  { "factorize",   0, FAC_CMD ,           CMD_12},
  { "fetch",       0, FETCH_CMD ,         CMD_2},
  { "fglm",        0, FGLM_CMD ,          CMD_2},
  { "find",        0, FIND_CMD ,          CMD_23},
  { "forif",       0, IF_CMD ,            IF_CMD},
  { "freemodule",  0, FREEMODULE_CMD ,    CMD_1},
  { "facstd",      0, FACSTD_CMD ,        CMD_12},
  { "gen",         0, E_CMD ,             CMD_1},
  { "getdump",     0, GETDUMP_CMD,        CMD_1},
  { "GCD",         0, GCD_CMD ,           CMD_2},
  { "hilb",        0, HILBERT_CMD ,       CMD_12},
  { "homog",       0, HOMOG_CMD ,         CMD_12},
  { "ideal",       0, IDEAL_CMD ,         IDEAL_CMD},
  { "if",          0, IF_CMD ,            IF_CMD},
  { "imap",        0, IMAP_CMD ,          CMD_2},
  { "indepSet",    0, INDEPSET_CMD ,      CMD_1},
  { "insert",      0, INSERT_CMD ,        CMD_23},
  { "int",         0, INT_CMD ,           ROOT_DECL},
  { "interred",    0, INTERRED_CMD ,      CMD_1},
  { "intersect",   0, INTERSECT_CMD ,     CMD_M},
  { "intmat",      0, INTMAT_CMD ,        INTMAT_CMD},
  { "intvec",      0, INTVEC_CMD ,        ROOT_DECL_LIST},
  { "jacob",       0, JACOB_CMD ,         CMD_1},
  { "jet",         0, JET_CMD ,           CMD_23},
  { "kbase",       0, KBASE_CMD ,         CMD_1},
  { "keepring",    0, KEEPRING_CMD ,      KEEPRING_CMD},
  { "kill",        0, KILL_CMD ,          KILL_CMD},
  { "killattrib",  0, KILLATTR_CMD ,      CMD_12},
  { "koszul",      0, KOSZUL_CMD ,        CMD_23},
  { "lead",        0, LEAD_CMD ,          CMD_1},
  { "leadcoef",    0, LEADCOEF_CMD ,      CMD_1},
  { "leadexp",     0, LEADEXP_CMD ,       CMD_1},
  { "LIB",         0, LIB_CMD ,           LIB_CMD},
  { "lift",        0, LIFT_CMD ,          CMD_2},
  { "liftstd",     0, LIFTSTD_CMD ,       CMD_2},
  { "link",        0, LINK_CMD ,          ROOT_DECL},
  { "listvar",     0, LISTVAR_CMD ,       LISTVAR_CMD},
  { "list",        0, LIST_CMD ,          ROOT_DECL_LIST},
  { "map",         0, MAP_CMD ,           RING_DECL},
  { "matrix",      0, MATRIX_CMD ,        MATRIX_CMD},
  { "maxideal",    0, MAXID_CMD ,         CMD_1},
  { "memory",      0, MEMORY_CMD ,        CMD_1},
  { "minbase",     0, MINBASE_CMD ,       CMD_1},
  { "minor",       0, MINOR_CMD ,         CMD_2},
  { "minres",      0, MINRES_CMD ,        CMD_1},
  { "module",      0, MODUL_CMD ,         MODUL_CMD},
  { "modulo",      0, MODULO_CMD ,        CMD_2},
  { "monitor",     0, MONITOR_CMD ,       CMD_12},
  { "mult",        0, MULTIPLICITY_CMD ,  CMD_1},
  { "mres",        0, MRES_CMD ,          CMD_23},
  { "mstd",        0, MSTD_CMD ,          CMD_1},
  { "nameof",      0, NAMEOF_CMD ,        CMD_1},
  { "names",       0, NAMES_CMD ,         CMD_M},
  { "ncols",       0, COLS_CMD ,          CMD_1},
  { "not",         0, NOT ,               NOT},
  { "npars",       0, NPARS_CMD ,         CMD_1},
  { "nrows",       0, ROWS_CMD ,          CMD_1},
  { "number",      0, NUMBER_CMD ,        RING_DECL},
  { "nvars",       0, NVARS_CMD ,         CMD_1},
  { "open",        0, OPEN_CMD ,          CMD_1},
  { "option",      0, OPTION_CMD ,        CMD_M},
  { "or",          0, '|' ,               '|'},
  { "ord",         0, ORD_CMD ,           CMD_1},
  { "ordstr",      0, ORDSTR_CMD ,        CMD_1},
  { "package",     0, PACKAGE_CMD ,       ROOT_DECL},
  { "par",         0, PAR_CMD ,           CMD_1},
  { "parameter",   0, PARAMETER ,         PARAMETER},
  { "pardeg",      0, PARDEG_CMD ,        CMD_1},
  { "parstr",      0, PARSTR_CMD ,        CMD_12},
  { "poly",        0, POLY_CMD ,          RING_DECL},
  { "preimage",    0, PREIMAGE_CMD ,      CMD_3},
  { "prime",       0, PRIME_CMD ,         CMD_1},
  { "print",       0, PRINT_CMD ,         CMD_12},
  { "prune",       0, PRUNE_CMD ,         CMD_1},
  { "proc",        0, PROC_CMD ,          PROC_CMD},
  { "qring",       0, QRING_CMD ,         ROOT_DECL},
  { "quotient",    0, QUOTIENT_CMD ,      CMD_2},
  { "quote",       0, QUOTE ,             QUOTE},
  { "qhweight",    0, QHWEIGHT_CMD ,      CMD_1},
  { "random",      0, RANDOM_CMD ,        CMD_23},
  { "read",        0, READ_CMD ,          CMD_12},
  { "reduce",      0, REDUCE_CMD ,        CMD_23},
  { "regularity",  0, REGULARITY_CMD ,    CMD_1},
  { "res",         0, RES_CMD ,           CMD_23},
  { "reservedName",0, RESERVEDNAME_CMD ,  CMD_M},
  { "resultant",   0, RESULTANT_CMD,      CMD_3},
  { "return",      0, RETURN ,            RETURN},
#ifdef SIC
  { "RETURN",      0, END_PROC ,          RETURN},
#else
  { "RETURN",      1, RETURN ,            RETURN},
#endif
  { "ring",        0, RING_CMD ,          RING_CMD},
  { "rvar",        0, IS_RINGVAR ,        CMD_1},
  { "setring",     0, SETRING_CMD ,       SETRING_CMD},
  { "simplify",    0, SIMPLIFY_CMD ,      CMD_2},
  { "size",        0, COUNT_CMD ,         CMD_1},
  { "sortvec",     0, SORTVEC_CMD ,       CMD_1},
  { "sres",        0, SRES_CMD ,          CMD_23},
  { "status",      0, STATUS_CMD,         CMD_23},
  { "std",         0, STD_CMD ,           CMD_12},
  { "string",      0, STRING_CMD ,        ROOT_DECL_LIST},
  { "subst",       0, SUBST_CMD ,         CMD_3},
  { "system",      0, SYSTEM_CMD,         CMD_M},
  { "syz",         0, SYZYGY_CMD ,        CMD_1},
  { "test",        0, TEST_CMD ,          CMD_M},
  { "trace",       0, TRACE_CMD ,         CMD_1},
  { "transpose",   0, TRANSPOSE_CMD ,     CMD_1},
  { "type",        0, TYPE_CMD ,          TYPE_CMD},
  { "typeof",      0, TYPEOF_CMD ,        CMD_1},
  { "var",         0, VAR_CMD ,           CMD_1},
  { "varstr",      0, VARSTR_CMD ,        CMD_12},
  { "vdim",        0, VDIM_CMD ,          CMD_1},
  { "vector",      0, VECTOR_CMD ,        RING_DECL},
  { "verbose",     0, VERBOSE_CMD ,       CMD_M},
  { "wedge",       0, WEDGE_CMD ,         CMD_2},
  { "weight",      0, WEIGHT_CMD ,        CMD_1},
  { "whileif",     0, IF_CMD ,            IF_CMD},
  { "write",       0, WRITE_CMD ,         CMD_M},
/* delete for next version:*/
  { "IN",          1, LEAD_CMD ,          CMD_1},
  { "NF",          1, REDUCE_CMD ,        CMD_23},
  { "multiplicity",1, MULTIPLICITY_CMD ,  CMD_1},
//  { "rank",        1, ROWS_CMD ,          CMD_1},

/* set sys vars*/
#ifdef SRING
  { "alternating", 0, VALTVARS ,          SYSVAR},
#endif
  { "degBound",    0, VMAXDEG ,           SYSVAR},
  { "echo",        0, VECHO ,             SYSVAR},
  { "minpoly",     0, VMINPOLY ,          SYSVAR},
  { "multBound",   0, VMAXMULT ,          SYSVAR},
  { "noether",     0, VNOETHER ,          SYSVAR},
  { "pagelength",  0, VPAGELENGTH ,       SYSVAR},
  { "pagewidth",   0, VCOLMAX ,           SYSVAR},
  { "printlevel",  0, VPRINTLEVEL ,       SYSVAR},
  { "short",       0, VSHORTOUT ,         SYSVAR},
  { "timer",       0, VTIMER ,            SYSVAR},
#ifdef HAVE_RTIMER
  { "rtimer",      0, VRTIMER,            SYSVAR},
#endif
  { "TRACE",       0, TRACE ,             SYSVAR},
  { "voice",       0, VOICE ,             SYSVAR},

/* other reserved words:scanner.l */
  { "pause",       0, -1 ,             0},
  { "while",       0, -1 ,             0},
  { "for",         0, -1 ,             0},
  { "help",        0, -1 ,             0},
  { "example",     0, -1 ,             0},
  { "newline",     0, -1 ,             0},
  { "exitall",     0, -1 ,             0},
  { "quit",        0, -1 ,             0},
/* end of list marker */
  { NULL, 0, 0, 0}
};
#endif

/*=================== operations with 2 args.: static proc =================*/
static BOOLEAN jjOP_IV_I(leftv res, leftv u, leftv v)
{
  intvec* aa= (intvec *)u->CopyD(INTVEC_CMD);
  int bb = (int)(v->Data());
  if (errorreported) return TRUE;
  switch (iiOp)
  {
    case '+': (*aa) += bb; break;
    case '-': (*aa) -= bb; break;
    case '*': (*aa) *= bb; break;
    case '/': (*aa) /= bb; break;
    case '%': (*aa) %= bb; break;
  }
  res->data=(char *)aa;
  return FALSE;
}
static BOOLEAN jjOP_I_IV(leftv res, leftv u, leftv v)
{
  return jjOP_IV_I(res,v,u);
}
static BOOLEAN jjOP_IM_I(leftv res, leftv u, leftv v)
{
  intvec* aa= (intvec *)u->CopyD(INTVEC_CMD);
  int bb = (int)(v->Data());
  int i=min(aa->rows(),aa->cols());
  switch (iiOp)
  {
    case '+': for (;i>0;i--) IMATELEM(*aa,i,i) += bb;
              break;
    case '-': for (;i>0;i--) IMATELEM(*aa,i,i) -= bb;
              break;
  }
  res->data=(char *)aa;
  return FALSE;
}
static BOOLEAN jjOP_I_IM(leftv res, leftv u, leftv v)
{
  return jjOP_IM_I(res,v,u);
}
static BOOLEAN jjDOTDOT(leftv res, leftv u, leftv v)
{
  res->data=(char *)new intvec((int)u->Data(),(int)v->Data());
  return FALSE;
}
static void jjEQUAL_REST(leftv res,leftv u,leftv v)
{
  if ((res->data) && (u->next!=NULL) && (v->next!=NULL))
  {
    int save_iiOp=iiOp;
    if (iiOp==NOTEQUAL)
      iiExprArith2(res,u->next,EQUAL_EQUAL,v->next);
    else
      iiExprArith2(res,u->next,iiOp,v->next);
    iiOp=save_iiOp;
  }
  if (iiOp==NOTEQUAL) res->data=(char *)(!(int)res->data);
}
static BOOLEAN jjCOMPARE_IV(leftv res, leftv u, leftv v)
{
  intvec*    a = (intvec * )(u->Data());
  intvec*    b = (intvec * )(v->Data());
  int r=a->compare(b);
  switch  (iiOp)
  {
    case '<':
      res->data  = (char *) (r<0);
      break;
    case '>':
      res->data  = (char *) (r>0);
      break;
    case LE:
      res->data  = (char *) (r<=0);
      break;
    case GE:
      res->data  = (char *) (r>=0);
      break;
    case EQUAL_EQUAL:
    case NOTEQUAL: /* negation handled by jjEQUAL_REST */
      res->data  = (char *) (r==0);
      break;
  }
  jjEQUAL_REST(res,u,v);
  return r==-2;
}
static BOOLEAN jjCOMPARE_IV_I(leftv res, leftv u, leftv v)
{
  intvec* a = (intvec * )(u->Data());
  int     b = (int )     (v->Data());
  int r=a->compare(b);
  switch  (iiOp)
  {
    case '<':
      res->data  = (char *) (r<0);
      break;
    case '>':
      res->data  = (char *) (r>0);
      break;
    case LE:
      res->data  = (char *) (r<=0);
      break;
    case GE:
      res->data  = (char *) (r>=0);
      break;
    case EQUAL_EQUAL:
    case NOTEQUAL: /* negation handled by jjEQUAL_REST */
      res->data  = (char *) (r==0);
      break;
  }
  jjEQUAL_REST(res,u,v);
  return FALSE;
}
static BOOLEAN jjCOMPARE_P(leftv res, leftv u, leftv v)
{
  poly p=(poly)u->Data();
  poly q=(poly)v->Data();
  int r=pComp(p,q);
  if (r==0)
  {
    /* compare lead coeffs */
    number h=nSub(pGetCoeff(p),pGetCoeff(q));
    r = -1+nIsZero(h)+2*nGreaterZero(h); /* -1: <, 0:==, 1: > */
    nDelete(&h);
  }
  else if (p==NULL)
  {
    if (q==NULL)
    {
      /* compare 0, 0 */
      r=0;
    }
    else if(pIsConstant(q))
    {
      /* compare 0, const */
      r = 1-2*nGreaterZero(pGetCoeff(q)); /* -1: <, 1: > */
    }
  }
  else if (q==NULL)
  {
    if (pIsConstant(p))
    {
      /* compare const, 0 */
      r = -1+2*nGreaterZero(pGetCoeff(p)); /* -1: <, 1: > */
    }
  }
  switch  (iiOp)
  {
    case '<':
      res->data  = (char *) (r < 0);
      break;
    case '>':
      res->data  = (char *) (r > 0);
      break;
    case LE:
      res->data  = (char *) (r <= 0);
      break;
    case GE:
      res->data  = (char *) (r >= 0);
      break;
    //case EQUAL_EQUAL:
    //case NOTEQUAL: /* negation handled by jjEQUAL_REST */
    //  res->data  = (char *) (r == 0);
    //  break;
  }
  jjEQUAL_REST(res,u,v);
  return FALSE;
}
static BOOLEAN jjCOMPARE_S(leftv res, leftv u, leftv v)
{
  char*    a = (char * )(u->Data());
  char*    b = (char * )(v->Data());
  int result = strcmp(a,b);
  switch  (iiOp)
  {
    case '<':
      res->data  = (char *) (result  < 0);
      break;
    case '>':
      res->data  = (char *) (result  > 0);
      break;
    case LE:
      res->data  = (char *) (result  <= 0);
      break;
    case GE:
      res->data  = (char *) (result  >= 0);
      break;
    case EQUAL_EQUAL:
    case NOTEQUAL: /* negation handled by jjEQUAL_REST */
      res->data  = (char *) (result  == 0);
      break;
  }
  jjEQUAL_REST(res,u,v);
  return FALSE;
}
static BOOLEAN jjPOWER_I(leftv res, leftv u, leftv v)
{
  int b=(int)u->Data();
  int e=(int)v->Data();
  int rc = 1;
  BOOLEAN overflow=FALSE;
  if ((e >= 0)&&(b!=0))
  {
    int oldrc;
    while ((e--)!=0)
    {
      oldrc=rc;
      rc *= b;
      if (!overflow)
      {
        if(rc/b!=oldrc) overflow=TRUE;
      }
    }
  }
  if (overflow)
    Warn("int overflow(^), result may be wrong");
  res->data = (char *)rc;
  return FALSE;
}
static BOOLEAN jjPOWER_N(leftv res, leftv u, leftv v)
{
  int e=(int)v->Data();
  number n=(number)u->CopyD();
  if (e<0)
  {
    number m=nInvers(n);
    nDelete(&n);
    n=m;
    e=-e;
  }
  nPower(n,e,(number*)&res->data);
  nDelete(&n);
  return FALSE;
}
static BOOLEAN jjPOWER_P(leftv res, leftv u, leftv v)
{
  res->data = (char *)pPower((poly)u->CopyD(POLY_CMD),(int)v->Data());
  return FALSE;
}
static BOOLEAN jjPOWER_ID(leftv res, leftv u, leftv v)
{
  res->data = (char *)idPower((ideal)(u->Data()),(int)(v->Data()));
  return FALSE;
}
static BOOLEAN jjPLUSMINUS_Gen(leftv res, leftv u, leftv v)
{
  u=u->next;
  v=v->next;
  if (u==NULL)
  {
    if (v==NULL) return FALSE;      /* u==NULL, v==NULL */
    if (iiOp=='-')                  /* u==NULL, v<>NULL, iiOp=='-'*/
      return iiExprArith1(res->next,v,'-');
    loop                            /* u==NULL, v<>NULL, iiOp=='+' */
    {
      res->next = (leftv)Alloc0(sizeof(sleftv));
      res=res->next;
      res->data = v->CopyD();
      res->rtyp = v->Typ();
      v=v->next;
      if (v==NULL) return FALSE;
    }
  }
  if (v!=NULL)                     /* u<>NULL, v<>NULL */
  {
    res->next = (leftv)Alloc(sizeof(sleftv));
    return iiExprArith2(res->next,u,iiOp,v);
  }
  loop                             /* u<>NULL, v==NULL */
  {
    res->next = (leftv)Alloc0(sizeof(sleftv));
    res=res->next;
    res->data = u->CopyD();
    res->rtyp = u->Typ();
    u=u->next;
    if (u==NULL) return FALSE;
  }
}
static BOOLEAN jjPLUS_I(leftv res, leftv u, leftv v)
{
  unsigned int a=(unsigned int)u->Data();
  unsigned int b=(unsigned int)v->Data();
  unsigned int c=a+b;
  res->data = (char *)c;
  if (((Sy_bit(31)&a)==(Sy_bit(31)&b))&&((Sy_bit(31)&a)!=(Sy_bit(31)&c)))
  {
    Warn("int overflow(+), result may be wrong");
  }
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjPLUS_N(leftv res, leftv u, leftv v)
{
  res->data = (char *)(nAdd((number)u->Data(), (number)v->Data()));
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjPLUS_P(leftv res, leftv u, leftv v)
{
  res->data = (char *)(pAdd((poly)u->CopyD(POLY_CMD) , (poly)v->CopyD(POLY_CMD)));
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjPLUS_IV(leftv res, leftv u, leftv v)
{
  res->data = (char *)ivAdd((intvec*)(u->Data()), (intvec*)(v->Data()));
  if (res->data==NULL)
  {
     WerrorS("intmat size not compatible");
     return TRUE;
  }
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjPLUS_MA(leftv res, leftv u, leftv v)
{
  res->data = (char *)(mpAdd((matrix)u->Data() , (matrix)v->Data()));
  if (res->data==NULL)
  {
     WerrorS("matrix size not compatible");
     return TRUE;
  }
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjPLUS_MA_P(leftv res, leftv u, leftv v)
{
  matrix m=(matrix)u->Data();
  matrix p= mpInitP(m->nrows,m->ncols,(poly)(v->CopyD()));
  if (iiOp=='+')
    res->data = (char *)mpAdd(m , p);
  else
    res->data = (char *)mpSub(m , p);
  idDelete((ideal *)&p);
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjPLUS_P_MA(leftv res, leftv u, leftv v)
{
  return jjPLUS_MA_P(res,v,u);
}
static BOOLEAN jjPLUS_S(leftv res, leftv u, leftv v)
{
  char*    a = (char * )(u->Data());
  char*    b = (char * )(v->Data());
  char*    r = (char * )AllocL(strlen(a) + strlen(b) + 1);
  strcpy(r,a);
  strcat(r,b);
  res->data=r;
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjPLUS_ID(leftv res, leftv u, leftv v)
{
  res->data = (char *)idAdd((ideal)u->Data(),(ideal)v->Data());
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjMINUS_I(leftv res, leftv u, leftv v)
{
  unsigned int a=(unsigned int)u->Data();
  unsigned int b=(unsigned int)v->Data();
  unsigned int c=a-b;
  if (((Sy_bit(31)&a)!=(Sy_bit(31)&b))&&((Sy_bit(31)&a)!=(Sy_bit(31)&c)))
  {
    Warn("int overflow(-), result may be wrong");
  }
  res->data = (char *)c;
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjMINUS_N(leftv res, leftv u, leftv v)
{
  res->data = (char *)(nSub((number)u->Data(), (number)v->Data()));
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjMINUS_P(leftv res, leftv u, leftv v)
{
  res->data = (char *)(pSub((poly)u->CopyD(POLY_CMD) , (poly)v->CopyD(POLY_CMD)));
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjMINUS_IV(leftv res, leftv u, leftv v)
{
  res->data = (char *)ivSub((intvec*)(u->Data()), (intvec*)(v->Data()));
  if (res->data==NULL)
  {
     WerrorS("intmat size not compatible");
     return TRUE;
  }
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjMINUS_MA(leftv res, leftv u, leftv v)
{
  res->data = (char *)(mpSub((matrix)u->Data() , (matrix)v->Data()));
  if (res->data==NULL)
  {
     WerrorS("matrix size not compatible");
     return TRUE;
  }
  return jjPLUSMINUS_Gen(res,u,v);
}
static BOOLEAN jjTIMES_I(leftv res, leftv u, leftv v)
{
  int a=(int)u->Data();
  int b=(int)v->Data();
  int c=a * b;
  if ((b!=0) && (c/b !=a))
    Warn("int overflow(*), result may be wrong");
  res->data = (char *)c;
  if (u->Next()!=NULL)
  {
    u=u->next;
    res->next = (leftv)Alloc(sizeof(sleftv));
    return iiExprArith2(res->next,u,iiOp,v);
  }
  else if (v->Next()!=NULL)
  {
    v=v->next;
    res->next = (leftv)Alloc(sizeof(sleftv));
    return iiExprArith2(res->next,u,iiOp,v);
  }
  return FALSE;
}
static BOOLEAN jjTIMES_N(leftv res, leftv u, leftv v)
{
  number a;
  number b;

  if (v->next==NULL)
  {
    a=(number)u->Data();
    if (u->next==NULL)
    {
      res->data = (char *)(nMult( a, (number)v->Data()));
      return FALSE;
    }
    // u->next exists: copy v
    b=(number)v->Data();
    res->data = (char *)(nMult( a, b));
    res->next = (leftv)Alloc(sizeof(sleftv));
    res=res->next;
    return iiExprArith2(res,u->next,iiOp,v);
  }
  // v->next exists: copy u
  a=nCopy((number)u->Data());
  b=(number)v->CopyD();
  res->data = (char *)(nMult( a, b));
  res->next = (leftv)Alloc(sizeof(sleftv));
  return iiExprArith2(res->next,u,iiOp,v->next);
}
static BOOLEAN jjTIMES_P(leftv res, leftv u, leftv v)
{
  poly a;
  poly b;

  if (v->next==NULL)
  {
    a=(poly)u->CopyD();
    if (u->next==NULL)
    {
      b=(poly)v->CopyD();
      res->data = (char *)(pMult( a, b));
      return FALSE;
    }
    // u->next exists: copy v
    b=pCopy((poly)v->Data());
    res->data = (char *)(pMult( a, b));
    res->next = (leftv)Alloc(sizeof(sleftv));
    res=res->next;
    return iiExprArith2(res,u->next,iiOp,v);
  }
  // v->next exists: copy u
  a=pCopy((poly)u->Data());
  b=(poly)v->CopyD();
  res->data = (char *)(pMult( a, b));
  res->next = (leftv)Alloc(sizeof(sleftv));
  return iiExprArith2(res->next,u,iiOp,v->next);
}
static BOOLEAN jjTIMES_ID(leftv res, leftv u, leftv v)
{
  res->data = (char *)idMult((ideal)u->Data(),(ideal)v->Data());
  return FALSE;
}
static BOOLEAN jjTIMES_IV(leftv res, leftv u, leftv v)
{
  res->data = (char *)ivMult((intvec*)(u->Data()), (intvec*)(v->Data()));
  if (res->data==NULL)
  {
     WerrorS("intmat size not compatible");
     return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjTIMES_MA_P1(leftv res, leftv u, leftv v)
{
  poly p=(poly)v->CopyD(POLY_CMD);
  int r=pMaxComp(p);/* recompute the rank for the case ideal*vector*/
  ideal I= (ideal)mpMultP((matrix)u->CopyD(MATRIX_CMD),p);
  if (r>0) I->rank=r;
  res->data = (char *)I;
  return FALSE;
}
static BOOLEAN jjTIMES_MA_P2(leftv res, leftv u, leftv v)
{
  return jjTIMES_MA_P1(res,v,u);
}
static BOOLEAN jjTIMES_MA_N1(leftv res, leftv u, leftv v)
{
  number n=(number)v->CopyD(NUMBER_CMD);
  poly p=pOne();
  pSetCoeff(p,n);
  res->data = (char *)mpMultP((matrix)u->CopyD(MATRIX_CMD),p);
  return FALSE;
}
static BOOLEAN jjTIMES_MA_N2(leftv res, leftv u, leftv v)
{
  return jjTIMES_MA_N1(res,v,u);
}
static BOOLEAN jjTIMES_MA_I1(leftv res, leftv u, leftv v)
{
  res->data = (char *)mpMultI((matrix)u->CopyD(MATRIX_CMD),(int)v->Data());
  return FALSE;
}
static BOOLEAN jjTIMES_MA_I2(leftv res, leftv u, leftv v)
{
  return jjTIMES_MA_I1(res,v,u);
}
static BOOLEAN jjTIMES_MA(leftv res, leftv u, leftv v)
{
  res->data = (char *)mpMult((matrix)u->Data(),(matrix)v->Data());
  if (res->data==NULL)
  {
     WerrorS("matrix size not compatible");
     return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjGE_I(leftv res, leftv u, leftv v)
{
  res->data = (char *)((int)u->Data() >= (int)v->Data());
  return FALSE;
}
static BOOLEAN jjGE_N(leftv res, leftv u, leftv v)
{
  number h=nSub((number)u->Data(),(number)v->Data());
  res->data = (char *) (nGreaterZero(h)||(nIsZero(h)));
  nDelete(&h);
  return FALSE;
}
static BOOLEAN jjGT_I(leftv res, leftv u, leftv v)
{
  res->data = (char *)((int)u->Data()>(int)v->Data());
  return FALSE;
}
static BOOLEAN jjGT_N(leftv res, leftv u, leftv v)
{
  number h=nSub((number)u->Data(),(number)v->Data());
  res->data = (char *) (nGreaterZero(h)&&(!nIsZero(h)));
  nDelete(&h);
  return FALSE;
}
static BOOLEAN jjLE_I(leftv res, leftv u, leftv v)
{
  res->data = (char *)((int)u->Data() <= (int)v->Data());
  return FALSE;
}
static BOOLEAN jjLE_N(leftv res, leftv u, leftv v)
{
  number h=nSub((number)v->Data(),(number)u->Data());
  res->data = (char *) (nGreaterZero(h)||nIsZero(h));
  nDelete(&h);
  return FALSE;
}
static BOOLEAN jjLT_I(leftv res, leftv u, leftv v)
{
  res->data = (char *)((int)u->Data() < (int)v->Data());
  return FALSE;
}
static BOOLEAN jjLT_N(leftv res, leftv u, leftv v)
{
  number h=nSub((number)v->Data(),(number)u->Data());
  res->data = (char *) (nGreaterZero(h)&&(!nIsZero(h)));
  nDelete(&h);
  return FALSE;
}
static BOOLEAN jjDIV_I(leftv res, leftv u, leftv v)
{
  int b= (int) v->Data();
  if (b==0)
  {
    WerrorS("div. by 0");
    return TRUE;
  }
  res->data = (char *)((int)u->Data() / b);
  return FALSE;
}
static BOOLEAN jjDIV_N(leftv res, leftv u, leftv v)
{
  number q=(number)v->Data();
  if (nIsZero(q))
  {
    WerrorS("div. by 0");
    return TRUE;
  }
  q = nDiv((number)u->Data(),q);
  nNormalize(q);
  res->data = (char *)q;
  return FALSE;
}
static BOOLEAN jjDIV_P(leftv res, leftv u, leftv v)
{
  poly q=(poly)v->Data();
  if (q==NULL)
  {
    WerrorS("div. by 0");
    return TRUE;
  }
  if (pNext(q)!=NULL)
  {
#ifdef HAVE_LIBFACTORY
    res->data=(void*)(singclap_pdivide((poly)(u->Data()),
                                       (poly)(v->Data())));
#else
    WerrorS("division only by a monomial");
    return TRUE;
#endif
  }
  else
  {
    res->data = (char *)pDivideM((poly)u->CopyD(POLY_CMD),pHead(q));
  }
  return FALSE;
}
static BOOLEAN jjMOD_I(leftv res, leftv u, leftv v)
{
  int a=(int)u->Data();
  int b=(int)v->Data();
  if (errorreported) return TRUE;
  if (b==0)
  {
    WerrorS("div. by 0");
    return TRUE;
  }
  res->data = (char *)(a % b);
  return FALSE;
}
static BOOLEAN jjEQUAL_I(leftv res, leftv u, leftv v)
{
  res->data = (char *)((int)u->Data() == (int)v->Data());
  jjEQUAL_REST(res,u,v);
  return FALSE;
}
static BOOLEAN jjEQUAL_Ma(leftv res, leftv u, leftv v)
{
  res->data = (char *)mpEqual((matrix)u->Data(),(matrix)v->Data());
  jjEQUAL_REST(res,u,v);
  return FALSE;
}
static BOOLEAN jjEQUAL_N(leftv res, leftv u, leftv v)
{
  res->data = (char *)nEqual((number)u->Data(),(number)v->Data());
  jjEQUAL_REST(res,u,v);
  return FALSE;
}
static BOOLEAN jjEQUAL_P(leftv res, leftv u, leftv v)
{
  poly p=(poly)u->Data();
  poly q=(poly)v->Data();
  if (p==NULL)
  {
    res->data=(char *)(q==NULL);
  }
  else
  {
    if (q==NULL) res->data=(char *)FALSE;
    else
    {
      int r=pComp(p,q);
      if (r==0)
      {
        p=pSub(pCopy(p),pCopy(q));
        res->data = (char *) (p==NULL);
        pDelete(&p);
      }
      else
        res->data = (char *) FALSE;
    }
  }
  jjEQUAL_REST(res,u,v);
  return FALSE;
}
static BOOLEAN jjAND_I(leftv res, leftv u, leftv v)
{
  res->data = (char *)((int)u->Data() && (int)v->Data());
  return FALSE;
}
static BOOLEAN jjOR_I(leftv res, leftv u, leftv v)
{
  res->data = (char *)((int)u->Data() || (int)v->Data());
  return FALSE;
}
static BOOLEAN jjINDEX_I(leftv res, leftv u, leftv v)
{
  res->rtyp=u->rtyp; u->rtyp=0;
  res->data=u->data; u->data=NULL;
  res->name=u->name; u->name=NULL;
  res->e=u->e;       u->e=NULL;
  if (res->e==NULL) res->e=jjMakeSub(v);
  else
  {
    Subexpr sh=res->e;
    while (sh->next != NULL) sh=sh->next;
    sh->next=jjMakeSub(v);
  }
  return FALSE;
}
static BOOLEAN jjINDEX_IV(leftv res, leftv u, leftv v)
{
  if ((u->rtyp!=IDHDL)||(u->e!=NULL))
  {
    WerrorS("indexed object must have a name");
    return TRUE;
  }
  intvec * iv=(intvec *)v->Data();
  leftv p=NULL;
  int i;
  sleftv t;

  memset(&t,0,sizeof(t));
  t.rtyp=INT_CMD;
  for (i=0;i<iv->length(); i++)
  {
    t.data=(char *)(*iv)[i];
    if (p==NULL)
    {
      p=res;
    }
    else
    {
      p->next=(leftv)Alloc0(sizeof(sleftv));
      p=p->next;
    }
    p->rtyp=IDHDL;
    p->data=u->data;
    p->name=u->name;
    p->flag|=u->flag;
    p->e=jjMakeSub(&t);
  }
  u->rtyp=0;
  u->data=NULL;
  u->name=NULL;
  return FALSE;
}
static BOOLEAN jjINDEX_P(leftv res, leftv u, leftv v)
{
  poly p=(poly)u->Data();
  int i=(int)v->Data();
  int j=0;
  while (p!=NULL)
  {
    j++;
    if (j==i)
    {
      res->data=(char *)pHead(p);
      return FALSE;
    }
    pIter(p);
  }
  return FALSE;
}
static BOOLEAN jjINDEX_P_IV(leftv res, leftv u, leftv v)
{
  poly p=(poly)u->Data();
  poly r=NULL;
  intvec *iv=(intvec *)v->CopyD();
  int i;
  int sum=0;
  for(i=iv->length()-1;i>=0;i--)
    sum+=(*iv)[i];
  int j=0;
  while ((p!=NULL) && (sum>0))
  {
    j++;
    for(i=iv->length()-1;i>=0;i--)
    {
      if (j==(*iv)[i])
      {
        r=pAdd(r,pHead(p));
        sum-=j;
        (*iv)[i]=0;
        break;
      }
    }
    pIter(p);
  }
  delete iv;
  res->data=(char *)r;
  return FALSE;
}
static BOOLEAN jjINDEX_V(leftv res, leftv u, leftv v)
{
  poly p=(poly)u->CopyD();
  poly r=p; // pointer to the begiining of component i
  poly o=NULL;
  int i=(int)v->Data();
  while (p!=NULL)
  {
    if (pGetComp(p)!=i)
    {
      if (r==p) r=pNext(p);
      if (o!=NULL)
      {
        pDelete1(&pNext(o));
        p=pNext(o);
      }
      else
        pDelete1(&p);
    }
    else
    {
      pGetComp(p)=0;
      o=p;
      p=pNext(o);
    }
  }
  res->data=(char *)r;
  return FALSE;
}
static BOOLEAN jjINDEX_V_IV(leftv res, leftv u, leftv v)
{
  poly p=(poly)u->CopyD();
  poly r=p;
  intvec *iv=(intvec *)v->Data();
  int i;
  while (p!=NULL)
  {
    for(i=0;i<iv->length();i++)
    {
      if (pGetComp(p)==(*iv)[i])
      {
        pIter(p);
        i=0;
      }
    }
    if (p!=NULL)
    {
      if (r==p) r=pNext(p);
       pDelete1(&p);
    }
  }
  res->data=(char *)r;
  return FALSE;
}
static BOOLEAN jjKLAMMER(leftv res, leftv u, leftv v)
{
  char * n = (char *)AllocL(strlen(u->name) + 6);
  sprintf(n,"%s(%d)",u->name,(int)v->Data());
  FreeL((ADDRESS)u->name);
  u->name=NULL;
  syMake(res,n);
  return FALSE;
}
static BOOLEAN jjKLAMMER_IV(leftv res, leftv u, leftv v)
{
  intvec * iv=(intvec *)v->Data();
  leftv p=NULL;
  int i;
  char *n;

  for (i=0;i<iv->length(); i++)
  {
    if (p==NULL)
    {
      p=res;
    }
    else
    {
      p->next=(leftv)Alloc0(sizeof(sleftv));
      p=p->next;
    }
    n = (char *)AllocL(strlen(u->name) + 6);
    sprintf(n,"%s(%d)",u->name,(*iv)[i]);
    syMake(p,n);
  }
  FreeL((ADDRESS)u->name);
  u->name=NULL;
  return FALSE;
}
static BOOLEAN jjPROC(leftv res, leftv u, leftv v)
{
  leftv sl = iiMake_proc((idhdl)u->data,v);
  if (sl==NULL)
  {
    return TRUE;
  }
  else
  {
    memcpy(res,sl,sizeof(sleftv));
  }    
  return FALSE;
}
#ifdef HAVE_DLD
static BOOLEAN jjBIN(leftv res, leftv u, leftv v)
{
  leftv sl = iiMake_binary((idhdl)u->data,v);
  if (sl==NULL)
  {
    res->rtyp=NONE;
  }
  else
  {
    memcpy(res,sl,sizeof(sleftv));
    Free((ADDRESS)sl,sizeof(sleftv));
  }    
  return FALSE;
}
#endif
static BOOLEAN jjMAP(leftv res, leftv u, leftv v)
{
  //Print("try to map %s with %s\n",$3.Name(),$1.Name());
  leftv sl=NULL;
  if (v->e==NULL)
  {
    idhdl h=(idhdl)u->data;
    sl=iiMap(IDMAP(h),v->name);
  }
  //else
  //{
  //  WerrorS("map_id(<name>) expected");
  //}
  if (sl==NULL) return TRUE;
  memcpy(res,sl,sizeof(sleftv));
  Free((ADDRESS)sl,sizeof(*sl));
  return FALSE;
}
static BOOLEAN jjCALL2MANY(leftv res, leftv u, leftv v)
{
  u->next=(leftv)Alloc(sizeof(sleftv));
  memcpy(u->next,v,sizeof(sleftv));
  BOOLEAN r=iiExprArithM(res,u,iiOp);
  v->rtyp=0; v->data=NULL; // iiExprArithM did the CleanUp
  return r;
}
static BOOLEAN jjCOEF(leftv res, leftv u, leftv v)
{
  res->data=(char *)mpCoeffProc((poly)u->Data(),(poly)v->Data());
  return FALSE;
}
static BOOLEAN jjCOEFFS_Id(leftv res, leftv u, leftv v)
{
  int i=pVar((poly)v->Data());
  if (i==0)
  {
    WerrorS("ringvar expected");
    return TRUE;
  }
  res->data=(char *)mpCoeffs((ideal)u->CopyD(),i);
  return FALSE;
}
static BOOLEAN jjCONTRACT(leftv res, leftv u, leftv v)
{
  res->data=(char *)idDiffOp((ideal)u->Data(),(ideal)v->Data(),FALSE);
  return FALSE;
}
static BOOLEAN jjDEG_IV(leftv res, leftv u, leftv v)
{
  short *iv=iv2array((intvec *)v->Data());
  res->data = (char *)pDegW((poly)u->Data(),iv);
  Free((ADDRESS)iv,(pVariables+1)*sizeof(short));
  return FALSE;
}
static BOOLEAN jjDIFF_P(leftv res, leftv u, leftv v)
{
  int i=pVar((poly)v->Data());
  if (i==0)
  {
    WerrorS("ringvar expected");
    return TRUE;
  }
  res->data=(char *)pDiff((poly)(u->Data()),i);
  return FALSE;
}
static BOOLEAN jjDIFF_ID(leftv res, leftv u, leftv v)
{
  int i=pVar((poly)v->Data());
  if (i==0)
  {
    WerrorS("ringvar expected");
    return TRUE;
  }
  res->data=(char *)idDiff((matrix)(u->Data()),i);
  return FALSE;
}
static BOOLEAN jjDIFF_ID_ID(leftv res, leftv u, leftv v)
{
  res->data=(char *)idDiffOp((ideal)u->Data(),(ideal)v->Data());
  return FALSE;
}
  
static BOOLEAN jjELIMIN(leftv res, leftv u, leftv v)
{
  res->data=(char *)idElimination((ideal)u->Data(),(poly)v->Data());
  setFlag(res,FLAG_STD);
  return FALSE;
}
static BOOLEAN jjDIM2(leftv res, leftv v, leftv w)
{
  assumeStdFlag(v);
  if(currQuotient==NULL)
    res->data = (char *)scDimInt((ideal)(v->Data()),(ideal)w->Data());
  else
  {
    ideal q=idSimpleAdd(currQuotient,(ideal)w->Data());
    res->data = (char *)scDimInt((ideal)(v->Data()),q);
    idDelete(&q);
  }
  return FALSE;
}
static BOOLEAN jjEXTGCD_I(leftv res, leftv u, leftv v)
{
  int p0=ABS((int)u->Data()),p1=ABS((int)v->Data());
  int f0 = 1, f1 = 0, g0 = 0, g1 = 1, q, r;

  while ( p1!=0 )
  {
    q=p0 / p1;
    r=p0 % p1;
    p0 = p1; p1 = r;
    r = g0 - g1 * q;
    g0 = g1; g1 = r;
    r = f0 - f1 * q;
    f0 = f1; f1 = r;
  }
  int a = f0;
  int b = g0;
  if ( (int)u->Data() < 0 ) a=-a;
  if ( (int)v->Data() < 0 ) b=-b;
  lists L=(lists)Alloc(sizeof(slists));
  L->Init(3);
  L->m[0].rtyp=INT_CMD;
  L->m[0].data=(void *)p0;
  L->m[1].rtyp=INT_CMD;
  L->m[1].data=(void *)a;
  L->m[2].rtyp=INT_CMD;
  L->m[2].data=(void *)b;
  res->rtyp=LIST_CMD;
  res->data=(char *)L;
  return FALSE;
}
#ifdef HAVE_LIBFACTORY
static BOOLEAN jjFACSTD2(leftv res, leftv v, leftv w)
{
  res->data=(void *)stdfac((ideal)v->Data(),NULL,testHomog,NULL,
           (ideal)w->Data());
  setFlag(res,FLAG_STD);
  return FALSE;
}
#endif
static BOOLEAN jjFETCH(leftv res, leftv u, leftv v)
{
  ring r=(ring)u->Data();
  idhdl w;

  if ((iiOp!=IMAP_CMD)
  && ((currRing->ch != r->ch)
    || ((currRing->N != r->N)&& (iiOp==FETCH_CMD)))
  )
    goto err_fetch;
  if ((w=r->idroot->get(v->Name(),myynest))!=NULL)
  {
    int *perm=NULL;
    int *par_perm=NULL;
    int par_perm_size=0;
    BOOLEAN bo;
    if (iiOp==IMAP_CMD)
    {
      if (!nSetMap(r->ch,r->parameter,r->P,r->minpoly))
      {
        if (iiOp!=IMAP_CMD)
          goto err_fetch;
        par_perm_size=rPar(r);
        if (r->ch==1)
        {
          if ((currRing->ch!=0)
          && (currRing->ch!=1))
            goto err_fetch;
        }
        else if(r->ch<(-1))
        {
          if ((currRing->ch==(-currRing->ch))
          && (currRing->ch!=currRing->ch))
            goto err_fetch;
        }
        BITSET save_test=test;
        naSetChar(r->ch,TRUE,r->parameter,r->P);
        nSetChar(currRing->ch,TRUE,currRing->parameter,currRing->P);
        test=save_test;
      }
      perm=(int *)Alloc0((r->N+1)*sizeof(int));
      if (par_perm_size!=0)
        par_perm=(int *)Alloc0(par_perm_size*sizeof(int));
      maFindPerm(r->names,       r->N,       r->parameter,        r->P,
                 currRing->names,currRing->N,currRing->parameter, currRing->P,
                 perm,par_perm);
    }
    sleftv tmpW;
    memset(&tmpW,0,sizeof(sleftv));
    tmpW.rtyp=IDTYP(w);
    tmpW.data=IDDATA(w);
    if ((bo=maApplyFetch(iiOp,NULL,res,&tmpW,perm,r->N,par_perm,par_perm_size)))
    {
      Werror("cannot map %s of type %s(%d)",v->name, Tok2Cmdname(w->typ),w->typ);
    }
    if (perm!=NULL)
      Free((ADDRESS)perm,(r->N+1)*sizeof(int));
    if (par_perm!=NULL)
      Free((ADDRESS)par_perm,par_perm_size*sizeof(int));
    return bo;
  }
  else
  {
    Werror("%s undefined in %s",v->Name(),u->Name());
  }
  return TRUE;
err_fetch:
  Werror("no identity map from %s",u->Name());
  return TRUE;
}
static BOOLEAN jjFIND2(leftv res, leftv u, leftv v)
{
  /*4
  * look for the substring what in the string where
  * return the position of the first char of what in where
  * or 0
  */
  char *where=(char *)u->Data();
  char *what=(char *)v->Data();
  char *found = strstr(where,what);
  if (found != NULL)
  {
    res->data=(char *)((found-where)+1);
  }
  /*else res->data=NULL;*/
  return FALSE;
}
static BOOLEAN jjGCD_I(leftv res, leftv u, leftv v)
{
  int p0=ABS((int)u->Data()),p1=ABS((int)v->Data());
  int q, r;

  while ( p1!=0 )
  {
    q=p0 / p1;
    r=p0 % p1;
    p0 = p1; p1 = r;
  }
  res->rtyp=INT_CMD;
  res->data=(char *)p0;
  return FALSE;
}
static BOOLEAN jjHILBERT2(leftv res, leftv u, leftv v)
{
  assumeStdFlag(u);
  intvec *module_w=(intvec*)atGet(u,"isHomog");
  intvec *iv=hFirstSeries((ideal)u->Data(),module_w,currQuotient);
  switch((int)v->Data())
  {
    case 1:
      res->data=(void *)iv;
      return FALSE;
    case 2:
      res->data=(void *)hSecondSeries(iv);
      delete iv;
      return FALSE;
  }
  WerrorS("not implemented yet");
  delete iv;
  return TRUE;
}
static BOOLEAN jjHOMOG_P(leftv res, leftv u, leftv v)
{
  int i=pVar((poly)v->Data());
  if (i==0)
  {
    WerrorS("ringvar expected");
    return TRUE;
  }
  res->data = (char *)pHomogen((poly)u->Data(),i);
  return FALSE;
}
static BOOLEAN jjHOMOG_ID(leftv res, leftv u, leftv v)
{
  int i=pVar((poly)v->Data());
  if (i==0)
  {
    WerrorS("ringvar expected");
    return TRUE;
  }
  res->data = (char *)idHomogen((ideal)u->Data(),i);
  return FALSE;
}
static BOOLEAN jjINTERSECT(leftv res, leftv u, leftv v)
{
  res->data=(char *)idSect((ideal)u->Data(),(ideal)v->Data());
  setFlag(res,FLAG_STD);
  return FALSE;
}
static BOOLEAN jjJET_P(leftv res, leftv u, leftv v)
{
  res->data = (char *)pJet((poly)u->Data(),(int)v->Data());
  return FALSE;
}
static BOOLEAN jjJET_ID(leftv res, leftv u, leftv v)
{
  res->data = (char *)idJet((ideal)u->Data(),(int)v->Data());
  return FALSE;
}
static BOOLEAN jjKoszul(leftv res, leftv u, leftv v)
{
  return mpKoszul(res, u,v);
}
static BOOLEAN jjKoszul_Id(leftv res, leftv u, leftv v)
{
  sleftv h;
  memset(&h,0,sizeof(sleftv));
  h.rtyp=INT_CMD;
  h.data=(void *)IDELEMS((ideal)v->Data());
  return mpKoszul(res, &h, u, v);
}
static BOOLEAN jjLIFT(leftv res, leftv u, leftv v)
{
  ideal m;
  int ul= IDELEMS((ideal)u->Data());
  int vl= IDELEMS((ideal)v->Data());
  if (hasFlag(u,FLAG_STD))
  {
    m = idLift((ideal)u->Data(),(ideal)v->Data());
    res->data = (char *)idModule2formatedMatrix(m,ul,vl);
  }
  else
  {
    // matrix ma=mpNew(1,1);
    // ideal mo=idLiftStd((ideal)u->CopyD(),currQuotient,&ma,testHomog);
    // m = idLift(mo,(ideal)v->Data());
    // matrix r=idModule2formatedMatrix(m, IDELEMS(mo),vl);
    // idDelete(&mo);
    // // idDelete(&m); already done by idModule2formatedMatrix
    // res->data=(char *)mpMult(ma,r);
    // idDelete((ideal *)&ma);
    // idDelete((ideal *)&r);
    m = idLiftNonStB((ideal)u->Data(),(ideal)v->Data());
    res->data = (char *)idModule2formatedMatrix(m,ul,vl);
  }
  return FALSE;
}
static BOOLEAN jjLIFTSTD(leftv res, leftv u, leftv v)
{
  if ((v->rtyp!=IDHDL)||(v->e!=NULL)) return TRUE;
  idhdl h=(idhdl)v->data;
  res->data = (char *)idLiftStd((ideal)u->CopyD(), currQuotient,
              &(h->data.umatrix),testHomog);
  setFlag(res,FLAG_STD);
  return FALSE;
}
static BOOLEAN jjMINOR(leftv res, leftv u, leftv v)
{
  res->data = (char *)idMinors((matrix)u->Data(),(int)v->Data());
  return FALSE;
}
static BOOLEAN jjMODULO(leftv res, leftv u, leftv v)
{
  res->data = (char *)idModulo((ideal)u->Data(),(ideal)v->Data());
  return FALSE;
}
static BOOLEAN jjMONITOR2(leftv res, leftv u,leftv v)
{
  char *opt=(char *)v->Data();
  int mode=0;
  while(*opt!='\0')
  {
    if (*opt=='i') mode |= PROT_I;
    else if (*opt=='o') mode |= PROT_O;
    opt++;
  }
  monitor((char *)(u->Data()),mode);
  return FALSE;
}
static BOOLEAN jjPARSTR2(leftv res, leftv u, leftv v)
{
  idhdl h=(idhdl)u->data;
  int i=(int)v->Data();
  int p=0;
  if ((0<i)
  && (IDRING(h)->parameter!=NULL)
  && (i<=(p=rPar(IDRING(h)))))
    res->data=mstrdup(IDRING(h)->parameter[i-1]);
  else
  {
    Werror("par number %d out of range 1..%d",i,p);
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjQUOT(leftv res, leftv u, leftv v)
{
  res->data = (char *)idQuot((ideal)u->Data(),(ideal)v->Data(),
    hasFlag(u,FLAG_STD));
  idDelMultiples((ideal)(res->data));
  return FALSE;
}
static BOOLEAN jjRANDOM(leftv res, leftv u, leftv v)
{
  int i=(int)u->Data();
  int j=(int)v->Data();
#ifdef buildin_rand
  res->data =(char *)((i > j) ? i : (siRand() % (j-i+1)) + i);
#else
  res->data =(char *)((i > j) ? i : (rand() % (j-i+1)) + i);
#endif
  return FALSE;
}
static BOOLEAN jjREAD2(leftv res, leftv u, leftv v)
{
  si_link l=(si_link)u->Data();
  leftv r=slRead(l,v);
  if (r==NULL)
  {
    const char *s;
    if ((l!=NULL)&&(l->name!=NULL)) s=l->name;
    else                            s=sNoName;
    Werror("cannot read from `%s`",s);
    return TRUE;
  }
  memcpy(res,r,sizeof(sleftv));
  Free((ADDRESS)r,sizeof(sleftv));
  return FALSE;
}
static BOOLEAN jjREDUCE_P(leftv res, leftv u, leftv v)
{
  assumeStdFlag(v);
  res->data = (char *)kNF((ideal)v->Data(),currQuotient,(poly)u->Data());
  return FALSE;
}
static BOOLEAN jjREDUCE_ID(leftv res, leftv u, leftv v)
{
  assumeStdFlag(v);
  res->data = (char *)kNF((ideal)v->Data(),currQuotient,(ideal)u->Data());
  return FALSE;
}
static BOOLEAN jjRES(leftv res, leftv u, leftv v)
{
  int maxl=(int)v->Data();
  if (maxl<0)
  {
    WerrorS("length for res must not be negative");
    return TRUE;
  }
  int l=0;
  resolvente r;
  intvec **weights=NULL;
  int wmaxl=maxl;
  maxl--;
  if ((maxl==-1) && (iiOp!=MRES_CMD))
    maxl = pVariables-1;
  if ((iiOp == RES_CMD) || (iiOp == MRES_CMD))
  {
    if (BTEST1(28))
    {
      r=syMinRes((ideal)u->Data(),maxl,&l, iiOp==MRES_CMD);
    }
    else
    {
      intvec * iv=(intvec*)atGet(u,"isHomog");
      if (iv!=NULL)
      {
        weights = (intvec**)Alloc0(sizeof(intvec*));
        weights[0] = ivCopy(iv);
        l=1;
      }
      r=syResolvente((ideal)u->Data(),maxl,&l, &weights, iiOp==MRES_CMD);
    }
  }
  else
    r=sySchreyerResolvente((ideal)u->Data(),maxl,&l);
  if (r==NULL) return TRUE;
  res->data=(void *)liMakeResolv(r,l,wmaxl,u->Typ(),weights);
  return FALSE;
}
static BOOLEAN jjRSUM(leftv res, leftv u, leftv v)
{
  ring r;
  int i=rSum((ring)u->Data(),(ring)v->Data(),r);
  res->data = (char *)r;
  return (i==-1);
}
#define SIMPL_MULT 8
#define SIMPL_EQU  4
#define SIMPL_NULL 2
#define SIMPL_NORM 1
static BOOLEAN jjSIMPL_ID(leftv res, leftv u, leftv v)
{
  int sw = (int)v->Data();
  ideal id = (ideal)u->CopyD();
  if (sw & SIMPL_NULL)
  {
    idSkipZeroes(id);
  }
  if (sw & SIMPL_NORM)
  {
    idNorm(id);
  }
  if (sw & SIMPL_MULT)
  {
    idDelMultiples(id);
  }
  else if(sw & SIMPL_EQU)
  {
    idDelEquals(id);
  }
  res->data = (char * )id;
  return FALSE;
}
static BOOLEAN jjSTATUS2(leftv res, leftv u, leftv v)
{
  res->data = mstrdup(slStatus((si_link) u->Data(), (char *) v->Data()));
  return FALSE;
}
static BOOLEAN jjSIMPL_P(leftv res, leftv u, leftv v)
{
  int sw = (int)v->Data();
  poly p = (poly)u->CopyD();
  if (sw & SIMPL_NORM)
  {
    pNorm(p);
  }
  res->data = (char * )p;
  return FALSE;
}
static BOOLEAN jjSTD_HILB(leftv res, leftv u, leftv v)
{
  ideal result;
  intvec *w=(intvec *)atGet(u,"isHomog");
  tHomog hom=testHomog;
  if (w!=NULL)
  {
    w=ivCopy(w);
    hom=isHomog;
  }
  result=std((ideal)(u->Data()),currQuotient,hom,&w,(intvec *)v->Data());
  idSkipZeroes(result);
  res->data = (char *)result;
  setFlag(res,FLAG_STD);
  if (w!=NULL) atSet(res,mstrdup("isHomog"),w,INTVEC_CMD);
  return FALSE;
}
static BOOLEAN jjSTD_1(leftv res, leftv u, leftv v)
{
  ideal result;
  intvec *w=(intvec *)atGet(u,"isHomog");
  tHomog hom=testHomog;
  if (w!=NULL)
  {
    w=ivCopy(w);
    hom=isHomog;
  }
  ideal i1=(ideal)u->Data();
  ideal i0=idInit(1,i1->rank);
  i0->m[0]=(poly)v->Data();
  i1=idSimpleAdd(i1,i0);
  i0->m[0]=NULL;
  idDelete(&i0);
  BITSET save_test=test;
  test|=Sy_bit(OPT_SB_1);
  result=std(i1,currQuotient,hom,&w);
  test=save_test;
  idDelete(&i1);
  idSkipZeroes(result);
  res->data = (char *)result;
  setFlag(res,FLAG_STD);
  if (w!=NULL) atSet(res,mstrdup("isHomog"),w,INTVEC_CMD);
  return FALSE;
}
static BOOLEAN jjVARSTR2(leftv res, leftv u, leftv v)
{
  idhdl h=(idhdl)u->data;
  int i=(int)v->Data();
  if ((0<i) && (i<=IDRING(h)->N))
    res->data=mstrdup(IDRING(h)->names[i-1]);
  else
  {
    Werror("var number %d out of range 1..%d",i,IDRING(h)->N);
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjWEDGE(leftv res, leftv u, leftv v)
{
  res->data = (char *)mpWedge((matrix)u->Data(),(int)v->Data());
  return FALSE;
}
#define jjWRONG2 (proc2)jjWRONG
#define jjWRONG3 (proc3)jjWRONG
static BOOLEAN jjWRONG(leftv res, leftv u)
{
  return TRUE;
}
/*=================== operations with 2 args.: table =================*/
struct sValCmd2 dArith2[]=
{
// operations:
// proc        cmd              res             arg1        arg2
 {jjPLUS_I,    '+',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjPLUS_N,    '+',            NUMBER_CMD,     NUMBER_CMD, NUMBER_CMD }
,{jjPLUS_P,    '+',            POLY_CMD,       POLY_CMD,   POLY_CMD }
,{jjPLUS_P,    '+',            VECTOR_CMD,     VECTOR_CMD, VECTOR_CMD }
,{jjPLUS_ID,   '+',            IDEAL_CMD,      IDEAL_CMD,  IDEAL_CMD }
,{jjPLUS_ID,   '+',            MODUL_CMD,      MODUL_CMD,  MODUL_CMD }
,{jjPLUS_P_MA, '+',            MATRIX_CMD,     POLY_CMD,   MATRIX_CMD }
,{jjPLUS_MA_P, '+',            MATRIX_CMD,     MATRIX_CMD, POLY_CMD }
,{jjPLUS_MA,   '+',            MATRIX_CMD,     MATRIX_CMD, MATRIX_CMD }
,{jjPLUS_S,    '+',            STRING_CMD,     STRING_CMD, STRING_CMD }
,{jjOP_IV_I,   '+',            INTVEC_CMD,     INTVEC_CMD, INT_CMD }
,{jjOP_I_IV,   '+',            INTVEC_CMD,     INT_CMD,    INTVEC_CMD }
,{jjOP_IM_I,   '+',            INTMAT_CMD,     INTMAT_CMD, INT_CMD }
,{jjOP_I_IM,   '+',            INTMAT_CMD,     INT_CMD,    INTMAT_CMD }
,{jjPLUS_IV,   '+',            INTVEC_CMD,     INTVEC_CMD, INTVEC_CMD }
,{jjPLUS_IV,   '+',            INTMAT_CMD,     INTMAT_CMD, INTMAT_CMD }
,{lAdd,        '+',            LIST_CMD,       LIST_CMD,   LIST_CMD }
,{jjRSUM,      '+',            RING_CMD,       RING_CMD,   RING_CMD }
,{jjMINUS_I,   '-',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjMINUS_N,   '-',            NUMBER_CMD,     NUMBER_CMD, NUMBER_CMD }
,{jjMINUS_P,   '-',            POLY_CMD,       POLY_CMD,   POLY_CMD }
,{jjMINUS_P,   '-',            VECTOR_CMD,     VECTOR_CMD, VECTOR_CMD }
,{jjPLUS_MA_P, '-',            MATRIX_CMD,     MATRIX_CMD, POLY_CMD }
,{jjMINUS_MA,  '-',            MATRIX_CMD,     MATRIX_CMD, MATRIX_CMD }
,{jjOP_IV_I,   '-',            INTVEC_CMD,     INTVEC_CMD, INT_CMD }
,{jjOP_IM_I,   '-',            INTMAT_CMD,     INTMAT_CMD, INT_CMD }
,{jjMINUS_IV,  '-',            INTVEC_CMD,     INTVEC_CMD, INTVEC_CMD }
,{jjMINUS_IV,  '-',            INTMAT_CMD,     INTMAT_CMD, INTMAT_CMD }
,{jjTIMES_I,   '*',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjTIMES_N,   '*',            NUMBER_CMD,     NUMBER_CMD, NUMBER_CMD }
,{jjTIMES_P,   '*',            POLY_CMD,       POLY_CMD,   POLY_CMD }
,{jjTIMES_P,   '*',            VECTOR_CMD,     POLY_CMD,   VECTOR_CMD }
,{jjTIMES_P,   '*',            VECTOR_CMD,     VECTOR_CMD, POLY_CMD}
,{jjTIMES_MA_P1,'*',           IDEAL_CMD,      IDEAL_CMD,  POLY_CMD }
,{jjTIMES_MA_P2,'*',           IDEAL_CMD,      POLY_CMD,   IDEAL_CMD }
,{jjTIMES_ID,  '*',            IDEAL_CMD,      IDEAL_CMD,  IDEAL_CMD }
,{jjTIMES_MA_P1,'*',           MODUL_CMD,      IDEAL_CMD,  VECTOR_CMD }
,{jjTIMES_MA_P2,'*',           MODUL_CMD,      VECTOR_CMD, IDEAL_CMD }
,{jjTIMES_ID,  '*',            MODUL_CMD,      IDEAL_CMD,  MODUL_CMD }
,{jjTIMES_ID,  '*',            MODUL_CMD,      MODUL_CMD,  IDEAL_CMD }
,{jjTIMES_MA_P1,'*',           MATRIX_CMD,     MATRIX_CMD, POLY_CMD }
,{jjTIMES_MA_P2,'*',           MATRIX_CMD,     POLY_CMD,   MATRIX_CMD }
,{jjTIMES_MA_N1,'*',           MATRIX_CMD,     MATRIX_CMD, NUMBER_CMD }
,{jjTIMES_MA_N2,'*',           MATRIX_CMD,     NUMBER_CMD, MATRIX_CMD }
,{jjTIMES_MA_I1,'*',           MATRIX_CMD,     MATRIX_CMD, INT_CMD }
,{jjTIMES_MA_I2,'*',           MATRIX_CMD,     INT_CMD,    MATRIX_CMD }
,{jjTIMES_MA,  '*',            MATRIX_CMD,     MATRIX_CMD, MATRIX_CMD }
,{jjOP_IV_I,   '*',            INTVEC_CMD,     INTVEC_CMD, INT_CMD }
,{jjOP_IV_I,   '*',            INTMAT_CMD,     INTMAT_CMD, INT_CMD }
,{jjTIMES_IV,  '*',            INTVEC_CMD,     INTMAT_CMD, INTVEC_CMD }
,{jjTIMES_IV,  '*',            INTMAT_CMD,     INTMAT_CMD, INTMAT_CMD }
,{jjDIV_I,     '/',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjDIV_N,     '/',            NUMBER_CMD,     NUMBER_CMD, NUMBER_CMD }
,{jjDIV_P,     '/',            POLY_CMD,       POLY_CMD,   POLY_CMD }
,{jjDIV_P,     '/',            VECTOR_CMD,     VECTOR_CMD, POLY_CMD }
,{jjOP_IV_I,   '/',            INTVEC_CMD,     INTVEC_CMD, INT_CMD }
,{jjOP_IV_I,   '/',            INTMAT_CMD,     INTMAT_CMD, INT_CMD }
,{jjMOD_I,     '%',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjOP_IV_I,   '%',            INTVEC_CMD,     INTVEC_CMD, INT_CMD }
,{jjOP_IV_I,   '%',            INTMAT_CMD,     INTMAT_CMD, INT_CMD }
,{jjPOWER_I,   '^',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjPOWER_N,   '^',            NUMBER_CMD,     NUMBER_CMD, INT_CMD }
,{jjPOWER_P,   '^',            POLY_CMD,       POLY_CMD,   INT_CMD }
,{jjPOWER_ID,  '^',            IDEAL_CMD,      IDEAL_CMD,  INT_CMD }
,{jjLE_I,      LE,             INT_CMD,        INT_CMD,    INT_CMD }
,{jjLE_N,      LE,             INT_CMD,        NUMBER_CMD, NUMBER_CMD }
,{jjCOMPARE_S, LE,             INT_CMD,        STRING_CMD, STRING_CMD }
,{jjCOMPARE_IV_I,LE,           INT_CMD,        INTVEC_CMD, INT_CMD }
,{jjCOMPARE_IV,LE,             INT_CMD,        INTVEC_CMD, INTVEC_CMD }
,{jjCOMPARE_P, LE,             INT_CMD,        POLY_CMD,   POLY_CMD }
,{jjCOMPARE_P, LE,             INT_CMD,        VECTOR_CMD, VECTOR_CMD }
,{jjLT_I,      '<',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjLT_N,      '<',            INT_CMD,        NUMBER_CMD, NUMBER_CMD }
,{jjCOMPARE_IV_I,'<',           INT_CMD,        INTVEC_CMD, INT_CMD }
,{jjCOMPARE_IV,'<',            INT_CMD,        INTVEC_CMD, INTVEC_CMD }
,{jjCOMPARE_S, '<',            INT_CMD,        STRING_CMD, STRING_CMD }
,{jjCOMPARE_P, '<',            INT_CMD,        POLY_CMD,   POLY_CMD }
,{jjCOMPARE_P, '<',            INT_CMD,        VECTOR_CMD, VECTOR_CMD }
,{jjGE_I,      GE,             INT_CMD,        INT_CMD,    INT_CMD }
,{jjGE_N,      GE,             INT_CMD,        NUMBER_CMD, NUMBER_CMD }
,{jjCOMPARE_S, GE,             INT_CMD,        STRING_CMD, STRING_CMD }
,{jjCOMPARE_IV_I,GE,           INT_CMD,        INTVEC_CMD, INT_CMD }
,{jjCOMPARE_IV,GE,             INT_CMD,        INTVEC_CMD, INTVEC_CMD }
,{jjCOMPARE_P, GE,             INT_CMD,        POLY_CMD,   POLY_CMD }
,{jjCOMPARE_P, GE,             INT_CMD,        VECTOR_CMD, VECTOR_CMD }
,{jjGT_I,      '>',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjGT_N,      '>',            INT_CMD,        NUMBER_CMD, NUMBER_CMD }
,{jjCOMPARE_S, '>',            INT_CMD,        STRING_CMD, STRING_CMD }
,{jjCOMPARE_IV_I,'>',          INT_CMD,        INTVEC_CMD, INT_CMD }
,{jjCOMPARE_IV,'>',            INT_CMD,        INTVEC_CMD, INTVEC_CMD }
,{jjCOMPARE_P, '>',            INT_CMD,        POLY_CMD,   POLY_CMD }
,{jjCOMPARE_P, '>',            INT_CMD,        VECTOR_CMD, VECTOR_CMD }
,{jjAND_I,     '&',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjOR_I,      '|',            INT_CMD,        INT_CMD,    INT_CMD }
,{jjEQUAL_I,   EQUAL_EQUAL,    INT_CMD,        INT_CMD,    INT_CMD }
,{jjEQUAL_N,   EQUAL_EQUAL,    INT_CMD,        NUMBER_CMD, NUMBER_CMD }
,{jjCOMPARE_S, EQUAL_EQUAL,    INT_CMD,        STRING_CMD, STRING_CMD }
,{jjEQUAL_P,   EQUAL_EQUAL,    INT_CMD,        POLY_CMD,   POLY_CMD }
,{jjEQUAL_P,   EQUAL_EQUAL,    INT_CMD,        VECTOR_CMD, VECTOR_CMD }
,{jjCOMPARE_IV_I,EQUAL_EQUAL,  INT_CMD,        INTVEC_CMD, INT_CMD }
,{jjCOMPARE_IV,EQUAL_EQUAL,    INT_CMD,        INTVEC_CMD, INTVEC_CMD }
,{jjCOMPARE_IV,EQUAL_EQUAL,    INT_CMD,        INTMAT_CMD, INTMAT_CMD }
,{jjEQUAL_Ma,  EQUAL_EQUAL,    INT_CMD,        MATRIX_CMD, MATRIX_CMD }
,{jjWRONG2,    EQUAL_EQUAL,    0,              IDEAL_CMD,  IDEAL_CMD }
,{jjWRONG2,    EQUAL_EQUAL,    0,              MODUL_CMD,  MODUL_CMD }
,{jjEQUAL_I,   NOTEQUAL,       INT_CMD,        INT_CMD,    INT_CMD }
,{jjEQUAL_N,   NOTEQUAL,       INT_CMD,        NUMBER_CMD, NUMBER_CMD }
,{jjCOMPARE_S, NOTEQUAL,       INT_CMD,        STRING_CMD, STRING_CMD }
,{jjEQUAL_P,   NOTEQUAL,       INT_CMD,        POLY_CMD,   POLY_CMD }
,{jjEQUAL_P,   NOTEQUAL,       INT_CMD,        VECTOR_CMD, VECTOR_CMD }
,{jjCOMPARE_IV,NOTEQUAL,       INT_CMD,        INTVEC_CMD, INTVEC_CMD }
,{jjCOMPARE_IV,NOTEQUAL,       INT_CMD,        INTMAT_CMD, INTMAT_CMD }
,{jjEQUAL_Ma,  NOTEQUAL,       INT_CMD,        MATRIX_CMD, MATRIX_CMD }
,{jjWRONG2,    NOTEQUAL,       0,              IDEAL_CMD,  IDEAL_CMD }
,{jjWRONG2,    NOTEQUAL,       0,              MODUL_CMD,  MODUL_CMD }
,{jjDOTDOT,    DOTDOT,         INTVEC_CMD,     INT_CMD,    INT_CMD }
,{jjINDEX_I,   '[',            INT_CMD,        INTVEC_CMD, INT_CMD }
,{jjINDEX_IV,  '[',            INT_CMD,        INTVEC_CMD, INTVEC_CMD }
,{jjINDEX_I,   '[',            POLY_CMD,       IDEAL_CMD,  INT_CMD }
,{jjINDEX_I,   '[',            POLY_CMD,       MAP_CMD,    INT_CMD }
,{jjINDEX_IV,  '[',            POLY_CMD,       IDEAL_CMD,  INTVEC_CMD }
,{jjINDEX_I,   '[',            VECTOR_CMD,     MODUL_CMD,  INT_CMD }
,{jjINDEX_IV,  '[',            VECTOR_CMD,     MODUL_CMD,  INTVEC_CMD }
,{jjINDEX_I,   '[',            STRING_CMD,     STRING_CMD, INT_CMD }
,{jjINDEX_IV,  '[',            STRING_CMD,     STRING_CMD, INTVEC_CMD }
,{jjINDEX_I,   '[',            ANY_TYPE/*set by p*/,LIST_CMD, INT_CMD }
,{jjINDEX_IV,  '[',            ANY_TYPE/*set by p*/,LIST_CMD, INTVEC_CMD }
,{jjINDEX_P,   '[',            POLY_CMD,       POLY_CMD,   INT_CMD }
,{jjINDEX_P_IV,'[',            POLY_CMD,       POLY_CMD,   INTVEC_CMD }
,{jjINDEX_V,   '[',            POLY_CMD,       VECTOR_CMD, INT_CMD }
,{jjINDEX_V_IV,'[',            VECTOR_CMD,     VECTOR_CMD, INTVEC_CMD }
,{jjPROC,      '(',            ANY_TYPE/*set by p*/,PROC_CMD, DEF_CMD }
#ifdef HAVE_DLD
,{jjBIN,       '(',            ANY_TYPE/*set by p*/,BIN_CMD, DEF_CMD }
#endif
,{jjMAP,       '(',            ANY_TYPE/*set by p*/,MAP_CMD, DEF_CMD }
,{jjKLAMMER,   '(',            ANY_TYPE/*set by p*/,ANY_TYPE, INT_CMD }
,{jjKLAMMER_IV,'(',            ANY_TYPE/*set by p*/,ANY_TYPE, INTVEC_CMD }
// and the procedures with 2 arguments:
,{atATTRIB2,   ATTRIB_CMD,     NONE/*set by p*/,DEF_CMD,   STRING_CMD }
,{jjCOEF,      COEF_CMD,       MATRIX_CMD,     POLY_CMD,   POLY_CMD }
,{jjCOEFFS_Id, COEFFS_CMD,     MATRIX_CMD,     IDEAL_CMD,  POLY_CMD }
,{jjCOEFFS_Id, COEFFS_CMD,     MATRIX_CMD,     MODUL_CMD,  POLY_CMD }
,{jjCONTRACT,  CONTRACT_CMD,   MATRIX_CMD,     IDEAL_CMD,  IDEAL_CMD }
,{jjDEG_IV,    DEG_CMD,        INT_CMD,        POLY_CMD,   INTVEC_CMD }
,{lDelete,     DELETE_CMD,     LIST_CMD,       LIST_CMD,   INT_CMD }
,{jjDIFF_P,    DIFF_CMD,       POLY_CMD,       POLY_CMD,   POLY_CMD }
,{jjDIFF_P,    DIFF_CMD,       VECTOR_CMD,     VECTOR_CMD, POLY_CMD }
,{jjDIFF_ID,   DIFF_CMD,       IDEAL_CMD,      IDEAL_CMD,  POLY_CMD }
,{jjDIFF_ID_ID,DIFF_CMD,       MATRIX_CMD,     IDEAL_CMD,  IDEAL_CMD }
,{jjDIFF_ID,   DIFF_CMD,       MODUL_CMD,      MODUL_CMD,  POLY_CMD }
,{jjDIFF_ID,   DIFF_CMD,       MATRIX_CMD,     MATRIX_CMD, POLY_CMD }
,{jjDIM2,      DIM_CMD,        INT_CMD,        IDEAL_CMD,  IDEAL_CMD }
,{jjDIM2,      DIM_CMD,        INT_CMD,        MODUL_CMD,  IDEAL_CMD }
,{jjELIMIN,    ELIMINATION_CMD,IDEAL_CMD,      IDEAL_CMD,  POLY_CMD }
,{jjELIMIN,    ELIMINATION_CMD,MODUL_CMD,      MODUL_CMD,  POLY_CMD }
,{jjEXTGCD_I,  EXTGCD_CMD,     LIST_CMD,       INT_CMD,    INT_CMD }
#ifdef HAVE_LIBFACTORY
,{jjEXTGCD_P,  EXTGCD_CMD,     LIST_CMD,       POLY_CMD,   POLY_CMD }
,{jjSQR_FREE_DEC,FAC_CMD,      IDEAL_CMD,      POLY_CMD,   INT_CMD }
,{jjFACSTD2,    FACSTD_CMD,    LIST_CMD,       IDEAL_CMD,  IDEAL_CMD }
#else
,{jjWRONG2,    EXTGCD_CMD,     LIST_CMD,       POLY_CMD,   POLY_CMD }
,{jjWRONG2,    FAC_CMD,        IDEAL_CMD,      POLY_CMD,   INT_CMD }
,{jjWRONG2,    FACSTD_CMD,     LIST_CMD,       IDEAL_CMD,  IDEAL_CMD }
#endif
,{jjFETCH,     FETCH_CMD,      ANY_TYPE/*set by p*/,RING_CMD,  ANY_TYPE }
,{jjFETCH,     FETCH_CMD,      ANY_TYPE/*set by p*/,QRING_CMD, ANY_TYPE }
#ifdef HAVE_FGLM 
,{fglmProc,    FGLM_CMD,       IDEAL_CMD,      RING_CMD,   DEF_CMD }
,{fglmProc,    FGLM_CMD,       IDEAL_CMD,      QRING_CMD,  DEF_CMD }
#endif 
,{jjFIND2,     FIND_CMD,       INT_CMD,        STRING_CMD, STRING_CMD }
,{jjGCD_I,     GCD_CMD,        INT_CMD,        INT_CMD,    INT_CMD }
#ifdef HAVE_LIBFACTORY
,{jjGCD_P,     GCD_CMD,        POLY_CMD,       POLY_CMD,   POLY_CMD }
#else
,{jjWRONG2,    GCD_CMD,        POLY_CMD,       POLY_CMD,   POLY_CMD }
#endif
,{jjHILBERT2,  HILBERT_CMD,    INTVEC_CMD,     IDEAL_CMD,  INT_CMD }
,{jjHILBERT2,  HILBERT_CMD,    INTVEC_CMD,     MODUL_CMD,  INT_CMD }
,{jjHOMOG_P,   HOMOG_CMD,      POLY_CMD,       POLY_CMD,   POLY_CMD }
,{jjHOMOG_P,   HOMOG_CMD,      VECTOR_CMD,     VECTOR_CMD, POLY_CMD }
,{jjHOMOG_ID,  HOMOG_CMD,      IDEAL_CMD,      IDEAL_CMD,  POLY_CMD }
,{jjHOMOG_ID,  HOMOG_CMD,      MODUL_CMD,      MODUL_CMD,  POLY_CMD }
,{jjCALL2MANY, IDEAL_CMD,      IDEAL_CMD,      DEF_CMD,    DEF_CMD }
,{jjFETCH,     IMAP_CMD,       ANY_TYPE/*set by p*/,RING_CMD,  ANY_TYPE }
,{jjFETCH,     IMAP_CMD,       ANY_TYPE/*set by p*/,QRING_CMD, ANY_TYPE }
,{lInsert,     INSERT_CMD,     LIST_CMD,       LIST_CMD,   DEF_CMD }
,{jjINTERSECT, INTERSECT_CMD,  IDEAL_CMD,      IDEAL_CMD,  IDEAL_CMD }
,{jjINTERSECT, INTERSECT_CMD,  MODUL_CMD,      MODUL_CMD,  MODUL_CMD }
,{jjJET_P,     JET_CMD,        POLY_CMD,       POLY_CMD,   INT_CMD }
,{jjJET_ID,    JET_CMD,        IDEAL_CMD,      IDEAL_CMD,  INT_CMD }
,{jjJET_P,     JET_CMD,        VECTOR_CMD,     VECTOR_CMD, INT_CMD }
,{jjJET_ID,    JET_CMD,        MODUL_CMD,      MODUL_CMD,  INT_CMD }
,{atKILLATTR2, KILLATTR_CMD,   NONE,           IDHDL,      STRING_CMD }
,{jjKoszul,    KOSZUL_CMD,     MATRIX_CMD,     INT_CMD,    INT_CMD }
,{jjKoszul_Id, KOSZUL_CMD,     MATRIX_CMD,     INT_CMD,    IDEAL_CMD }
,{jjLIFT,      LIFT_CMD,       MATRIX_CMD,     IDEAL_CMD,  IDEAL_CMD }
,{jjLIFT,      LIFT_CMD,       MATRIX_CMD,     MODUL_CMD,  MODUL_CMD }
,{jjLIFTSTD,   LIFTSTD_CMD,    IDEAL_CMD,      IDEAL_CMD,  MATRIX_CMD }
,{jjLIFTSTD,   LIFTSTD_CMD,    MODUL_CMD,      MODUL_CMD,  MATRIX_CMD }
,{jjCALL2MANY, LIST_CMD,       LIST_CMD,       DEF_CMD,    DEF_CMD }
,{jjMINOR,     MINOR_CMD,      IDEAL_CMD,      MATRIX_CMD, INT_CMD }
,{jjCALL2MANY, MODUL_CMD,      MODUL_CMD,      DEF_CMD,    DEF_CMD }
,{jjMODULO,    MODULO_CMD,     MODUL_CMD,      IDEAL_CMD,  IDEAL_CMD }
,{jjMODULO,    MODULO_CMD,     MODUL_CMD,      MODUL_CMD,  MODUL_CMD }
,{jjMONITOR2,  MONITOR_CMD,    NONE,           STRING_CMD, STRING_CMD }
,{jjRES,       MRES_CMD,       LIST_CMD,       IDEAL_CMD,  INT_CMD }
,{jjRES,       MRES_CMD,       LIST_CMD,       MODUL_CMD,  INT_CMD }
,{jjPARSTR2,   PARSTR_CMD,     STRING_CMD,     RING_CMD,   INT_CMD }
,{jjPARSTR2,   PARSTR_CMD,     STRING_CMD,     QRING_CMD,  INT_CMD }
,{jjPRINT_FORMAT, PRINT_CMD,   NONE,           DEF_CMD,    STRING_CMD }
,{jjQUOT,      QUOTIENT_CMD,   IDEAL_CMD,      IDEAL_CMD,  IDEAL_CMD }
,{jjQUOT,      QUOTIENT_CMD,   MODUL_CMD,      MODUL_CMD,  IDEAL_CMD }
,{jjQUOT,      QUOTIENT_CMD,   IDEAL_CMD,      MODUL_CMD,  MODUL_CMD }
,{jjRANDOM,    RANDOM_CMD,     INT_CMD,        INT_CMD,    INT_CMD }
,{jjREAD2,     READ_CMD,       STRING_CMD,     LINK_CMD,   STRING_CMD }
,{jjREDUCE_P,  REDUCE_CMD,     POLY_CMD,       POLY_CMD,   IDEAL_CMD }
,{jjREDUCE_P,  REDUCE_CMD,     VECTOR_CMD,     VECTOR_CMD, IDEAL_CMD }
,{jjREDUCE_P,  REDUCE_CMD,     VECTOR_CMD,     VECTOR_CMD, MODUL_CMD }
,{jjREDUCE_ID, REDUCE_CMD,     IDEAL_CMD,      IDEAL_CMD,  IDEAL_CMD }
,{jjREDUCE_ID, REDUCE_CMD,     MODUL_CMD,      MODUL_CMD,  MODUL_CMD }
,{jjREDUCE_ID, REDUCE_CMD,     MODUL_CMD,      MODUL_CMD,  IDEAL_CMD }
,{jjRES,       RES_CMD,        LIST_CMD,       IDEAL_CMD,  INT_CMD }
,{jjRES,       RES_CMD,        LIST_CMD,       MODUL_CMD,  INT_CMD }
,{jjSTATUS2,   STATUS_CMD,     STRING_CMD,     LINK_CMD,   STRING_CMD}
,{jjSIMPL_P,   SIMPLIFY_CMD,   POLY_CMD,       POLY_CMD,   INT_CMD }
,{jjSIMPL_P,   SIMPLIFY_CMD,   VECTOR_CMD,     VECTOR_CMD, INT_CMD }
,{jjSIMPL_ID,  SIMPLIFY_CMD,   IDEAL_CMD,      IDEAL_CMD,  INT_CMD }
,{jjSIMPL_ID,  SIMPLIFY_CMD,   MODUL_CMD,      MODUL_CMD,  INT_CMD }
,{jjRES,       SRES_CMD,       LIST_CMD,       IDEAL_CMD,  INT_CMD }
,{jjRES,       SRES_CMD,       LIST_CMD,       MODUL_CMD,  INT_CMD }
,{jjCALL2MANY, SYSTEM_CMD,     ANY_TYPE/*set by p*/,STRING_CMD, DEF_CMD }
,{jjSTD_1,     STD_CMD,        IDEAL_CMD,      IDEAL_CMD,  POLY_CMD }
,{jjSTD_1,     STD_CMD,        MODUL_CMD,      MODUL_CMD,  VECTOR_CMD }
,{jjSTD_HILB,  STD_CMD,        IDEAL_CMD,      IDEAL_CMD,  INTVEC_CMD }
,{jjSTD_HILB,  STD_CMD,        MODUL_CMD,      MODUL_CMD,  INTVEC_CMD }
,{jjVARSTR2,   VARSTR_CMD,     STRING_CMD,     RING_CMD,   INT_CMD }
,{jjVARSTR2,   VARSTR_CMD,     STRING_CMD,     QRING_CMD,  INT_CMD }
,{jjWEDGE,     WEDGE_CMD,      MATRIX_CMD,     MATRIX_CMD, INT_CMD }
,{NULL,        0,              0,              0,          0 }
};
/*=================== operations with 1 arg.: static proc =================*/
static BOOLEAN jjDUMMY(leftv res, leftv u)
{
  res->data = (char *)u->CopyD();
  return FALSE;
}
//static BOOLEAN jjPLUSPLUS(leftv res, leftv u)
//{
//  res->data = (char *)((int)u->Data()+1);
//  return FALSE;
//}
//static BOOLEAN jjMINUSMINUS(leftv res, leftv u)
//{
//  res->data = (char *)((int)u->Data()-1);
//  return FALSE;
//}
static BOOLEAN jjPLUSPLUS(leftv res, leftv u)
{
  if (IDTYP((idhdl)u->data)==INT_CMD)
  {
    if (iiOp==PLUSPLUS) IDINT((idhdl)u->data)++;
    else                IDINT((idhdl)u->data)--;
    return FALSE;
  }
  return TRUE;
}
static BOOLEAN jjUMINUS_I(leftv res, leftv u)
{
  res->data = (char *)(-(int)u->Data());
  return FALSE;
}
static BOOLEAN jjUMINUS_N(leftv res, leftv u)
{
  number n=nNeg((number)u->CopyD());
  res->data = (char *)n;
  return FALSE;
}
static BOOLEAN jjUMINUS_P(leftv res, leftv u)
{
  res->data = (char *)pNeg((poly)u->CopyD(POLY_CMD));
  return FALSE;
}
static BOOLEAN jjUMINUS_MA(leftv res, leftv u)
{
  poly m1=pISet(-1);
  res->data = (char *)mpMultP((matrix)u->CopyD(MATRIX_CMD),m1);
  return FALSE;
}
static BOOLEAN jjUMINUS_IV(leftv res, leftv u)
{
  intvec *iv=(intvec *)u->CopyD(INTVEC_CMD);
  (*iv)*=(-1);
  res->data = (char *)iv;
  return FALSE;
}
static BOOLEAN jjPROC1(leftv res, leftv u)
{
  leftv sl = iiMake_proc((idhdl)u->data,NULL);
  if (sl==NULL)
  {
    return TRUE;
  }
  else
  {
    memcpy(res,sl,sizeof(sleftv));
  }    
  return FALSE;
}
#ifdef HAVE_DLD
static BOOLEAN jjBIN1(leftv res, leftv u)
{
  leftv sl = iiMake_binary((idhdl)u->data,NULL);
  if (sl==NULL)
  {
    res->rtyp=NONE;
  }
  else
  {
    memcpy(res,sl,sizeof(sleftv));
    Free((ADDRESS)sl,sizeof(sleftv));
  }    
  return FALSE;
}
#endif
static BOOLEAN jjBAREISS(leftv res, leftv v)
{
  matrix m=(matrix)v->Data();
  m=mpBareiss(m,FALSE);
  res->data = (char *)m;
  return FALSE;
}
static BOOLEAN jjBAREISS_IM(leftv res, leftv v)
{
  intvec *m=(intvec *)v->CopyD();
  ivTriangMat(m,1,1);
  res->data = (char *)m;
  return FALSE;
}
static BOOLEAN jjCALL1MANY(leftv res, leftv u)
{
  return iiExprArithM(res,u,iiOp);
}
static BOOLEAN jjCHAR(leftv res, leftv v)
{
  res->data = (char *)rChar((ring)v->Data());
  return FALSE;
}
static BOOLEAN jjCOLS(leftv res, leftv v)
{
  res->data = (char *)MATCOLS((matrix)(v->Data()));
  return FALSE;
}
static BOOLEAN jjCOLS_IV(leftv res, leftv v)
{
  res->data = (char *)((intvec*)(v->Data()))->cols();
  return FALSE;
}
static BOOLEAN jjCONTENT(leftv res, leftv v)
{
  poly p=(poly)v->CopyD();
  pCleardenom(p);
  res->data = (char *)p;
  return FALSE;
}
static BOOLEAN jjCOUNT_N(leftv res, leftv v)
{
  res->data = (char *)nSize((number)v->Data());
  return FALSE;
}
static BOOLEAN jjCOUNT_L(leftv res, leftv v)
{
  lists l=(lists)v->Data();
  res->data = (char *)(l->nr+1);
  return FALSE;
}
static BOOLEAN jjCOUNT_M(leftv res, leftv v)
{
  matrix m=(matrix)v->Data();
  res->data = (char *)(MATROWS(m)*MATCOLS(m));
  return FALSE;
}
static BOOLEAN jjCOUNT_IV(leftv res, leftv v)
{
  res->data = (char *)((intvec*)(v->Data()))->length();
  return FALSE;
}
static BOOLEAN jjDEG(leftv res, leftv v)
{
  int dummy;
  poly p=(poly)v->Data();
  if (p!=NULL) res->data = (char *)pLDeg(p,&dummy);
  else res->data=(char *)-1;
  return FALSE;
}
static BOOLEAN jjDEGREE(leftv res, leftv v)
{
  assumeStdFlag(v);
  scDegree((ideal)v->Data(),currQuotient);
  return FALSE;
}
static BOOLEAN jjDEFINED(leftv res, leftv v)
{
  if ((v->rtyp==IDHDL)
  && ((myynest==IDLEV((idhdl)v->data))||(0==IDLEV((idhdl)v->data))))
  {
    res->data=(void *)(IDLEV((idhdl)v->data)+1);
  }
  else if (v->rtyp!=0) res->data=(void *)(-1);
  return FALSE;
}
#ifdef HAVE_LIBFACTORY
static BOOLEAN jjDET(leftv res, leftv v)
{
  int i,j;
  matrix m=(matrix)v->Data();
  if (currRing->parameter==NULL)
  {
    for(i=1;i<=m->rows();i++)
    {
      for(j=1;j<=m->cols();j++)
      {
        if((MATELEM(m,i,j)!=NULL)
        && (!pIsConstant(MATELEM(m,i,j))))
        {
          goto nonconst;
        }
      }
    }

    res->data = (char *)singclap_det((matrix)(v->Data()));
    return FALSE;
  }
nonconst:
  res->data = (char *)mpDet((matrix)(v->Data()));
  return FALSE;
}
#endif
static BOOLEAN jjDIM(leftv res, leftv v)
{
  assumeStdFlag(v);
  res->data = (char *)scDimInt((ideal)(v->Data()),currQuotient);
  return FALSE;
}
static BOOLEAN jjDUMP(leftv res, leftv v)
{
  si_link l = (si_link)v->Data();
  if (slDump(l))
  {
    const char *s;
    if ((l!=NULL)&&(l->name!=NULL)) s=l->name;
    else                            s=sNoName;
    Werror("cannot dump to `%s`",s);
    return TRUE;
  }
  else
    return FALSE;
}
static BOOLEAN jjE(leftv res, leftv v)
{
  res->data = (char *)pOne();
  pSetComp((poly)res->data,(int)v->Data());
  return FALSE;
}
static BOOLEAN jjGETDUMP(leftv res, leftv v)
{
  si_link l = (si_link)v->Data();
  if (slGetDump(l))
  {
    const char *s;
    if ((l!=NULL)&&(l->name!=NULL)) s=l->name;
    else                            s=sNoName;
    Werror("cannot get dump from `%s`",s);
    return TRUE;
  }
  else
    return FALSE;
}
#ifdef HAVE_LIBFACTORY
static BOOLEAN jjFACSTD(leftv res, leftv v)
{
  res->data=(void *)stdfac((ideal)v->Data(),NULL,testHomog,NULL);
  setFlag(res,FLAG_STD);
  return FALSE;
}
#endif
static BOOLEAN jjHILBERT(leftv res, leftv v)
{
  assumeStdFlag(v);
  intvec *module_w=(intvec*)atGet(v,"isHomog");
  //scHilbertPoly((ideal)v->Data(),currQuotient);
  hLookSeries((ideal)v->Data(),module_w,currQuotient);
  return FALSE;
}
static BOOLEAN jjHILBERT_IV(leftv res, leftv v)
{
  res->data=(void *)hSecondSeries((intvec *)v->Data());
  return FALSE;
}
static BOOLEAN jjHOMOG1(leftv res, leftv v)
{
  intvec *w;
  res->data=(void *)idHomModule((ideal)v->Data(),currQuotient,&w);
  if ((v->rtyp==IDHDL)&&(w!=NULL))
    atSet((idhdl)v->data,mstrdup("isHomog"),w,INTVEC_CMD);
  return FALSE;
}
static BOOLEAN jjIDEAL_Ma(leftv res, leftv v)
{
  matrix mat=(matrix)v->CopyD();
  IDELEMS((ideal)mat)=MATCOLS(mat)*MATROWS(mat);
  MATROWS(mat)=1;
  mat->rank=1;
  res->data=(char *)mat;
  return FALSE;
}
static BOOLEAN jjIDEAL_R(leftv res, leftv v)
{
  if (currRing!=NULL)
  {
    ring q=(ring)v->Data();
    if ((q->ch==currRing->ch)
    && (q->N==currRing->N)) /* && varnames equ. */
    {
      if (q->qideal==NULL)
        res->data=(char *)idInit(1,1);
      else
        res->data=(char *)idCopy(q->qideal);
      return FALSE;
    }
  }
  return TRUE;
}
static BOOLEAN jjIm2Iv(leftv res, leftv v)
{
  intvec *iv = (intvec *)v->CopyD();
  iv->makeVector();
  res->data = iv;
  return FALSE;
}
static BOOLEAN jjINDEPSET(leftv res, leftv v)
{
  res->data=(void *)scIndIntvec((ideal)(v->Data()),currQuotient);
  return FALSE;
}
static BOOLEAN jjINTERRED(leftv res, leftv v)
{
  ideal result=kInterRed((ideal)(v->Data()),currQuotient);
  //if (result==NULL) return TRUE;
  idSkipZeroes(result);
  res->data = result;
  return FALSE;
}
static BOOLEAN jjIS_RINGVAR_P(leftv res, leftv v)
{
  res->data = (char *)pVar((poly)v->Data());
  return FALSE;
}
static BOOLEAN jjIS_RINGVAR_S(leftv res, leftv v)
{
  res->data = (char *)(rIsRingVar((char *)v->Data())+1);
  return FALSE;
}
static BOOLEAN jjIS_RINGVAR0(leftv res, leftv v)
{
  res->data = (char *)0;
  return FALSE;
}
static BOOLEAN jjJACOB_P(leftv res, leftv v)
{
  ideal i=idInit(pVariables,1);
  int k;
  for (k=0;k<pVariables;k++)
  {
    i->m[k]=pDiff(pCopy((poly)(v->Data())),k+1);
  }
  res->data = (char *)i;
  return FALSE;
}
static BOOLEAN jjKBASE(leftv res, leftv v)
{
  assumeStdFlag(v);
  res->data = (char *)scKBase((ideal)(v->Data()),currQuotient);
  return FALSE;
}
#ifdef MDEBUG
static BOOLEAN jjpHead(leftv res, leftv v)
{
  res->data=(char *)pHead((poly)v->Data());
  return FALSE;
}
#endif
static BOOLEAN jjLEADCOEF(leftv res, leftv v)
{
  poly p=(poly)v->Data();
  if (p==NULL)
  {
    res->data=(char *)nInit(0);
  }
  else
  {
    res->data=(char *)nCopy(pGetCoeff(p));
  }
  return FALSE;
}
static BOOLEAN jjLEADEXP(leftv res, leftv v)
{
  poly p=(poly)v->Data();
  intvec *iv=new intvec(pVariables);
  if (p!=NULL)
  {
    for(int i=0;i<pVariables;i++)
    {
      (*iv)[i]=pGetExp(p,i+1);
    }
  }
  res->data=(char *)iv;
  return FALSE;
}
static BOOLEAN jjMEMORY(leftv res, leftv v)
{
#ifdef MM_COUNT
  switch(((int)v->Data()))
  {
  case 0:
    res->data = (char *)mmMemUsed();
    break;
  case 1:
    res->data = (char *)mmMemReal();
    break;
  default:
#ifdef MM_STAT
    mmStat((int)v->Data());
#endif
    res->data = (char *)0;
  }
  return FALSE;
#else
  res->data = (char *)0;
  return FALSE;
#endif
}
static BOOLEAN jjMONITOR1(leftv res, leftv v)
{
  monitor((char *)(v->Data()),PROT_I);
  return FALSE;
}
static BOOLEAN jjMSTD(leftv res, leftv v)
{
  res->data=(void *)min_std((ideal)v->Data(),currQuotient,testHomog,NULL);
  return FALSE;
}
static BOOLEAN jjMULT(leftv res, leftv v)
{
  assumeStdFlag(v);
  res->data = (char *)scMultInt((ideal)(v->Data()),currQuotient);
  return FALSE;
}
static BOOLEAN jjNAMEOF(leftv res, leftv v)
{
  res->data = (char *)v->name;
  if (res->data==NULL) res->data=mstrdup("");
  v->name=NULL;
  return FALSE;
}
static BOOLEAN jjNAMES(leftv res, leftv v)
{
  res->data=ipNameList(((ring)v->Data())->idroot);
  return FALSE;
}
static BOOLEAN jjNVARS(leftv res, leftv v)
{
  res->data = (char *)(((ring)(v->Data()))->N);
  return FALSE;
}
static BOOLEAN jjOpenClose(leftv res, leftv v)
{
  si_link l=(si_link)v->Data();
  if (iiOp==OPEN_CMD) return slOpen(l, SI_LINK_OPEN);
  else                return slClose(l);
}
static BOOLEAN jjORD(leftv res, leftv v)
{
  poly p=(poly)v->Data();
  res->data=(char *)( p==NULL ? -1 : pFDeg(p) );
  return FALSE;
}
static BOOLEAN jjPAR1(leftv res, leftv v)
{
  int i=(int)v->Data();
  int p=0;
  p=rPar(currRing);
  if ((0<i) && (i<=p))
  {
    res->data=(char *)nPar(i);
  }
  else
  {
    Werror("par number %d out of range 1..%d",i,p);
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjPARDEG(leftv res, leftv v)
{
  res->data = (char *)nParDeg((number)v->Data());
  return FALSE;
}
static BOOLEAN jjPARSTR1(leftv res, leftv v)
{
  if (currRing==NULL)
  {
    WerrorS("no ring active");
    return TRUE;
  }
  int i=(int)v->Data();
  int p=0;
  if ((0<i) && (currRing->parameter!=NULL) && (i<=(p=rPar(currRing))))
    res->data=mstrdup(currRing->parameter[i-1]);
  else
  {
    Werror("par number %d out of range 1..%d",i,p);
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjP2I(leftv res, leftv v)
{
  poly p=(poly)v->Data();
  if (p==NULL) { /*res->data=(char *)0;*/ return FALSE; }
  if ((pNext(p)!=NULL)|| (!pIsConstant(p)))
  {
    WerrorS("poly must be constant");
    return TRUE;
  }
  res->data = (char *)nInt(pGetCoeff(p));
  return FALSE;
}
static BOOLEAN jjPRIME(leftv res, leftv v)
{
  int i = IsPrime((int)(v->Data()));
  res->data = (char *)(i > 1 ? i : 2);
  return FALSE;
}
static BOOLEAN jjP2N(leftv res, leftv v)
{
  number n;
  poly p;
  if (((p=(poly)v->Data())!=NULL)
  && (pIsConstant(p)))
  {
    n=nCopy(pGetCoeff(p));
  }
  else
  {
    n=nInit(0);
  }
  res->data = (char *)n;
  return FALSE;
}
static BOOLEAN jjRESERVEDNAME(leftv res, leftv v)
{
  char *s= (char *)v->Data();
  int i = 1;
  int l = strlen(s);
  while (cmds[i].tokval!=0)
  {
    if (strcmp(s, cmds[i].name) == 0)
    {
      res->data = (char *)1;
      return FALSE;
    }
    i++;
  }
  //res->data = (char *)0;
  return FALSE;
}
static BOOLEAN jjREAD(leftv res, leftv v)
{
  return jjREAD2(res,v,NULL);
}
static BOOLEAN jjREGULARITY(leftv res, leftv v)
{
  res->data = (char *)iiRegularity((lists)v->Data());
  return FALSE;
}
static BOOLEAN jjROWS(leftv res, leftv v)
{
  ideal i = (ideal)v->Data();
  res->data = (char *)i->rank;
  return FALSE;
}
static BOOLEAN jjROWS_IV(leftv res, leftv v)
{
  res->data = (char *)((intvec*)(v->Data()))->rows();
  return FALSE;
}
static BOOLEAN jjRPAR(leftv res, leftv v)
{
  res->data = (char *)((ring)v->Data())->P;
  return FALSE;
}
static BOOLEAN jjSTD(leftv res, leftv v)
{
  ideal result;
  intvec *w=(intvec *)atGet(v,"isHomog");
  tHomog hom=testHomog;
  if (w!=NULL)
  {
    w=ivCopy(w);
    hom=isHomog;
  }
  //if (hasFlag(v,FLAG_STD))
  //  result=stdred((ideal)v->Data(),currQuotient,hom,&w);
  //else
    result=std((ideal)(v->Data()),currQuotient,hom,&w);
  idSkipZeroes(result);
  res->data = (char *)result;
  setFlag(res,FLAG_STD);
  if (w!=NULL) atSet(res,mstrdup("isHomog"),w,INTVEC_CMD);
  return FALSE;
}
static BOOLEAN jjSTRING(leftv res, leftv v)
{
  res->data = (char *)v->String();
  if (res->data==NULL)
  {
    Werror("cannot convert %s to string",Tok2Cmdname(v->Typ()));
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjSort_Id(leftv res, leftv v)
{
  res->data = (char *)idSort((ideal)v->Data());
  return FALSE;
}
static BOOLEAN jjSTRING_PROC(leftv res, leftv v)
{
#ifdef SIC
  if (sic) { MYYERROR("not implemented"); return TRUE;}
#endif
  res->data=mstrdup(IDSTRING((idhdl)v->data));
  return FALSE;
}
static BOOLEAN jjSYZYGY(leftv res, leftv v)
{
  intvec *w=NULL;
  res->data = (char *)idSyzygies((ideal)v->Data(),currQuotient,testHomog,&w);
  if (w!=NULL) delete w;
  return FALSE;
}
static BOOLEAN jjTRACE_IV(leftv res, leftv v)
{
  res->data = (char *)ivTrace((intvec*)(v->Data()));
  return FALSE;
}
static BOOLEAN jjTRANSP_IV(leftv res, leftv v)
{
  res->data = (char *)ivTranp((intvec*)(v->Data()));
  return FALSE;
}
static BOOLEAN jjTYPEOF(leftv res, leftv v)
{
  switch ((int)v->data)
  {
    case INT_CMD:     res->data=mstrdup("int"); break;
    case POLY_CMD:   res->data=mstrdup("poly"); break;
    case VECTOR_CMD:  res->data=mstrdup("vector"); break;
    case STRING_CMD:  res->data=mstrdup("string"); break;
    case INTVEC_CMD:  res->data=mstrdup("intvec"); break;
    case IDEAL_CMD:   res->data=mstrdup("ideal"); break;
    case MATRIX_CMD: res->data=mstrdup("matrix"); break;
    case MODUL_CMD:   res->data=mstrdup("module"); break;
    case MAP_CMD:     res->data=mstrdup("map"); break;
    case PROC_CMD:    res->data=mstrdup("proc"); break;
    case RING_CMD:    res->data=mstrdup("ring"); break;
    case QRING_CMD:   res->data=mstrdup("qring"); break;
    case INTMAT_CMD:  res->data=mstrdup("intmat"); break;
    case NUMBER_CMD:  res->data=mstrdup("number"); break;
    case LIST_CMD:   res->data=mstrdup("list"); break;
    case PACKAGE_CMD: res->data=mstrdup("package"); break;
    case LINK_CMD:   res->data=mstrdup("link"); break; 
    case DEF_CMD:
    case NONE:    res->data=mstrdup("none"); break;
    default:       res->data=mstrdup("?unknown type?");
  }
  return FALSE;
}
static BOOLEAN jjVAR1(leftv res, leftv v)
{
  int i=(int)v->Data();
  if ((0<i) && (i<=currRing->N))
  {
    poly p=pOne();
    pSetExp(p,i,1);
    pSetm(p);
    res->data=(char *)p;
  }
  else
  {
    Werror("var number %d out of range 1..%d",i,currRing->N);
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjVARSTR1(leftv res, leftv v)
{
  if (currRing==NULL)
  {
    WerrorS("no ring active");
    return TRUE;
  }
  int i=(int)v->Data();
  if ((0<i) && (i<=currRing->N))
    res->data=mstrdup(currRing->names[i-1]);
  else
  {
    Werror("var number %d out of range 1..%d",i,currRing->N);
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjVDIM(leftv res, leftv v)
{
  assumeStdFlag(v);
  res->data = (char *)scMult0Int((ideal)v->Data(),currQuotient);
  return FALSE;
}
/*=================== operations with 1 arg.: table =================*/

#ifdef INIT_BUG
#define jjstrlen       (proc1)1
#define jjpLength      (proc1)2
#define jjidElem       (proc1)3
#define jjmpDet        (proc1)4
#define jjidFreeModule (proc1)5
#define jjidVec2Ideal  (proc1)6
#define jjrCharStr     (proc1)7
#ifndef MDEBUG
#define jjpHead        (proc1)8
#endif
#define jjidHead       (proc1)9
#define jjidMaxIdeal   (proc1)10
#define jjidMinBase    (proc1)11
#define jjsyMinBase    (proc1)12
#define jjpMaxComp     (proc1)13
#define jjmpTrace      (proc1)14
#define jjmpTransp     (proc1)15
#define jjrOrdStr      (proc1)16
#define jjrVarStr      (proc1)18
#define jjrParStr      (proc1)19
#define jjidMinEmbedding (proc1)20

extern struct sValCmd1 dArith1[];
void jjInitTab1()
{
  int i=0;
  for (;dArith1[i].cmd!=0;i++)
  {
    if (dArith1[i].res<0)
    {
      switch ((int)dArith1[i].p)
      {
        case (int)jjstrlen:       dArith1[i].p=(proc1)strlen; break;
        case (int)jjpLength:      dArith1[i].p=(proc1)pLength; break;
        case (int)jjidElem:       dArith1[i].p=(proc1)idElem; break;
        case (int)jjidVec2Ideal:  dArith1[i].p=(proc1)idVec2Ideal; break;
#ifndef HAVE_LIBFACTORY
        case (int)jjmpDet:        dArith1[i].p=(proc1)mpDet; break;
#endif
        case (int)jjidFreeModule: dArith1[i].p=(proc1)idFreeModule; break;
        case (int)jjrCharStr:     dArith1[i].p=(proc1)rCharStr; break;
#ifndef MDEBUG
        case (int)jjpHead:        dArith1[i].p=(proc1)pHead; break;
#endif
        case (int)jjidHead:       dArith1[i].p=(proc1)idHead; break;
        case (int)jjidMaxIdeal:   dArith1[i].p=(proc1)idMaxIdeal; break;
        case (int)jjidMinBase:    dArith1[i].p=(proc1)idMinBase; break;
        case (int)jjsyMinBase:    dArith1[i].p=(proc1)syMinBase; break;
        case (int)jjpMaxComp:     dArith1[i].p=(proc1)pMaxComp; break;
        case (int)jjmpTrace:      dArith1[i].p=(proc1)mpTrace; break;
        case (int)jjmpTransp:     dArith1[i].p=(proc1)mpTransp; break;
        case (int)jjrOrdStr:      dArith1[i].p=(proc1)rOrdStr; break;
        case (int)jjrVarStr:      dArith1[i].p=(proc1)rVarStr; break;
        case (int)jjrParStr:      dArith1[i].p=(proc1)rParStr; break;
        case (int)jjidMinEmbedding: dArith1[i].p=(proc1)idMinEmbedding; break;
#ifdef GENTABLE
        default: Werror("missing proc1-definition for %d",(int)dArith1[i].p);
#endif
      }
    }
  }
}
#else
#define jjstrlen       (proc1)strlen
#define jjpLength      (proc1)pLength
#define jjidElem       (proc1)idElem
#define jjmpDet        (proc1)mpDet
#define jjidFreeModule (proc1)idFreeModule
#define jjidVec2Ideal  (proc1)idVec2Ideal
#define jjrCharStr     (proc1)rCharStr
#ifndef MDEBUG
#define jjpHead        (proc1)pHead
#endif
#define jjidHead       (proc1)idHead
#define jjidMaxIdeal   (proc1)idMaxIdeal
#define jjidMinBase    (proc1)idMinBase
#define jjsyMinBase    (proc1)syMinBase
#define jjpMaxComp     (proc1)pMaxComp
#define jjmpTrace      (proc1)mpTrace
#define jjmpTransp     (proc1)mpTransp
#define jjrOrdStr      (proc1)rOrdStr
#define jjrVarStr      (proc1)rVarStr
#define jjrParStr      (proc1)rParStr
#define jjidMinEmbedding (proc1)idMinEmbedding
#endif
BOOLEAN jjnInt(leftv res, leftv u)
{
  number n=(number)u->CopyD();
  res->data=(char *)nInt(n);
  nDelete(&n);
  return FALSE;
}
#define s short
struct sValCmd1 dArith1[]=
{
// operations:
// proc         cmd               res             arg
// {jjPLUSPLUS,   PLUSPLUS,        INT_CMD,        INT_CMD }
//,{jjMINUSMINUS, MINUSMINUS,      INT_CMD,        INT_CMD }
 {jjPLUSPLUS,   PLUSPLUS,        NONE,           IDHDL }
,{jjPLUSPLUS,   MINUSMINUS,      NONE,           IDHDL }
,{jjUMINUS_I,   '-',             INT_CMD,        INT_CMD }
,{jjUMINUS_N,   '-',             NUMBER_CMD,     NUMBER_CMD }
,{jjUMINUS_P,   '-',             POLY_CMD,       POLY_CMD }
,{jjUMINUS_P,   '-',             VECTOR_CMD,     VECTOR_CMD }
,{jjUMINUS_MA,  '-',             MATRIX_CMD,     MATRIX_CMD }
,{jjUMINUS_IV,  '-',             INTVEC_CMD,     INTVEC_CMD }
,{jjUMINUS_IV,  '-',             INTMAT_CMD,     INTMAT_CMD }
,{jjPROC1,      '(',             ANY_TYPE/*set by p*/,PROC_CMD }
#ifdef HAVE_DLD
,{jjBIN1,       '(',             ANY_TYPE/*set by p*/,BINARY_CMD }
#endif
// and the procedures with 1 argument:
,{atATTRIB1,    ATTRIB_CMD,      NONE,           DEF_CMD }
,{jjBAREISS_IM, BAREISS_CMD,     INTMAT_CMD,     INTMAT_CMD }
,{jjBAREISS,    BAREISS_CMD,     MATRIX_CMD,     MATRIX_CMD }
,{jjBETTI,      BETTI_CMD,       INTMAT_CMD,     LIST_CMD }
,{jjCHAR,       CHARACTERISTIC_CMD, INT_CMD,     RING_CMD }
,{jjCHAR,       CHARACTERISTIC_CMD, INT_CMD,     QRING_CMD }
#ifdef HAVE_LIBFACTORY
,{jjCHARSERIES, CHAR_SERIES_CMD, MATRIX_CMD,     IDEAL_CMD }
#else
,{jjWRONG,      CHAR_SERIES_CMD, MATRIX_CMD,     IDEAL_CMD }
#endif
,{jjrCharStr,   CHARSTR_CMD,    -((s)STRING_CMD),RING_CMD }
,{jjrCharStr,   CHARSTR_CMD,    -((s)STRING_CMD),QRING_CMD }
,{jjOpenClose,  CLOSE_CMD,       NONE,           LINK_CMD }
,{jjWRONG,      COLS_CMD,        0,              VECTOR_CMD }
,{jjCOLS,       COLS_CMD,        INT_CMD,        MATRIX_CMD }
,{jjCOLS,       COLS_CMD,        INT_CMD,        IDEAL_CMD }
,{jjCOLS,       COLS_CMD,        INT_CMD,        MODUL_CMD }
,{jjCOLS_IV,    COLS_CMD,        INT_CMD,        INTMAT_CMD }
,{jjWRONG,      COLS_CMD,        0,              INTVEC_CMD }
,{jjCONTENT,    CONTENT_CMD,     POLY_CMD,       POLY_CMD }
,{jjCONTENT,    CONTENT_CMD,     VECTOR_CMD,     VECTOR_CMD }
,{jjCOUNT_N,    COUNT_CMD,       INT_CMD,        NUMBER_CMD }
,{jjstrlen,     COUNT_CMD,      -((s)INT_CMD),   STRING_CMD }
,{jjpLength,    COUNT_CMD,      -((s)INT_CMD),   POLY_CMD }
,{jjpLength,    COUNT_CMD,      -((s)INT_CMD),   VECTOR_CMD }
,{jjidElem,     COUNT_CMD,      -((s)INT_CMD),   IDEAL_CMD }
,{jjidElem,     COUNT_CMD,      -((s)INT_CMD),   MODUL_CMD }
,{jjCOUNT_M,    COUNT_CMD,       INT_CMD,        MATRIX_CMD }
,{jjCOUNT_IV,   COUNT_CMD,       INT_CMD,        INTVEC_CMD }
,{jjCOUNT_IV,   COUNT_CMD,       INT_CMD,        INTMAT_CMD }
,{jjCOUNT_L,    COUNT_CMD,       INT_CMD,        LIST_CMD }
,{jjWRONG,      DEF_CMD,         0,              ANY_TYPE }
,{jjDEG,        DEG_CMD,         INT_CMD,        POLY_CMD }
,{jjDEG,        DEG_CMD,         INT_CMD,        VECTOR_CMD }
,{jjDEGREE,     DEGREE_CMD,      NONE,           IDEAL_CMD }
,{jjDEGREE,     DEGREE_CMD,      NONE,           MODUL_CMD }
,{jjDEFINED,    DEFINED_CMD,     INT_CMD,        DEF_CMD }
#ifdef HAVE_LIBFACTORY
,{jjDET,        DET_CMD,         POLY_CMD,       MATRIX_CMD }
#else
,{jjmpDet,      DET_CMD,        -((s)POLY_CMD),  MATRIX_CMD }
#endif
,{jjDIM,        DIM_CMD,         INT_CMD,        IDEAL_CMD }
,{jjDIM,        DIM_CMD,         INT_CMD,        MODUL_CMD }
,{jjDUMP,       DUMP_CMD,        NONE,           LINK_CMD }
,{jjE,          E_CMD,           VECTOR_CMD,     INT_CMD }
#ifdef HAVE_LIBFACTORY
,{jjFAC_P,      FAC_CMD,         LIST_CMD,       POLY_CMD }
#else
,{jjWRONG,      FAC_CMD,         LIST_CMD,       POLY_CMD }
#endif
,{jjidFreeModule,FREEMODULE_CMD,-((s)MODUL_CMD), INT_CMD }
#ifdef HAVE_LIBFACTORY
,{jjFACSTD,     FACSTD_CMD,      LIST_CMD,       IDEAL_CMD }
#else
,{jjWRONG,      FACSTD_CMD,      LIST_CMD,       IDEAL_CMD }
#endif
,{jjGETDUMP,    GETDUMP_CMD,     NONE,           LINK_CMD } 
,{jjHILBERT,    HILBERT_CMD,     NONE,           IDEAL_CMD }
,{jjHILBERT,    HILBERT_CMD,     NONE,           MODUL_CMD }
,{jjHILBERT_IV, HILBERT_CMD,     INTVEC_CMD,     INTVEC_CMD }
,{jjHOMOG1,     HOMOG_CMD,       INT_CMD,        IDEAL_CMD }
,{jjHOMOG1,     HOMOG_CMD,       INT_CMD,        MODUL_CMD }
,{jjidVec2Ideal,IDEAL_CMD,      -((s)IDEAL_CMD), VECTOR_CMD }
,{jjIDEAL_Ma,   IDEAL_CMD,       IDEAL_CMD,      MATRIX_CMD }
,{jjIDEAL_R,    IDEAL_CMD,       IDEAL_CMD,      QRING_CMD }
,{jjIDEAL_R,    IDEAL_CMD,       IDEAL_CMD,      RING_CMD }
,{jjDUMMY,      IDEAL_CMD,       IDEAL_CMD,      IDEAL_CMD }
,{jjINDEPSET,   INDEPSET_CMD,    INTVEC_CMD,     IDEAL_CMD }
,{jjDUMMY,      INT_CMD,         INT_CMD,        INT_CMD }
,{jjnInt,       INT_CMD,         INT_CMD,        NUMBER_CMD }
,{jjP2I,        INT_CMD,         INT_CMD,        POLY_CMD }
,{jjINTERRED,   INTERRED_CMD,    IDEAL_CMD,      IDEAL_CMD }
,{jjINTERRED,   INTERRED_CMD,    MODUL_CMD,      MODUL_CMD }
,{jjDUMMY,      INTMAT_CMD,      INTMAT_CMD,     INTMAT_CMD }
,{jjIm2Iv,      INTVEC_CMD,      INTVEC_CMD,     INTMAT_CMD }
,{jjDUMMY,      INTVEC_CMD,      INTVEC_CMD,     INTVEC_CMD }
,{jjIS_RINGVAR_P, IS_RINGVAR,    INT_CMD,        POLY_CMD }
,{jjIS_RINGVAR_S, IS_RINGVAR,    INT_CMD,        STRING_CMD }
,{jjIS_RINGVAR0,IS_RINGVAR,      INT_CMD,        ANY_TYPE }
,{jjJACOB_P,    JACOB_CMD,       IDEAL_CMD,      POLY_CMD }
,{mpJacobi,     JACOB_CMD,       MATRIX_CMD,     IDEAL_CMD }
,{jjKBASE,      KBASE_CMD,       IDEAL_CMD,      IDEAL_CMD }
,{jjKBASE,      KBASE_CMD,       MODUL_CMD,      MODUL_CMD }
,{atKILLATTR1,  KILLATTR_CMD,    NONE,           IDHDL }
,{jjCALL1MANY,  LIST_CMD,        LIST_CMD,       DEF_CMD }
#ifdef MDEBUG
,{jjpHead,      LEAD_CMD,        POLY_CMD,       POLY_CMD }
#else
,{jjpHead,      LEAD_CMD,       -((s)POLY_CMD),  POLY_CMD }
#endif
,{jjidHead,     LEAD_CMD,       -((s)IDEAL_CMD), IDEAL_CMD }
#ifdef MDEBUG
,{jjpHead,      LEAD_CMD,        VECTOR_CMD,     VECTOR_CMD }
#else
,{jjpHead,      LEAD_CMD,       -((s)VECTOR_CMD),VECTOR_CMD }
#endif
,{jjidHead,     LEAD_CMD,       -((s)MODUL_CMD), MODUL_CMD }
,{jjLEADCOEF,   LEADCOEF_CMD,    NUMBER_CMD,     POLY_CMD }
,{jjLEADCOEF,   LEADCOEF_CMD,    NUMBER_CMD,     VECTOR_CMD }
,{jjLEADEXP,    LEADEXP_CMD,     INTVEC_CMD,     POLY_CMD }
,{jjLEADEXP,    LEADEXP_CMD,     INTVEC_CMD,     VECTOR_CMD }
,{jjWRONG,      MAP_CMD,         0,              ANY_TYPE}
,{jjDUMMY,      MATRIX_CMD,      MATRIX_CMD,     MATRIX_CMD }
,{jjidMaxIdeal, MAXID_CMD,      -((s)IDEAL_CMD), INT_CMD }
,{jjMEMORY,     MEMORY_CMD,      INT_CMD,        INT_CMD }
,{jjidMinBase,  MINBASE_CMD,    -((s)IDEAL_CMD), IDEAL_CMD }
,{jjidMinBase,  MINBASE_CMD,    -((s)MODUL_CMD), MODUL_CMD }
,{jjMINRES,     MINRES_CMD,      LIST_CMD,       LIST_CMD }
,{jjDUMMY,      MODUL_CMD,       MODUL_CMD,      MODUL_CMD }
,{jjMONITOR1,   MONITOR_CMD,     NONE,           STRING_CMD }
,{jjMULT,       MULTIPLICITY_CMD,  INT_CMD,      IDEAL_CMD }
,{jjMULT,       MULTIPLICITY_CMD,  INT_CMD,      MODUL_CMD }
,{jjMSTD,       MSTD_CMD,        LIST_CMD,       IDEAL_CMD }
,{jjNAMEOF,     NAMEOF_CMD,      STRING_CMD,     ANY_TYPE }
,{jjNAMES,      NAMES_CMD,       LIST_CMD,       RING_CMD }
,{jjNAMES,      NAMES_CMD,       LIST_CMD,       QRING_CMD }
,{jjDUMMY,      NUMBER_CMD,      NUMBER_CMD,     NUMBER_CMD }
,{jjP2N,        NUMBER_CMD,      NUMBER_CMD,     POLY_CMD }
,{jjRPAR,       NPARS_CMD,       INT_CMD,        RING_CMD }
,{jjRPAR,       NPARS_CMD,       INT_CMD,        QRING_CMD }
,{jjNVARS,      NVARS_CMD,       INT_CMD,        RING_CMD }
,{jjNVARS,      NVARS_CMD,       INT_CMD,        QRING_CMD }
,{jjOpenClose,  OPEN_CMD,        NONE,           LINK_CMD }
,{jjORD,        ORD_CMD,         INT_CMD,        POLY_CMD }
,{jjORD,        ORD_CMD,         INT_CMD,        VECTOR_CMD }
,{jjrOrdStr,    ORDSTR_CMD,     -((s)STRING_CMD),RING_CMD }
,{jjrOrdStr,    ORDSTR_CMD,     -((s)STRING_CMD),QRING_CMD }
,{jjPAR1,       PAR_CMD,         NUMBER_CMD,     INT_CMD }
,{jjPARDEG,     PARDEG_CMD,      INT_CMD,        NUMBER_CMD }
,{jjPARSTR1,    PARSTR_CMD,      STRING_CMD,     INT_CMD }
,{jjrParStr,    PARSTR_CMD,     -((s)STRING_CMD),RING_CMD }
,{jjrParStr,    PARSTR_CMD,     -((s)STRING_CMD),QRING_CMD }
,{jjDUMMY,      POLY_CMD,        POLY_CMD,       POLY_CMD }
,{jjPRIME,      PRIME_CMD,       INT_CMD,        INT_CMD }
,{jjPRINT_GEN,  PRINT_CMD,       NONE,           INT_CMD }
,{jjPRINT_INTVEC,PRINT_CMD,      NONE,           INTVEC_CMD }
,{jjPRINT_INTMAT,PRINT_CMD,      NONE,           INTMAT_CMD }
,{jjPRINT_GEN,  PRINT_CMD,       NONE,           STRING_CMD }
,{jjPRINT_GEN,  PRINT_CMD,       NONE,           POLY_CMD }
,{jjPRINT_V,    PRINT_CMD,       NONE,           VECTOR_CMD }
,{jjPRINT_GEN,  PRINT_CMD,       NONE,           IDEAL_CMD }
,{jjPRINT_MA,   PRINT_CMD,       NONE,           MATRIX_CMD }
,{jjPRINT_LIST, PRINT_CMD,       NONE,           LIST_CMD }
,{jjidMinEmbedding, PRUNE_CMD,  -((s)MODUL_CMD), MODUL_CMD }
,{kQHWeight,    QHWEIGHT_CMD,    INTVEC_CMD,     IDEAL_CMD }
,{kQHWeight,    QHWEIGHT_CMD,    INTVEC_CMD,     MODUL_CMD }
,{jjWRONG,      QRING_CMD,       0,              ANY_TYPE}
,{jjREAD,       READ_CMD,        STRING_CMD,     LINK_CMD }
,{jjREGULARITY, REGULARITY_CMD,  INT_CMD,        LIST_CMD }
,{jjRESERVEDNAME,RESERVEDNAME_CMD, INT_CMD,      STRING_CMD }
,{jjWRONG,      ROWS_CMD,        0,              POLY_CMD }
,{jjpMaxComp,   ROWS_CMD,       -((s)INT_CMD),   VECTOR_CMD }
,{jjROWS,       ROWS_CMD,        INT_CMD,        MODUL_CMD }
,{jjROWS,       ROWS_CMD,        INT_CMD,        MATRIX_CMD }
,{jjROWS_IV,    ROWS_CMD,        INT_CMD,        INTMAT_CMD }
,{jjCOUNT_IV,   ROWS_CMD,        INT_CMD,        INTVEC_CMD }
,{jjSort_Id,    SORTVEC_CMD,     INTVEC_CMD,     IDEAL_CMD }
,{jjSort_Id,    SORTVEC_CMD,     INTVEC_CMD,     MODUL_CMD }
,{jjSTD,        STD_CMD,         IDEAL_CMD,      IDEAL_CMD }
,{jjSTD,        STD_CMD,         MODUL_CMD,      MODUL_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     STRING_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     INT_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     POLY_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     VECTOR_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     NUMBER_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     MATRIX_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     MODUL_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     IDEAL_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     MAP_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     INTVEC_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     INTMAT_CMD }
,{jjSTRING_PROC,STRING_CMD,      STRING_CMD,     PROC_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     RING_CMD }
,{jjSTRING,     STRING_CMD,      STRING_CMD,     QRING_CMD }
,{jjSYSTEM,     SYSTEM_CMD,      NONE,           STRING_CMD }
,{jjSYZYGY,     SYZYGY_CMD,      MODUL_CMD,      IDEAL_CMD }
,{jjSYZYGY,     SYZYGY_CMD,      MODUL_CMD,      MODUL_CMD }
,{jjWRONG,      TRACE_CMD,       0,              INTVEC_CMD }
,{jjWRONG,      TRACE_CMD,       0,              IDEAL_CMD }
,{jjTRACE_IV,   TRACE_CMD,       INT_CMD,        INTMAT_CMD }
,{jjmpTrace,    TRACE_CMD,      -((s)POLY_CMD),  MATRIX_CMD }
,{jjTRANSP_IV,  TRANSPOSE_CMD,   INTMAT_CMD,     INTVEC_CMD }
,{jjTRANSP_IV,  TRANSPOSE_CMD,   INTMAT_CMD,     INTMAT_CMD }
,{jjmpTransp,   TRANSPOSE_CMD,  -((s)MATRIX_CMD),MATRIX_CMD }
,{jjTYPEOF,     TYPEOF_CMD,      STRING_CMD,     ANY_TYPE }
,{jjDUMMY,      VECTOR_CMD,      VECTOR_CMD,     VECTOR_CMD }
,{jjVDIM,       VDIM_CMD,        INT_CMD,        IDEAL_CMD }
,{jjVDIM,       VDIM_CMD,        INT_CMD,        MODUL_CMD }
,{jjVAR1,       VAR_CMD,         POLY_CMD,       INT_CMD }
,{jjVARSTR1,    VARSTR_CMD,      STRING_CMD,     INT_CMD }
,{jjrVarStr,    VARSTR_CMD,     -((s)STRING_CMD),RING_CMD }
,{jjrVarStr,    VARSTR_CMD,     -((s)STRING_CMD),QRING_CMD }
,{kWeight,      WEIGHT_CMD,      INTVEC_CMD,     IDEAL_CMD }
,{kWeight,      WEIGHT_CMD,      INTVEC_CMD,     MODUL_CMD }
,{NULL,         0,               0,              0}
};
#undef s
/*=================== operations with 3 args.: static proc =================*/
static BOOLEAN jjBRACK_S(leftv res, leftv u, leftv v,leftv w)
{
  char *s= (char *)u->Data();
  int   r = (int)v->Data();
  int   c = (int)w->Data();
  int l = strlen(s);

  if ( (r<1) || (r>l) || (c<0) )
  {
    Werror("wrong range[%d,%d] in string %s",r,c,u->Name());
    return TRUE;
  }
  res->data = (char *)AllocL(c+1);
  sprintf((char *)res->data,"%-*.*s",c,c,s+r-1);
  return FALSE;
}
static BOOLEAN jjBRACK_Im(leftv res, leftv u, leftv v,leftv w)
{
  intvec *iv = (intvec *)u->Data();
  int   r = (int)v->Data();
  int   c = (int)w->Data();
  if ((r<1)||(r>iv->rows())||(c<1)||(c>iv->cols()))
  {
    Werror("wrong range[%d,%d] in intmat %s(%d x %d)",
           r,c,u->Name(),iv->rows(),iv->cols());
    return TRUE;
  }
  res->data=u->data;
  u->data=NULL;
  res->rtyp=u->rtyp;
  u->rtyp=0;
  res->name=u->name;
  u->name=NULL;
  Subexpr e=jjMakeSub(v);
          e->next=jjMakeSub(w);
  if (u->e==NULL) res->e=e;
  else
  {
    Subexpr h=u->e;
    while (h->next!=NULL) h=h->next;
    h->next=e;
    res->e=u->e;
    u->e=NULL;
  }
  return FALSE;
}
static BOOLEAN jjBRACK_Ma(leftv res, leftv u, leftv v,leftv w)
{
  matrix m= (matrix)u->Data();
  int   r = (int)v->Data();
  int   c = (int)w->Data();
  //Print("gen. elem %d, %d\n",r,c);
  if ((r<1)||(r>MATROWS(m))||(c<1)||(c>MATCOLS(m)))
  {
    Werror("wrong range[%d,%d] in matrix %s(%d x %d)",r,c,u->Name(),
      MATROWS(m),MATCOLS(m));
    return TRUE;
  }
  res->data=u->data;
  u->data=NULL;
  res->rtyp=u->rtyp;
  u->rtyp=0;
  res->name=u->name;
  u->name=NULL;
  Subexpr e=jjMakeSub(v);
          e->next=jjMakeSub(w);
  if (u->e==NULL)
    res->e=e;
  else
  {
    Subexpr h=u->e;
    while (h->next!=NULL) h=h->next;
    h->next=e;
    res->e=u->e;
    u->e=NULL;
  }
  return FALSE;
}
static BOOLEAN jjBRACK_Ma_I_IV(leftv res, leftv u, leftv v,leftv w)
{
  sleftv t;
  sleftv ut;
  leftv p=NULL;
  intvec *iv=(intvec *)w->Data();
  int l;
  BOOLEAN nok;

  if ((u->rtyp!=IDHDL)||(u->e!=NULL))
  {
    WerrorS("cannot build expression lists from unnamed objects");
    return TRUE;
  }
  memcpy(&ut,u,sizeof(ut));
  memset(&t,0,sizeof(t));
  t.rtyp=INT_CMD;
  for (l=0;l< iv->length(); l++)
  {
    t.data=(char *)(*iv)[l];
    if (p==NULL)
    {
      p=res;
    }
    else
    {
      p->next=(leftv)Alloc0(sizeof(sleftv));
      p=p->next;
    }
    memcpy(u,&ut,sizeof(ut));
    if (u->Typ() == MATRIX_CMD)
      nok=jjBRACK_Ma(p,u,v,&t);
    else /* INTMAT_CMD */
      nok=jjBRACK_Im(p,u,v,&t);
    if (nok)
    {
      while (res->next!=NULL)
      {
        p=res->next->next;
        Free((ADDRESS)res->next,sizeof(sleftv));
        // res->e aufraeumen !!!!
        res->next=p;
      }
      return TRUE;
    }
  }
  return FALSE;
}
static BOOLEAN jjBRACK_Ma_IV_I(leftv res, leftv u, leftv v,leftv w)
{
  sleftv t;
  sleftv ut;
  leftv p=NULL;
  intvec *iv=(intvec *)v->Data();
  int l;
  BOOLEAN nok;

  if ((u->rtyp!=IDHDL)||(u->e!=NULL))
  {
    WerrorS("cannot build expression lists from unnamed objects");
    return TRUE;
  }
  memcpy(&ut,u,sizeof(ut));
  memset(&t,0,sizeof(t));
  t.rtyp=INT_CMD;
  for (l=0;l< iv->length(); l++)
  {
    t.data=(char *)((*iv)[l]);
    if (p==NULL)
    {
      p=res;
    }
    else
    {
      p->next=(leftv)Alloc0(sizeof(sleftv));
      p=p->next;
    }
    memcpy(u,&ut,sizeof(ut));
    if (u->Typ() == MATRIX_CMD)
      nok=jjBRACK_Ma(p,u,&t,w);
    else /* INTMAT_CMD */
      nok=jjBRACK_Im(p,u,&t,w);
    if (nok)
    {
      while (res->next!=NULL)
      {
        p=res->next->next;
        Free((ADDRESS)res->next,sizeof(sleftv));
        // res->e aufraeumen !!
        res->next=p;
      }
      return TRUE;
    }
  }
  return FALSE;
}
static BOOLEAN jjBRACK_Ma_IV_IV(leftv res, leftv u, leftv v,leftv w)
{
  sleftv t1,t2,ut;
  leftv p=NULL;
  intvec *vv=(intvec *)v->Data();
  intvec *wv=(intvec *)w->Data();
  int vl;
  int wl;
  BOOLEAN nok;

  if ((u->rtyp!=IDHDL)||(u->e!=NULL))
  {
    WerrorS("cannot build expression lists from unnamed objects");
    return TRUE;
  }
  memcpy(&ut,u,sizeof(ut));
  memset(&t1,0,sizeof(sleftv));
  memset(&t2,0,sizeof(sleftv));
  t1.rtyp=INT_CMD;
  t2.rtyp=INT_CMD;
  for (vl=0;vl< vv->length(); vl++)
  {
    t1.data=(char *)((*vv)[vl]);
    for (wl=0;wl< wv->length(); wl++)
    {
      t2.data=(char *)((*wv)[wl]);
      if (p==NULL)
      {
        p=res;
      }
      else
      {
        p->next=(leftv)Alloc0(sizeof(sleftv));
        p=p->next;
      }
      memcpy(u,&ut,sizeof(ut));
      if (u->Typ() == MATRIX_CMD)
        nok=jjBRACK_Ma(p,u,&t1,&t2);
      else /* INTMAT_CMD */
        nok=jjBRACK_Im(p,u,&t1,&t2);
      if (nok)
      {
        res->CleanUp();
        return TRUE;
      }
    }
  }
  return FALSE;
}
static BOOLEAN jjCALL3MANY(leftv res, leftv u, leftv v, leftv w)
{
  u->next=(leftv)Alloc(sizeof(sleftv));
  memcpy(u->next,v,sizeof(sleftv));
  u->next->next=(leftv)Alloc(sizeof(sleftv));
  memcpy(u->next->next,w,sizeof(sleftv));
  BOOLEAN r=iiExprArithM(res,u,iiOp);
  v->rtyp=0; v->data=NULL; // iiExprArithM did the CleanUp
  w->rtyp=0; w->data=NULL;
  return r;
}
static BOOLEAN jjCOEFFS3_Id(leftv res, leftv u, leftv v, leftv w)
{
  if ((w->rtyp!=IDHDL)||(w->e!=NULL))
  {
    WerrorS("3rd argument must be a name of a matrix");
    return TRUE;
  }
  ideal i=(ideal)u->Data();
  int rank=(int)i->rank;
  BOOLEAN r=jjCOEFFS_Id(res,u,v);
  if (r) return TRUE;
  mpMonomials((matrix)res->data, rank, pVar((poly)v->Data()),(matrix)w->Data());
  return FALSE;
}
static BOOLEAN jjCOEFFS3_KB(leftv res, leftv u, leftv v, leftv w)
{
  res->data=(void*)idCoeffOfKBase((ideal)(u->Data()),
           (ideal)(v->Data()),(poly)(w->Data()));
  return FALSE;
}
static BOOLEAN jjCOEFFS3_P(leftv res, leftv u, leftv v, leftv w)
{
  if ((w->rtyp!=IDHDL)||(w->e!=NULL))
  {
    WerrorS("3rd argument must be a name of a matrix");
    return TRUE;
  }
  poly p=(poly)u->CopyD();
  ideal i=idInit(1,1);
  i->m[0]=p;
  sleftv t;
  memset(&t,0,sizeof(t));
  t.data=(char *)i;
  t.rtyp=IDEAL_CMD;
  int rank=1;
  if (u->Typ()==VECTOR_CMD)
  {
    i->rank=rank=pMaxComp(p);
    t.rtyp=MODUL_CMD;
  }
  BOOLEAN r=jjCOEFFS_Id(res,&t,v);
  t.CleanUp();
  if (r) return TRUE;
  mpMonomials((matrix)res->data, rank, pVar((poly)v->Data()),(matrix)w->Data());
  return FALSE;
}
static BOOLEAN jjELIMIN_HILB(leftv res, leftv u, leftv v, leftv w)
{
  res->data=(char *)idElimination((ideal)u->Data(),(poly)v->Data(),
    (intvec *)w->Data());
  setFlag(res,FLAG_STD);
  return FALSE;
}
static BOOLEAN jjFIND3(leftv res, leftv u, leftv v, leftv w)
{
  /*4
  * look for the substring what in the string where
  * starting at position n
  * return the position of the first char of what in where
  * or 0
  */
  int n=(int)w->Data();
  char *where=(char *)u->Data();
  char *what=(char *)v->Data();
  char *found;
  if ((1>n)||(n>(int)strlen(where)))
  {
    Werror("start position %d out of range",n);
    return TRUE;
  }
  found = strchr(where+n-1,*what);
  if (*(what+1)!='\0')
  {
    while((found !=NULL) && (strncmp(found+1,what+1,strlen(what+1))!=0))
    {
      found=strchr(found+1,*what);
    }
  }
  if (found != NULL)
  {
    res->data=(char *)((found-where)+1);
  }
  return FALSE;
}
static BOOLEAN jjJET_P_IV(leftv res, leftv u, leftv v, leftv w)
{
  short *iw=iv2array((intvec *)w->Data());
  res->data = (char *)pJetW((poly)u->Data(),(int)v->Data(),iw);
  Free((ADDRESS)iw,(pVariables+1)*sizeof(short));
  return FALSE;
}
static BOOLEAN jjJET_ID_IV(leftv res, leftv u, leftv v, leftv w)
{
  res->data = (char *)idJetW((ideal)u->Data(),(int)v->Data(),
                             (intvec *)w->Data());
  return FALSE;
}
static BOOLEAN jjPREIMAGE(leftv res, leftv u, leftv v, leftv w)
{
  idhdl h,rr;
  map mapping;

  rr=(idhdl)u->data;
  if ((h=IDRING(rr)->idroot->get(v->name,myynest))!=NULL)
  {
    if (h->typ==MAP_CMD)
    {
      mapping=IDMAP(h);
    }
    else if (h->typ==IDEAL_CMD)
    {
      mapping=IDMAP(h);
    }
    else
    {
      Werror("`%s` is no map nor ideal",IDID(h));
      return TRUE;
    }
  }
  else
  {
    Werror("`%s` is not defined in `%s`",v->name,u->name);
    return TRUE;
  }
  if ((h=IDRING(rr)->idroot->get(w->name,myynest))!=NULL)
  {
    if (h->typ==IDEAL_CMD)
    {
      res->data=(char *)maGetPreimage(IDRING(rr),mapping,IDIDEAL(h));
    }
    else
    {
      Werror("`%s` is no ideal",IDID(h));
      return TRUE;
    }
  }
  else
  {
    Werror("`%s` is not defined in `%s`",w->name,u->name);
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjRANDOM_Im(leftv res, leftv u, leftv v, leftv w)
{
  int di, k;
  int i=(int)u->Data();
  int r=(int)v->Data();
  int c=(int)w->Data();
  intvec *iv = new intvec(r, c, 0);
  if (iv->rows()==0)
  {
    delete iv;
    return TRUE;
  }
  if (i!=0)
  {
    if (i<0) i = -i;
    di = 2 * i + 1;
    for (k=0; k<iv->length(); k++)
    {
#ifdef buildin_rand
      (*iv)[k] = ((siRand() % di) - i);
#else
      (*iv)[k] = ((rand() % di) - i);
#endif
    }
  }
  res->data = (char *)iv;
  return FALSE;
}
static BOOLEAN jjSUBST_Test(leftv v,leftv w,
  int &ringvar, poly &monomexpr)
{
  monomexpr=(poly)w->Data();
  if (pLength(monomexpr)>1)
  {
    Werror("`%s` substitutes a ringvar only by a term",
      Tok2Cmdname(SUBST_CMD));
    return TRUE;
  }
  if (!(ringvar=pVar((poly)v->Data())))
  {
    WerrorS("ringvar expected");
    return TRUE;
  }
  return FALSE;
}
static BOOLEAN jjSUBST_P(leftv res, leftv u, leftv v,leftv w)
{
  int ringvar;
  poly monomexpr;
  BOOLEAN nok=jjSUBST_Test(v,w,ringvar,monomexpr);
  if (nok) return TRUE;
  res->data = pSubst((poly)u->CopyD(POLY_CMD),ringvar,monomexpr);
  return FALSE;
}
static BOOLEAN jjSUBST_Id(leftv res, leftv u, leftv v,leftv w)
{
  int ringvar;
  poly monomexpr;
  BOOLEAN nok=jjSUBST_Test(v,w,ringvar,monomexpr);
  if (nok) return TRUE;
  res->data = idSubst((ideal)u->CopyD(IDEAL_CMD),ringvar,monomexpr);
  return FALSE;
}
static BOOLEAN jjMATRIX_Id(leftv res, leftv u, leftv v,leftv w)
{
  matrix m=mpNew((int)v->Data(),(int)w->Data());
  ideal I=(ideal)u->CopyD(IDEAL_CMD);
  int i=min(IDELEMS(I),(int)v->Data()*(int)w->Data());
  for(i=i-1;i>=0;i--)
  {
    m->m[i]=I->m[i];
    I->m[i]=NULL;
  }
  idDelete(&I);
  res->data = (char *)m;
  return FALSE;
}
static BOOLEAN jjMATRIX_Mo(leftv res, leftv u, leftv v,leftv w)
{
  res->data = (char *)idModule2formatedMatrix((ideal)u->CopyD(MODUL_CMD),
           (int)v->Data(),(int)w->Data());
  return FALSE;
}
static BOOLEAN jjMATRIX_Ma(leftv res, leftv u, leftv v,leftv w)
{
  matrix m=mpNew((int)v->Data(),(int)w->Data());
  matrix I=(matrix)u->CopyD(MATRIX_CMD);
  int r=min(MATROWS(I),(int)v->Data());
  int c=min(MATCOLS(I),(int)w->Data());
  int i,j;
  for(i=r;i>0;i--)
  {
    for(j=c;j>0;j--)
    {
      MATELEM(m,i,j)=MATELEM(I,i,j);
      MATELEM(I,i,j)=NULL;
    }
  }
  idDelete((ideal *)&I);
  res->data = (char *)m;
  return FALSE;
}
static BOOLEAN jjREDUCE3_P(leftv res, leftv u, leftv v, leftv w)
{
  assumeStdFlag(v);
  res->data = (char *)kNF((ideal)v->Data(),currQuotient,(poly)u->Data(),
    0,(int)w->Data());
  return FALSE;
}
static BOOLEAN jjREDUCE3_ID(leftv res, leftv u, leftv v, leftv w)
{
  assumeStdFlag(v);
  res->data = (char *)kNF((ideal)v->Data(),currQuotient,(ideal)u->Data(),
    0,(int)w->Data());
  return FALSE;
}
static BOOLEAN jjRES3(leftv res, leftv u, leftv v, leftv w)
{
#ifdef OLD_RES
  int maxl=(int)v->Data();
  int l=0;
  resolvente r;
  intvec **weights=NULL;
  int wmaxl=maxl;
  maxl--;
  if ((maxl==-1) && (iiOp!=MRES_CMD))
    maxl = pVariables-1;
  if ((iiOp == RES_CMD) || (iiOp == MRES_CMD))
  {
    if (BTEST1(28))
    {
      r=syMinRes((ideal)u->Data(),maxl,&l,iiOp==MRES_CMD);
    }
    else
    {
      intvec * iv=(intvec*)atGet(u,"isHomog");
      if (iv!=NULL)
      {
        weights = (intvec**)Alloc0(sizeof(intvec*));
        weights[0] = ivCopy(iv);
        l=1;
      }
      r=syResolvente((ideal)u->Data(),maxl,&l, &weights, iiOp==MRES_CMD);
    }
  }
  else
    r=sySchreyerResolvente((ideal)u->Data(),maxl,&l);
  if (r==NULL) return TRUE;
  int t3=u->Typ();
  iiMakeResolv(r,l,wmaxl,w->name,t3,weights);
  return FALSE;
#else
  return TRUE;
#endif
}
static BOOLEAN jjSTATUS3(leftv res, leftv u, leftv v, leftv w)
{
  int yes;
  jjSTATUS2(res, u, v);
  yes = (strcmp((char *) res->data, (char *) w->Data()) == 0);
  FreeL((ADDRESS) res->data);
  res->data = (void *) yes;
  return FALSE;
}

/*=================== operations with 3 args.: table =================*/
struct sValCmd3 dArith3[]=
{
// operations:
// proc             cmd          res         arg1        arg2        arg3
 {jjBRACK_S,        '[',        STRING_CMD, STRING_CMD, INT_CMD,    INT_CMD }
,{jjBRACK_Im,       '[',        INT_CMD,    INTMAT_CMD, INT_CMD,    INT_CMD }
,{jjBRACK_Ma_I_IV,  '[',        INT_CMD,    INTMAT_CMD, INT_CMD,    INTVEC_CMD }
,{jjBRACK_Ma_IV_I,  '[',        INT_CMD,    INTMAT_CMD, INTVEC_CMD, INT_CMD }
,{jjBRACK_Ma_IV_IV, '[',        INT_CMD,    INTMAT_CMD, INTVEC_CMD, INTVEC_CMD }
,{jjBRACK_Ma,       '[',        POLY_CMD,   MATRIX_CMD, INT_CMD,    INT_CMD }
,{jjBRACK_Ma_I_IV,  '[',        POLY_CMD,   MATRIX_CMD, INT_CMD,    INTVEC_CMD }
,{jjBRACK_Ma_IV_I,  '[',        POLY_CMD,   MATRIX_CMD, INTVEC_CMD, INT_CMD }
,{jjBRACK_Ma_IV_IV, '[',        POLY_CMD,   MATRIX_CMD, INTVEC_CMD, INTVEC_CMD }
,{atATTRIB3,        ATTRIB_CMD, NONE,       IDHDL,      STRING_CMD, DEF_CMD }
,{jjCOEFFS3_P,      COEFFS_CMD, MATRIX_CMD, POLY_CMD,   POLY_CMD,   MATRIX_CMD }
,{jjCOEFFS3_P,      COEFFS_CMD, MATRIX_CMD, VECTOR_CMD, POLY_CMD,   MATRIX_CMD }
,{jjCOEFFS3_Id,     COEFFS_CMD, MATRIX_CMD, IDEAL_CMD,  POLY_CMD,   MATRIX_CMD }
,{jjCOEFFS3_Id,     COEFFS_CMD, MATRIX_CMD, MODUL_CMD,  POLY_CMD,   MATRIX_CMD }
,{jjCOEFFS3_KB,     COEFFS_CMD, MATRIX_CMD, IDEAL_CMD,  IDEAL_CMD,  POLY_CMD }
,{jjCOEFFS3_KB,     COEFFS_CMD, MATRIX_CMD, MODUL_CMD,  MODUL_CMD,  POLY_CMD }
,{jjELIMIN_HILB,    ELIMINATION_CMD,IDEAL_CMD, IDEAL_CMD, POLY_CMD, INTVEC_CMD }
,{jjFIND3,          FIND_CMD,   INT_CMD,    STRING_CMD, STRING_CMD, INT_CMD }
,{jjCALL3MANY,      IDEAL_CMD,  IDEAL_CMD,  DEF_CMD,    DEF_CMD,    DEF_CMD }
//,{jjCALL3MANY,      INTERSECT_CMD,  NONE,   DEF_CMD,    DEF_CMD,    DEF_CMD }
,{lInsert3,         INSERT_CMD, LIST_CMD,   LIST_CMD,   DEF_CMD,    INT_CMD }
,{jjCALL3MANY,      INTVEC_CMD, INTVEC_CMD, DEF_CMD,    DEF_CMD,    DEF_CMD }
,{jjJET_P_IV,       JET_CMD,    POLY_CMD,   POLY_CMD,   INT_CMD,    INTVEC_CMD }
,{jjJET_ID_IV,      JET_CMD,    IDEAL_CMD,  IDEAL_CMD,  INT_CMD,    INTVEC_CMD }
,{jjJET_P_IV,       JET_CMD,    VECTOR_CMD, VECTOR_CMD, INT_CMD,    INTVEC_CMD }
,{jjJET_ID_IV,      JET_CMD,    MODUL_CMD,  MODUL_CMD,  INT_CMD,    INTVEC_CMD }
,{mpKoszul,         KOSZUL_CMD, MATRIX_CMD, INT_CMD,    INT_CMD,    IDEAL_CMD }
,{jjCALL3MANY,      LIST_CMD,   LIST_CMD,   DEF_CMD,    DEF_CMD,    DEF_CMD }
,{jjMATRIX_Id,      MATRIX_CMD, MATRIX_CMD, IDEAL_CMD,  INT_CMD,    INT_CMD }
,{jjMATRIX_Mo,      MATRIX_CMD, MATRIX_CMD, MODUL_CMD,  INT_CMD,    INT_CMD }
,{jjMATRIX_Ma,      MATRIX_CMD, MATRIX_CMD, MATRIX_CMD, INT_CMD,    INT_CMD }
,{jjCALL3MANY,      MODUL_CMD,  MODUL_CMD,  DEF_CMD,    DEF_CMD,    DEF_CMD }
,{jjRES3,           MRES_CMD,   NONE,       IDEAL_CMD,  INT_CMD,    ANY_TYPE }
,{jjRES3,           MRES_CMD,   NONE,       MODUL_CMD,  INT_CMD,    ANY_TYPE }
,{jjPREIMAGE,       PREIMAGE_CMD, IDEAL_CMD, RING_CMD,  ANY_TYPE,   ANY_TYPE }
,{jjRANDOM_Im,      RANDOM_CMD, INTMAT_CMD, INT_CMD,    INT_CMD,    INT_CMD }
,{jjREDUCE3_P,      REDUCE_CMD, POLY_CMD,   POLY_CMD,   IDEAL_CMD,  INT_CMD }
,{jjREDUCE3_P,      REDUCE_CMD, VECTOR_CMD, VECTOR_CMD, IDEAL_CMD,  INT_CMD }
,{jjREDUCE3_P,      REDUCE_CMD, VECTOR_CMD, VECTOR_CMD, MODUL_CMD,  INT_CMD }
,{jjREDUCE3_ID,     REDUCE_CMD, IDEAL_CMD,  IDEAL_CMD,  IDEAL_CMD,  INT_CMD }
,{jjREDUCE3_ID,     REDUCE_CMD, MODUL_CMD,  MODUL_CMD,  MODUL_CMD,  INT_CMD }
,{jjREDUCE3_ID,     REDUCE_CMD, MODUL_CMD,  MODUL_CMD,  IDEAL_CMD,  INT_CMD }
,{jjRES3,           RES_CMD,    NONE,       IDEAL_CMD,  INT_CMD,    ANY_TYPE }
,{jjRES3,           RES_CMD,    NONE,       MODUL_CMD,  INT_CMD,    ANY_TYPE }
#ifdef HAVE_LIBFACTORY
,{jjRESULTANT,      RESULTANT_CMD, POLY_CMD,POLY_CMD,   POLY_CMD,   POLY_CMD }
#else
,{jjWRONG3,         RESULTANT_CMD, POLY_CMD,POLY_CMD,   POLY_CMD,   POLY_CMD }
#endif
,{jjRES3,           SRES_CMD,   NONE,       IDEAL_CMD,  INT_CMD,    ANY_TYPE }
,{jjRES3,           SRES_CMD,   NONE,       MODUL_CMD,  INT_CMD,    ANY_TYPE }
,{jjSTATUS3,        STATUS_CMD, INT_CMD,    LINK_CMD,   STRING_CMD, STRING_CMD}
,{jjSUBST_P,        SUBST_CMD,  POLY_CMD,   POLY_CMD,   POLY_CMD,   POLY_CMD }
,{jjSUBST_P,        SUBST_CMD,  VECTOR_CMD, VECTOR_CMD, POLY_CMD,   POLY_CMD }
,{jjSUBST_Id,       SUBST_CMD,  IDEAL_CMD,  IDEAL_CMD,  POLY_CMD,   POLY_CMD }
,{jjSUBST_Id,       SUBST_CMD,  MODUL_CMD,  MODUL_CMD,  POLY_CMD,   POLY_CMD }
,{jjSUBST_Id,       SUBST_CMD,  MATRIX_CMD, MATRIX_CMD, POLY_CMD,   POLY_CMD }
,{jjCALL3MANY,      SYSTEM_CMD, NONE,       STRING_CMD, DEF_CMD,    DEF_CMD }
,{NULL,             0,          0,          0,          0,          0 }
};
/*=================== operations with many arg.: static proc =================*/
static BOOLEAN jjCALL1ARG(leftv res, leftv v)
{
  return iiExprArith1(res,v,iiOp);
}
static BOOLEAN jjCALL2ARG(leftv res, leftv u)
{
  leftv v=u->next;
  u->next=NULL;
  BOOLEAN b=iiExprArith2(res,u,iiOp,v);
  u->next=v;
  return b;
}
static BOOLEAN jjCOEF_M(leftv res, leftv v)
{
  if((v->Typ() != VECTOR_CMD)
  || (v->next->Typ() != POLY_CMD)
  || (v->next->next->Typ() != MATRIX_CMD)
  || (v->next->next->next->Typ() != MATRIX_CMD))
     return TRUE;
  if (v->next->next->rtyp!=IDHDL) return TRUE;
  idhdl c=(idhdl)v->next->next->data;
  if (v->next->next->next->rtyp!=IDHDL) return TRUE;
  idhdl m=(idhdl)v->next->next->next->data;
  idDelete((ideal *)&(c->data.uideal));
  idDelete((ideal *)&(m->data.uideal));
  mpCoef2((poly)v->Data(),(poly)v->next->Data(),
    (matrix *)&(c->data.umatrix),(matrix *)&(m->data.umatrix));
  return FALSE;
}
static BOOLEAN jjIDEAL_PL(leftv res, leftv v)
{
  int s=1;
  leftv h=v;
  if (h!=NULL) s=exprlist_length(h);
  ideal id=idInit(s,1);
  int rank=1;
  int i=0;
  poly p=NULL;
  while (h!=NULL)
  {
    switch(h->Typ())
    {
      case POLY_CMD:
      {
        p=(poly)h->CopyD();
        break;
      }
      case INT_CMD:
      {
        number n=nInit((int)h->Data());
        if (!nIsZero(n))
        {
          p=pOne();
          pSetCoeff(p,n);
        }
        else
        {
          //p=NULL;
          nDelete(&n);
        }
        break;
      }
      case NUMBER_CMD:
      {
        number n=(number)h->CopyD();
        if (!nIsZero(n))
        {
          p=pOne();
          pSetCoeff(p,n);
        }
        else
        {
          //p=NULL;
          nDelete(&n);
        }
        break;
      }
      case VECTOR_CMD:
      {
        p=(poly)h->CopyD();
        if (iiOp!=MODUL_CMD)
        {
          idDelete(&id);
          pDelete(&p);
          return TRUE;
        }
        rank=max(rank,pMaxComp(p));
        break;
      }
      default:
      {
        idDelete(&id);
        return TRUE;
      }
    }
    if ((iiOp==MODUL_CMD)&&(p!=NULL)&&(pGetComp(p)==0))
    {
      pSetCompP(p,1);
    }
    id->m[i]=p;
    i++;
    h=h->next;
  }
  id->rank=rank;
  res->data=(char *)id;
  return FALSE;
}
static BOOLEAN jjINTERSECT_PL(leftv res, leftv v)
{
  leftv h=v;
  int l=v->listLength();
  resolvente r=(resolvente)Alloc0(l*sizeof(ideal));
  BOOLEAN *copied=(BOOLEAN *)Alloc0(l*sizeof(BOOLEAN));
  int t=0;
  // try to convert to IDEAL_CMD
  while (h!=NULL)
  {
    if (iiTestConvert(h->Typ(),IDEAL_CMD)!=0)
    {
      t=IDEAL_CMD;
    }
    else break;
    h=h->next;
  }
  // if failure, try MODUL_CMD
  if (t==0)
  {
    h=v;
    while (h!=NULL)
    {
      if (iiTestConvert(h->Typ(),MODUL_CMD)!=0)
      {
        t=MODUL_CMD;
      }
      else break;
      h=h->next;
    }
  }
  // check for success  in converting
  if (t==0)
  {
    WerrorS("cannot convert to ideal or module");
    return TRUE;
  }
  // call idMultSect
  h=v;
  int i=0;
  sleftv tmp;
  while (h!=NULL)
  {
    if (h->Typ()==t)
    {
      r[i]=(ideal)h->Data(); /*no copy*/
      h=h->next;
    }
    else if(iiConvert(h->Typ(),t,iiTestConvert(h->Typ(),t),h,&tmp))
    {
      Free((ADDRESS)copied,l*sizeof(BOOLEAN));
      Free((ADDRESS)r,l*sizeof(ideal));
      Werror("cannot convert arg. %d to %s",i+1,Tok2Cmdname(t));
      return TRUE;
    }
    else
    {
      r[i]=(ideal)tmp.Data(); /*now it's a copy*/
      copied[i]=TRUE;
      h=tmp.next;
    }
    i++;
  }
  res->rtyp=t;
  res->data=(char *)idMultSect(r,i);
  while(i>0)
  {
    i--;
    if (copied[i]) idDelete(&(r[i]));
  }
  Free((ADDRESS)copied,l*sizeof(BOOLEAN));
  Free((ADDRESS)r,l*sizeof(ideal));
  return FALSE;
}
static BOOLEAN jjINTVEC_PL(leftv res, leftv v)
{
  int i=0;
  leftv h=v;
  if (h!=NULL) i=exprlist_length(h);
  intvec *iv=new intvec(i);
  i=0;
  while (h!=NULL)
  {
    if(h->Typ()==INT_CMD)
    {
      (*iv)[i]=(int)h->Data();
    }
    else
    {
      delete iv;
      return TRUE;
    }
    i++;
    h=h->next;
  }
  res->data=(char *)iv;
  return FALSE;
}
static BOOLEAN jjKLAMMER_PL(leftv res, leftv u)
{
  leftv v=u->next;
  BOOLEAN b;
  if(v==NULL)
    b=iiExprArith1(res,u,iiOp);
  else
  {
    u->next=NULL;
    b=iiExprArith2(res,u,iiOp,v);
    u->next=v;
  }  
  return b;
}
static BOOLEAN jjLIST_PL(leftv res, leftv v)
{
  int sl=0;
  if (v!=NULL) sl = v->listLength();
  lists L=(lists)Alloc(sizeof(slists));
  leftv h=NULL;
  int i;
  int rt;

  L->Init(sl);
  for (i=0;i<sl;i++)
  {
    if (h!=NULL) { /* e.g. not in the first step:
                   * h is the pointer to the old sleftv,
                   * v is the pointer to the next sleftv
                   * (in this moment) */
                   h->next=v;
                 }
    h=v;
    v=v->next;
    h->next=NULL;
    rt=h->Typ();
    if (rt==0)
    {
      L->Clean();
      Werror("`%s` is undefined",h->Name());
      return TRUE;
    }
    if ((rt==RING_CMD)||(rt==QRING_CMD))
    {
      L->m[i].rtyp=rt;
      L->m[i].data=h->Data();
      ((ring)L->m[i].data)->ref++;
    }
    else
      L->m[i].Copy(h);
  }
  res->data=(char *)L;
  return FALSE;
}
static BOOLEAN jjNAMES0(leftv res, leftv v)
{
  res->data=(void *)ipNameList(idroot);
  return FALSE;
}
static BOOLEAN jjOPTION_PL(leftv res, leftv v)
{
  if(v==NULL)
  {
    showOption(iiOp);
    return FALSE;
  }
  return setOption(res,v);
}
static BOOLEAN jjRESERVED0(leftv res, leftv v)
{
  int i=1;
  while(cmds[i].name!=NULL)
  {
    PrintS(cmds[i].name);PrintLn();
    i++;
  }
  return FALSE;
}
static BOOLEAN jjSTRING_PL(leftv res, leftv v)
{
  if ((v!=NULL)&&(v->next==NULL))
    return iiExprArith1(res,v,iiOp);
  int sl=0;
  leftv h=v,nachher;
  sleftv tmpR;
  BOOLEAN bo;
  char *sum=NULL;
  while(h!=NULL)
  {
    /* convert to string =================================*/
    nachher=h->next;
    h->next=NULL;
    bo=iiExprArith1(&tmpR,h,iiOp);
    if(bo)
    {
      h->next=nachher;
      FreeL(sum);
      return TRUE;
    }
    /* append to 'sum' =================================*/
    if(sum==NULL)
    {
      sum=(char *)tmpR.data;
      sl=strlen(sum);
    }
    else
    {
      int nl=strlen((char *)tmpR.data);
      char *s=(char *)AllocL(sl+nl+1);
      memcpy(s,sum,sl);
      memcpy(s+sl,(char *)tmpR.data,nl);
      s[sl+nl+1]='\0';
      sl+=nl;
      FreeL(sum);
      sum=s;
    }
    /* next sleftv =================================*/
    h->next=nachher;
    h=nachher;
  }
  if (sum==NULL) sum=mstrdup("");
  res->data=(char *)sum;
  return FALSE;
}
static BOOLEAN jjTEST(leftv res, leftv v)
{
  do
  {
    if (v->Typ()!=INT_CMD)
      return TRUE;
    test_cmd((int)v->Data());
    v=v->next;
  }
  while (v!=NULL);
  return FALSE;
}
/*=================== operations with many arg.: table =================*/
/* number_of_args:  -1: any, -2: any >0, .. */
struct sValCmdM dArithM[]=
{
// operations:
// proc         cmd               res            number_of_args
 {jjKLAMMER_PL,  '(',           ANY_TYPE,           -2  }
,{jjCALL2ARG,  COEF_CMD,        MATRIX_CMD,         2  }
,{jjCOEF_M,    COEF_CMD,        NONE,               4  }
,{jjDBPRINT,   DBPRINT_CMD,     NONE,               -2 }
,{jjCALL1ARG,  IDEAL_CMD,       IDEAL_CMD,          1  }
,{jjIDEAL_PL,  IDEAL_CMD,       IDEAL_CMD,          -1 }
,{jjCALL2ARG,  INTERSECT_CMD,   IDEAL_CMD,          2  }
,{jjINTERSECT_PL,INTERSECT_CMD, IDEAL_CMD,          -2 }
,{jjCALL1ARG,  INTVEC_CMD,      INTVEC_CMD,         1  }
,{jjINTVEC_PL, INTVEC_CMD,      INTVEC_CMD,         -1 }
,{jjLIST_PL,   LIST_CMD,        LIST_CMD,           -1 }
,{jjCALL1ARG,  MODUL_CMD,       MODUL_CMD,          1  }
,{jjIDEAL_PL,  MODUL_CMD,       MODUL_CMD,          -1 }
,{jjCALL1ARG,  NAMES_CMD,       LIST_CMD,            1 }
,{jjNAMES0,    NAMES_CMD,       LIST_CMD,            0 }
,{jjOPTION_PL, OPTION_CMD,      NONE/*or set by p*/,-1 }
,{jjCALL1ARG,  RESERVEDNAME_CMD, INT_CMD,            1 }
,{jjRESERVED0, RESERVEDNAME_CMD, NONE,               0 }
,{jjSTRING_PL, STRING_CMD,      STRING_CMD,         -1 }
,{jjSYSTEM,    SYSTEM_CMD,      NONE/*or set by p*/,-2 }
,{jjTEST,      TEST_CMD,        NONE,               -2 }
,{jjOPTION_PL, VERBOSE_CMD,     NONE/*or set by p*/,-1 }
,{iiWRITE,     WRITE_CMD,       NONE,               -2 }
,{NULL,        0,               0,                  0  }
};
#ifdef MDEBUG
static Subexpr jjDBMakeSub(leftv e,char *f, int l)
#else
static Subexpr jjMakeSub(leftv e)
#endif
{
  // assume: e->Typ()==INT_CMD
#ifdef MDEBUG
  Subexpr r=(Subexpr)mmDBAllocBlock0(sizeof(*r),f,l);
#else
  Subexpr r=(Subexpr)Alloc0(sizeof(*r));
#endif
  r->start =(int)e->Data();
  return r;
}

/*================ generating tables ============================*/
#ifdef GENTABLE
extern struct sValAssign dAssign[];
struct sValCmdTab dArithTab1[]={ {0,0}};
#define JJTAB1LEN 0
struct sValCmdTab dArithTab2[]={ {0,0}};
#define JJTAB2LEN 0
void ttGen1()
{
  FILE *outfile = fopen("iparith.inc","w");
  int i,j,l1=0,l2=0;
  currRing=(ring)Alloc(sizeof(*currRing));
  fprintf(outfile,
  "/****************************************\n"
  "*  Computer Algebra System SINGULAR     *\n"
  "****************************************/\n\n");
/*-------------------------------------------------------------------*/
  fprintf(outfile,"// syntax table for Singular\n//\n");
  fprintf(outfile,"// - search for an exact match of the argument types\n");
  fprintf(outfile,"// - otherwise search for the first possibility\n");
  fprintf(outfile,"//   with converted types of the arguments\n");
  fprintf(outfile,"// - otherwise report an error\n//\n");

  int op;
  i=0;
  while ((op=dArith1[i].cmd)!=0)
  {
    if (dArith1[i].p==jjWRONG)
      fprintf(outfile,"// DUMMY ");
    char *s = iiTwoOps(op);
    fprintf(outfile,"// operation: %s (%s)  ->  %s\n",
          s,
          Tok2Cmdname(dArith1[i].arg),
          Tok2Cmdname(ABS(dArith1[i].res)));
    i++;
  }
  fprintf(outfile,"/*---------------------------------------------*/\n");
  i=0;
  while ((op=dArith2[i].cmd)!=0)
  {
    if (dArith2[i].p==jjWRONG2)
      fprintf(outfile,"// DUMMY ");
    char *s = iiTwoOps(op);
    fprintf(outfile,"// operation: %s (%s, %s)  ->  %s\n",
          s,
          Tok2Cmdname(dArith2[i].arg1),
          Tok2Cmdname(dArith2[i].arg2),
          Tok2Cmdname(dArith2[i].res));
    i++;
  }
  fprintf(outfile,"/*---------------------------------------------*/\n");
  i=0;
  while ((op=dArith3[i].cmd)!=0)
  {
    char *s = iiTwoOps(op);
    if (dArith3[i].p==jjWRONG3)
      fprintf(outfile,"// DUMMY ");
    fprintf(outfile,"// operation: %s (%s, %s, %s)  ->  %s\n",
          s,
          Tok2Cmdname(dArith3[i].arg1),
          Tok2Cmdname(dArith3[i].arg2),
          Tok2Cmdname(dArith3[i].arg3),
          Tok2Cmdname(dArith3[i].res));
    i++;
  }
  fprintf(outfile,"/*---------------------------------------------*/\n");
  i=0;
  while ((op=dArithM[i].cmd)!=0)
  {
    char *s = iiTwoOps(op);
    fprintf(outfile,"// operation: %s (...)  ->  %s",
          s,
          Tok2Cmdname(dArithM[i].res));
    switch(dArithM[i].number_of_args)
    {
      case -2:
         fprintf(outfile," ( number of arguments >0 )\n");
         break;
      case -1:
         fprintf(outfile," ( any number of arguments )\n");
         break;
      default:
         fprintf(outfile," ( %d arguments )\n",dArithM[i].number_of_args);
         break;
    }
    i++;
  }
  fprintf(outfile,"/*---------------------------------------------*/\n");
  i=0;
  while ((op=dAssign[i].res)!=0)
  {
    fprintf(outfile,"// assign: %s =  %s\n",
          Tok2Cmdname(op/*dAssign[i].res*/),
          Tok2Cmdname(dAssign[i].arg));
    i++;
  }
/*-------------------------------------------------------------------*/
  fprintf(outfile,"/*---------------------------------------------*/\n");
  for (j=257;j<=MAX_TOK+1;j++)
  {
    for(i=257;i<=MAX_TOK+1;i++)
    {
      if ((i!=j) && (j!=IDHDL) && (j!=DEF_CMD) && (j!=ANY_TYPE)
      && iiTestConvert(i,j))
      {
        fprintf(outfile,"// convert %s -> %s\n",
          Tok2Cmdname(i), Tok2Cmdname(j));
        if (j==ANY_TYPE) break;
      }
    }
  }
  fprintf(outfile,"/*---------------------------------------------*/\n");
  char ops[]="=><+*/[.^,%(";
  for(i=0;ops[i]!='\0';i++)
    fprintf(outfile,"// token %d : %c\n", (int)ops[i], ops[i]);
  for (i=257;i<=MAX_TOK;i++)
  {
    char *s=iiTwoOps(i);
    if (s[0]!='$')
    {
      fprintf(outfile,"// token %d : %s\n", i, s);
    }
  }
/*-------------------------------------------------------------------*/
  fprintf(outfile,"/*--max. token: %d, gr: %d --*/\n",MAX_TOK,UMINUS);
/*-------------------------------------------------------------------*/
  fprintf(outfile,"/*---------------------------------------------*/\n");
  fprintf(outfile,
  "struct sValCmdTab dArithTab1[]=\n"
  "{\n");
  for (j=1;j<=MAX_TOK+1;j++)
  {
    for(i=0;dArith1[i].cmd!=0;i++)
    {
      if (dArith1[i].cmd==j)
      {
        fprintf(outfile," { %d,%d },\n",j,i);
        l1++;
        break;
      }
    }
  }
  fprintf(outfile," { 10000,0 }\n};\n");
  fprintf(outfile,"#define JJTAB1LEN %d\n",l1);
/*-------------------------------------------------------------------*/
  fprintf(outfile,
  "struct sValCmdTab dArithTab2[]=\n"
  "{\n");
  for (j=1;j<=MAX_TOK+1;j++)
  {
    for(i=0;dArith2[i].cmd!=0;i++)
    {
      if (dArith2[i].cmd==j)
      {
        fprintf(outfile," { %d,%d },\n",j,i);
        l2++;
        break;
      }
    }
  }
  fprintf(outfile," { 10000,0 }\n};\n");
  fprintf(outfile,"#define JJTAB2LEN %d\n",l2);
  fclose(outfile);
}
/*-------------------------------------------------------------------*/
void ttGen2()
{
  FILE *outfile = fopen("iparith.inc","a");
  fprintf(outfile,
  "/****************************************\n"
  "*  Computer Algebra System SINGULAR     *\n"
  "****************************************/\n\n");
/*-------------------------------------------------------------------*/
  fprintf(outfile,"// identifier table for Singular\n//\n");

  fprintf(outfile,
  "cmdnames cmds[] =\n"
  "{  // name-string     alias  tokval toktype\n"
  "{ \"$INVALID$\",            0,  -1, 0},\n");
  int i=1;
  int m=-1;
  int id_nr=0;
  BOOLEAN f=FALSE;
  loop
  {
    while (cmds[i].tokval!=0)
    {
      if ((cmds[i].tokval!=-1)&&(cmds[i].name!=NULL))
      {
        if(m==-1)
        {
          m=i;
          f=TRUE;
        }
        else if(strcmp(cmds[m].name,cmds[i].name)>0)
        {
          m=i;
          f=TRUE;
        }
      }
      i++;
    }
    if(f)
    {
      id_nr++;
      fprintf(outfile,"  {\"%s\", %*d, %3d, ",cmds[m].name,
                                             20-strlen(cmds[m].name),
                                             cmds[m].alias,
                                             cmds[m].tokval);
      switch(cmds[m].toktype)
      {
        case CMD_1:    fprintf(outfile,"CMD_1 },\n"); break;
        case CMD_2:    fprintf(outfile,"CMD_2 },\n"); break;
        case CMD_3:    fprintf(outfile,"CMD_3 },\n"); break;
        case CMD_12:   fprintf(outfile,"CMD_12 },\n"); break;
        case CMD_123 : fprintf(outfile,"CMD_123 },\n"); break;
        case CMD_23:   fprintf(outfile,"CMD_23 },\n"); break;
        case CMD_M:    fprintf(outfile,"CMD_M },\n"); break;
        case SYSVAR:   fprintf(outfile,"SYSVAR },\n"); break;
        case ROOT_DECL:   fprintf(outfile,"ROOT_DECL },\n"); break;
        case ROOT_DECL_LIST:   fprintf(outfile,"ROOT_DECL_LIST },\n"); break;
        case RING_DECL:   fprintf(outfile,"RING_DECL },\n"); break;
        case NONE:   fprintf(outfile,"NONE },\n"); break;
        default:       fprintf(outfile,"%d },\n",cmds[m].toktype);
      }
      cmds[m].name=NULL;
      m=-1;
      i=1;
      f=FALSE;
    }
    else break;
  }
  fprintf(outfile,
"/* list of scanner identifiers/only for feread/reservedName */\n");
  f=FALSE;
  i=1;m=-1;
  loop
  {
    while (cmds[i].tokval!=0)
    {
      if (cmds[i].name!=NULL)
      {
        if(m==-1)
        {
          m=i;
          f=TRUE;
        }
        else if(strcmp(cmds[m].name,cmds[i].name)>0)
        {
          m=i;
          f=TRUE;
        }
      }
      i++;
    }
    if(f)
    {
      fprintf(outfile,"  {\"%s\", %*d,  -1, 0 },\n",cmds[m].name,
                                             20-strlen(cmds[m].name),
                                             0/*cmds[m].alias*/
                                             /*-1 cmds[m].tokval*/
                                             /*0 cmds[m].toktype*/);
      cmds[m].name=NULL;
      m=-1;
      i=1;
      f=FALSE;
    }
    else break;
  }
  fprintf(outfile,
"/* end of list marker */\n"
"  { NULL, 0, 0, 0}\n"
"};\n"
"#define LAST_IDENTIFIER %d\n",id_nr);
  fclose(outfile);
}
/*-------------------------------------------------------------------*/
#if 0
void ttGen3()
{
  FILE *outfile = fopen("mpsr_tok.inc","w");
  fprintf(outfile,
  "/****************************************\n"
  "*  Computer Algebra System SINGULAR     *\n"
  "****************************************/\n\n");
/*-------------------------------------------------------------------*/
  fprintf(outfile,"// token table for Singular\n//\n");

  fprintf(outfile,
  "short vtok[] =\n"
  "{\n");
  // operations with 1 arg: ===========================================
  int i=0;
  while (dArith1[i].cmd!=0)
  {
    if ((dArith1[i].p!=jjWRONG)
    &&((i==0)||(dArith1[i].cmd!=dArith1[i-1].cmd)))
    {
      fprintf(outfile,"  %d,\n",dArith1[i].cmd);
    }
    i++;
  }
  // operations with 2 args: ===========================================
  i=0;
  while (dArith2[i].cmd!=0)
  {
    if ((dArith2[i].p!=jjWRONG2)
    &&((i==0)||(dArith2[i].cmd!=dArith2[i-1].cmd)))
    {
      fprintf(outfile,"  %d,\n",dArith2[i].cmd);
    }
    i++;
  }
  // operations with 3 args: ===========================================
  i=0;
  while (dArith3[i].cmd!=0)
  {
    if (
    ((i==0)||(dArith3[i].cmd!=dArith3[i-1].cmd)))
    {
      fprintf(outfile,"  %d,\n",dArith3[i].cmd);
    }
    i++;
  }
  // operations with many args: ===========================================
  i=0;
  while (dArithM[i].cmd!=0)
  {
    if (
    ((i==0)||(dArithM[i].cmd!=dArithM[i-1].cmd)))
    {
      fprintf(outfile,"  %d,\n",dArithM[i].cmd);
    }
    i++;
  }
  // ====================================================================
  fprintf(outfile,
  "/* end of list marker */\n"
  " %d };\n",MAX_TOK);
  fclose(outfile);
}
#endif
/*-------------------------------------------------------------------*/
#else
#include "iparith.inc"
#endif

/*=================== operations with 2 args. ============================*/

BOOLEAN iiExprArith2(leftv res, leftv a, int op, leftv b, BOOLEAN proccall)
{
#ifndef GENTABLE

  memset(res,0,sizeof(sleftv));

  if (!errorreported)
  {
#ifdef SIC
    if (sic)
    {
      emStoreExpr(a);
      emStoreExpr(b);
      emStoreOp(op,2);
      res->rtyp=SIC_MASK;
      return FALSE;
    }
#endif
#ifdef SIQ
    if (siq>0)
    {
      //Print("siq:%d\n",siq);
      command d=(command)Alloc0(sizeof(ip_command));
      memcpy(&d->arg1,a,sizeof(sleftv));
      memcpy(&d->arg2,b,sizeof(sleftv));
      d->argc=2;
      d->op=op;
      res->data=(char *)d;
      res->rtyp=COMMAND;
      return FALSE;
    }
#endif
    int at=a->Typ();
    int bt=b->Typ();
    int i=iiTabIndex(dArithTab2,JJTAB2LEN,op);

    iiOp=op;
    while (dArith2[i].cmd==op)
    {
      if ((at==dArith2[i].arg1)
      && (bt==dArith2[i].arg2))
      {
        res->rtyp=dArith2[i].res;
        if (dArith2[i].p(res,a,b))
        {
          break;// leave loop, goto error handling
        }
        a->CleanUp();
        b->CleanUp();
        //Print("op: %d,result typ:%d\n",op,res->rtyp);
        return FALSE;
      }
      i++;
    }
    // implicite type conversion ----------------------------------------------
    if (dArith2[i].cmd!=op)
    {
      int ai,bi;
      leftv an = (leftv)Alloc0(sizeof(sleftv));
      leftv bn = (leftv)Alloc0(sizeof(sleftv));
      BOOLEAN failed=FALSE;
      i=iiTabIndex(dArithTab2,JJTAB2LEN,op);
      //Print("op: %c, type: %s %s\n",op,Tok2Cmdname(at),Tok2Cmdname(bt));
      while (dArith2[i].cmd==op)
      {
        //Print("test %s %s\n",Tok2Cmdname(dArith2[i].arg1),Tok2Cmdname(dArith2[i].arg2));
        if ((ai=iiTestConvert(at,dArith2[i].arg1))!=0)
        {
          if ((bi=iiTestConvert(bt,dArith2[i].arg2))!=0)
          {
            res->rtyp=dArith2[i].res;
            failed= ((iiConvert(at,dArith2[i].arg1,ai,a,an))
            || (iiConvert(bt,dArith2[i].arg2,bi,b,bn))
            || (dArith2[i].p(res,an,bn)));
            // everything done, clean up temp. variables
            if (failed)
            {
              // leave loop, goto error handling
              break;
            }
            else
            {
              // everything ok, clean up and return
              an->CleanUp();
              bn->CleanUp();
              Free((ADDRESS)an,sizeof(sleftv));
              Free((ADDRESS)bn,sizeof(sleftv));
              a->CleanUp();
              b->CleanUp();
              return FALSE;
            }
          }
        }
        i++;
      }
      an->CleanUp();
      bn->CleanUp();
      Free((ADDRESS)an,sizeof(sleftv));
      Free((ADDRESS)bn,sizeof(sleftv));
    }
    // error handling ---------------------------------------------------
    const char *s=NULL;
    if (!errorreported)
    {
      if ((at==0) && (a->Name()!=sNoName))
      {
        s=a->Name();
      }
      else if ((bt==0) && (b->Name()!=sNoName))
      {
        s=b->Name();
      }
      if (s!=NULL)
        Werror("`%s` is not defined",s);
      else
      {
        i=iiTabIndex(dArithTab2,JJTAB2LEN,op);
        s = iiTwoOps(op);
        if (proccall)
        {
          Werror("%s(`%s`,`%s`) is not supported"
                ,s,Tok2Cmdname(at),Tok2Cmdname(bt));
        }
        else
        {
          Werror("`%s` %s `%s` is not supported"
                ,Tok2Cmdname(at),s,Tok2Cmdname(bt));
        }
        if (BVERBOSE(V_SHOW_USE))
        {
          while (dArith2[i].cmd==op)
          {
            if(((at==dArith2[i].arg1)||(bt==dArith2[i].arg2))
            && (dArith2[i].res!=0)
            && (dArith2[i].p!=jjWRONG2))
            {
              if (proccall)
                Werror("expected %s(`%s`,`%s`)"
                  ,s,Tok2Cmdname(dArith2[i].arg1),Tok2Cmdname(dArith2[i].arg2));
              else
                Werror("expected `%s` %s `%s`"
                  ,Tok2Cmdname(dArith2[i].arg1),s,Tok2Cmdname(dArith2[i].arg2));
            }
            i++;
          }
        }
      }
    }
    res->rtyp = UNKNOWN;
  }
  a->CleanUp();
  b->CleanUp();
#endif
  return TRUE;
}

/*==================== operations with 1 arg. ===============================*/

BOOLEAN iiExprArith1(leftv res, leftv a, int op)
{
#ifndef GENTABLE
  memset(res,0,sizeof(sleftv));

  if (!errorreported)
  {
#ifdef SIC
    if (sic && (a->rtyp==SIC_MASK))
    {
      emStoreOp(op,1);
      res->rtyp=SIC_MASK;
      return FALSE;
    }
#endif
#ifdef SIQ
    if (siq>0)
    {
      //Print("siq:%d\n",siq);
      command d=(command)Alloc0(sizeof(ip_command));
      memcpy(&d->arg1,a,sizeof(sleftv));
      d->op=op;
      d->argc=1;
      res->data=(char *)d;
      res->rtyp=COMMAND;
      return FALSE;
    }
#endif
    int at=a->Typ();
    BOOLEAN failed=FALSE;

    iiOp=op;
    int i=iiTabIndex(dArithTab1,JJTAB1LEN,op);
    while (dArith1[i].cmd==op)
    {
      if (at==dArith1[i].arg)
      {
        res->rtyp=dArith1[i].res;
#ifdef SIC
        if (sic)
        {
          emStoreExpr(a);
          emStoreOp(op,1);
          res->rtyp=ABS(res->rtyp)|SIC_MASK;
          a->CleanUp();
          return FALSE;
        }
#endif
        if (dArith1[i].res<0)
        {
          res->rtyp=-res->rtyp;
          res->data=(char *)((Proc1)dArith1[i].p)((char *)a->Data());
        }
        else if (dArith1[i].p(res,a))
        {
          break;// leave loop, goto error handling
        }
        if (a->Next()!=NULL)
        {
          res->next=(leftv)Alloc(sizeof(sleftv));
          failed=iiExprArith1(res->next,a->next,op);
        }
        a->CleanUp();
        return failed;
      }
      i++;
    }
    // implicite type conversion --------------------------------------------
    if (dArith1[i].cmd!=op)
    {
      int ai;
      leftv an = (leftv)Alloc0(sizeof(sleftv));
      i=iiTabIndex(dArithTab1,JJTAB1LEN,op);
      //Print("fuer %c , typ: %s\n",op,Tok2Cmdname(at));
      while (dArith1[i].cmd==op)
      {
        //Print("test %s\n",Tok2Cmdname(dArith1[i].arg));
        if ((ai=iiTestConvert(at,dArith1[i].arg))!=0)
        {
          res->rtyp=dArith1[i].res;
#ifdef SIC
          if (sic)
          {
            emStoreExpr(a);
            emStoreOp(op,1);
            res->rtyp=ABS(res->rtyp)|SIC_MASK;
            a->CleanUp();
            return FALSE;
          }
#endif
          if (dArith1[i].res<0)
          {
            res->rtyp=-res->rtyp;
            failed= iiConvert(at,dArith1[i].arg,ai,a,an);
            if (!failed)
              res->data=(char *)((Proc1)dArith1[i].p)((char *)an->Data());
          }
          else
          {
            failed= ((iiConvert(at,dArith1[i].arg,ai,a,an))
            || (dArith1[i].p(res,an)));
          }
          // everything done, clean up temp. variables
          if (failed)
          {
            // leave loop, goto error handling
            break;
          }
          else
          {
            // everything ok, clean up and return
            an->CleanUp();
            Free((ADDRESS)an,sizeof(sleftv));
            a->CleanUp();
            return FALSE;
          }
        }
        i++;
      }
      an->CleanUp();
      Free((ADDRESS)an,sizeof(sleftv));
    }
    // error handling
    if (!errorreported)
    {
      if ((at==0) && (a->Name()!=sNoName))
      {
        Werror("`%s` is not defined",a->Name());
      }
      else
      {
        i=iiTabIndex(dArithTab1,JJTAB1LEN,op);
        char *s = iiTwoOps(op);
        Werror("%s(`%s`) is not supported"
                ,s,Tok2Cmdname(at));
        if (BVERBOSE(V_SHOW_USE))
        {
          while (dArith1[i].cmd==op)
          {
            if ((dArith1[i].res!=0)
            && (dArith1[i].p!=jjWRONG))
              Werror("expected %s(`%s`)"
                ,s,Tok2Cmdname(dArith1[i].arg));
            i++;
          }
        }
      }
    }
    res->rtyp = UNKNOWN;
  }
  a->CleanUp();
#endif
  return TRUE;
}

/*=================== operations with 3 args. ============================*/

BOOLEAN iiExprArith3(leftv res, int op, leftv a, leftv b, leftv c)
{
#ifndef GENTABLE
  memset(res,0,sizeof(sleftv));

  if (!errorreported)
  {
#ifdef SIC
    if (sic)
    {
      emStoreExpr(a);
      emStoreExpr(b);
      emStoreExpr(c);
      emStoreOp(op,3);
      res->rtyp=SIC_MASK;
      return FALSE;
    }
#endif
#ifdef SIQ
    if (siq>0)
    {
      //Print("siq:%d\n",siq);
      command d=(command)Alloc0(sizeof(ip_command));
      memcpy(&d->arg1,a,sizeof(sleftv));
      memcpy(&d->arg2,b,sizeof(sleftv));
      memcpy(&d->arg3,c,sizeof(sleftv));
      d->op=op;
      d->argc=3;
      res->data=(char *)d;
      res->rtyp=COMMAND;
      return FALSE;
    }
#endif
    int at=a->Typ();
    int bt=b->Typ();
    int ct=c->Typ();

    iiOp=op;
    int i=0;
    while ((dArith3[i].cmd!=op)&&(dArith3[i].cmd!=0)) i++;
    while (dArith3[i].cmd==op)
    {
      if ((at==dArith3[i].arg1)
      && (bt==dArith3[i].arg2)
      && (ct==dArith3[i].arg3))
      {
        res->rtyp=dArith3[i].res;
        if (dArith3[i].p(res,a,b,c))
        {
          break;// leave loop, goto error handling
        }
        a->CleanUp();
        b->CleanUp();
        c->CleanUp();
        return FALSE;
      }
      i++;
    }
    // implicite type conversion ----------------------------------------------
    if (dArith3[i].cmd!=op)
    {
      int ai,bi,ci;
      leftv an = (leftv)Alloc0(sizeof(sleftv));
      leftv bn = (leftv)Alloc0(sizeof(sleftv));
      leftv cn = (leftv)Alloc0(sizeof(sleftv));
      BOOLEAN failed=FALSE;
      i=0;
      while ((dArith3[i].cmd!=op)&&(dArith3[i].cmd!=0)) i++;
      while (dArith3[i].cmd==op)
      {
        if ((ai=iiTestConvert(at,dArith3[i].arg1))!=0)
        {
          if ((bi=iiTestConvert(bt,dArith3[i].arg2))!=0)
          {
            if ((ci=iiTestConvert(ct,dArith3[i].arg3))!=0)
            {
              res->rtyp=dArith3[i].res;
              failed= ((iiConvert(at,dArith3[i].arg1,ai,a,an))
                || (iiConvert(bt,dArith3[i].arg2,bi,b,bn))
                || (iiConvert(ct,dArith3[i].arg3,ci,c,cn))
                || (dArith3[i].p(res,an,bn,cn)));
              // everything done, clean up temp. variables
              if (failed)
              {
                // leave loop, goto error handling
                break;
              }
              else
              {
                // everything ok, clean up and return
                an->CleanUp();
                bn->CleanUp();
                cn->CleanUp();
                Free((ADDRESS)an,sizeof(sleftv));
                Free((ADDRESS)bn,sizeof(sleftv));
                Free((ADDRESS)cn,sizeof(sleftv));
                a->CleanUp();
                b->CleanUp();
                c->CleanUp();
        //Print("op: %d,result typ:%d\n",op,res->rtyp);
                return FALSE;
              }
            }
          }
        }
        i++;
      }
      an->CleanUp();
      bn->CleanUp();
      cn->CleanUp();
      Free((ADDRESS)an,sizeof(sleftv));
      Free((ADDRESS)bn,sizeof(sleftv));
      Free((ADDRESS)cn,sizeof(sleftv));
    }
    // error handling ---------------------------------------------------
    if (!errorreported)
    {
      const char *s=NULL;
      if ((at==0) && (a->Name()!=sNoName))
      {
        s=a->Name();
      }
      else if ((bt==0) && (b->Name()!=sNoName))
      {
        s=b->Name();
      }
      else if ((ct==0) && (c->Name()!=sNoName))
      {
        s=c->Name();
      }
      if (s!=NULL)
        Werror("`%s` is not defined",s);
      else
      {
        i=0;
        while ((dArith3[i].cmd!=op)&&(dArith3[i].cmd!=0)) i++;
        char *s = iiTwoOps(op);
        Werror("%s(`%s`,`%s`,`%s`) is not supported"
                ,s,Tok2Cmdname(at),Tok2Cmdname(bt),Tok2Cmdname(ct));
        if (BVERBOSE(V_SHOW_USE))
        {
          while (dArith3[i].cmd==op)
          {
            if(((at==dArith3[i].arg1)
            ||(bt==dArith3[i].arg2)
            ||(ct==dArith3[i].arg3))
            && (dArith3[i].res!=0))
            {
              Werror("expected %s(`%s`,`%s`,`%s`)"
                  ,s,Tok2Cmdname(dArith3[i].arg1)
                  ,Tok2Cmdname(dArith3[i].arg2)
                  ,Tok2Cmdname(dArith3[i].arg3));
            }
            i++;
          }
        }
      }
    }
    res->rtyp = UNKNOWN;
  }
  a->CleanUp();
  b->CleanUp();
  c->CleanUp();
        //Print("op: %d,result typ:%d\n",op,res->rtyp);
#endif
  return TRUE;
}
/*==================== operations with many arg. ===============================*/

BOOLEAN iiExprArithM(leftv res, leftv a, int op)
{
#ifndef GENTABLE
  memset(res,0,sizeof(sleftv));

  if (!errorreported)
  {
#ifdef SIC
    if (sic)
    {
      emStoreExpr(a);
      emStoreOp(op,a->listLength());
      res->rtyp=SIC_MASK;
      return FALSE;
    }
#endif
#ifdef SIQ
    if (siq>0)
    {
      //Print("siq:%d\n",siq);
      command d=(command)Alloc0(sizeof(ip_command));
      d->op=op;
      res->data=(char *)d;
      if (a!=NULL)
      {
        d->argc=a->listLength();
        // else : d->argc=0;
        memcpy(&d->arg1,a,sizeof(sleftv));
        switch(d->argc)
        {
          case 3:
            memcpy(&d->arg3,a->next->next,sizeof(sleftv));
            a->next->next->rtyp=0;
            a->next->next->data=NULL;
            a->next->next->name=NULL;
            a->next->next->attribute=NULL;
          case 2:
            memcpy(&d->arg2,a->next,sizeof(sleftv));
            a->next->rtyp=0;
            a->next->name=NULL;
            a->next->data=NULL;
            a->next->attribute=NULL;
            d->arg2.next=NULL;
          case 1:
            d->arg1.next=NULL;
        }
        if (d->argc>3) a->next=NULL;
        a->rtyp=0;
        a->data=NULL;
        a->name=NULL;
        a->CleanUp();
      }
      res->rtyp=COMMAND;
      return FALSE;
    }
#endif
    BOOLEAN failed=FALSE;
    int args=0;
    if (a!=NULL) args=a->listLength();

    iiOp=op;
    int i=0;
    while ((dArithM[i].cmd!=op)&&(dArithM[i].cmd!=0)) i++;
    int ii=i;
    while (dArithM[i].cmd==op)
    {
      if ((args==dArithM[i].number_of_args)
      || (dArithM[i].number_of_args==-1)
      || ((dArithM[i].number_of_args==-2)&&(args>0)))
      {
        res->rtyp=dArithM[i].res;
        if (dArithM[i].p(res,a))
        {
          break;// leave loop, goto error handling
        }
        if (a!=NULL) a->CleanUp();
        //Print("op: %d,result typ:%d\n",op,res->rtyp);
        return failed;
      }
      i++;
    }
    // error handling
    if (!errorreported)
    {
      if ((a->rtyp==0) && (a->Name()!=sNoName))
      {
        Werror("`%s` is not defined",a->Name());
      }
      else
      {
        char *s = iiTwoOps(op);
        Werror("%s(...) is not supported",s);
      }
    }
    res->rtyp = UNKNOWN;
  }
  if (a!=NULL) a->CleanUp();
        //Print("op: %d,result typ:%d\n",op,res->rtyp);
#endif
  return TRUE;
}

/*=================== general utilities ============================*/
int IsCmd(char *n, int & tok)
{
  int an=1;
  int i,v;
#ifndef GENTABLE
  int en=LAST_IDENTIFIER;

  loop
  {
    if(an>=en-1)
    {
      if (strcmp(n, cmds[an].name) == 0)
      {
        i=an;
        break;
      }
      else if (strcmp(n, cmds[en].name) == 0)
      {
        i=en;
        break;
      }
      else
      {
        return 0;
      }
    }
    i=(an+en)/2;
    v=strcmp(n,cmds[i].name);
    if(v<0)
    {
      en=i;
    }
    else if(v>0)
    {
      an=i;
    }
    else /*v==0*/
    {
      break;
    }
  }
  lastreserved=cmds[i].name;
  tok=cmds[i].tokval;
  #ifndef SIC
  if (currRingHdl==NULL)
  {
    #ifdef SIQ
    if (siq<=0)
    {
    #endif
      if ((tok>=BEGIN_RING) && (tok<=END_RING))
      {
         Werror("no ring active");
         return 0;
      }
    #ifdef SIQ
    }
    #endif
  }
  #endif
  if (!expected_parms)
  {
    switch (tok)
    {
      #ifdef HAVE_DLD
      case BINARY_CMD:
      #endif
      case IDEAL_CMD:
      case INT_CMD:
      case INTVEC_CMD:
      case MAP_CMD:
      case MATRIX_CMD:
      case MODUL_CMD:
      case POLY_CMD:
      case PROC_CMD:
      case RING_CMD:
      case STRING_CMD:
        cmdtok = tok;
        break;
    }
  }
#endif
  return cmds[i].toktype;
}
static int iiTabIndex(const jjValCmdTab dArithTab, const int len, const int op)
{
#ifndef GENTABLE
  int a=0;
  int e=len;
  int p=len/2;
  while ( a!=e)
  {
     if (op==dArithTab[p].cmd) return dArithTab[p].start;
     else if (op<dArithTab[p].cmd)
     { e=p; p=a+(e-a)/2;}
     else
     { a=p; p=p+(e-p)/2; }
  }
#endif
  return -1;
}

