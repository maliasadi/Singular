// emacs edit mode for this file is -*- C++ -*-
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
// $Id: clapsing.cc,v 1.27 2008-03-17 18:33:55 Singular Exp $
/*
* ABSTRACT: interface between Singular and factory
*/

//#define FACTORIZE2_DEBUG
#include "mod2.h"
#include "omalloc.h"
#ifdef HAVE_FACTORY
#define SI_DONT_HAVE_GLOBAL_VARS
#include "structs.h"
#include "clapsing.h"
#include "numbers.h"
#include "ring.h"
#include <factory.h>
#include "clapconv.h"
#ifdef HAVE_LIBFAC_P
#include <factor.h>
//CanonicalForm algcd(const CanonicalForm & F, const CanonicalForm & g, const CFList & as, const Varlist & order);
CanonicalForm alg_gcd(const CanonicalForm &, const CanonicalForm &, const CFList &);
#endif
#include "ring.h"

//
// FACTORY_GCD_TEST: use new gcd instead of old one.  Does not work
//   without new gcd-implementation which is not publicly available.
//
// FACTORY_GCD_STAT: print statistics on polynomials.  Works only
//   with the file `gcd_stat.cc' and `gcd_stat.h which may be found
//   in the repository, module `factory-devel'.
//   Overall statistics may printed using `system("gcdstat");'.
//
// FACTORY_GCD_TIMING: accumulate time used for gcd calculations.
//   Time may be printed (and reset) with `system("gcdtime");'.
//   For this define, `timing.h' from the factory source directory
//   has to be copied to the Singular source directory.
//   Note: for better readability, the macros `TIMING_START()' and
//   `TIMING_END()' are used in any case.  However, they expand to
//   nothing if `FACTORY_GCD_TIMING' is off.
//
// FACTORY_GCD_DEBOUT: print polynomials involved in gcd calculations.
//   The polynomials are printed by means of the macros
//   `FACTORY_*OUT_POLY' which are defined to be empty if
//   `FACTORY_GCD_DEBOUT' is off.
//
// FACTORY_GCD_DEBOUT_PATTERN: print degree patterns of polynomials
//   involved in gcd calculations.
//   The patterns are printed by means of the macros
//   `FACTORY_*OUT_PAT' which are defined to be empty if
//   `FACTORY_GCD_DEBOUT_PATTERN' is off.
//
//   A degree pattern looks like this:
//
//   totDeg  size    deg(v1) deg(v2) ...
//
//   where "totDeg" means total degree, "size" the number of terms,
//   and "deg(vi)" is the degree with respect to variable i.
//   In univariate case, the "deg(vi)" are missing.  For this feature
//   you need the files `gcd_stat.cc' and `gcd_stat.h'.
//
//
// And here is what the functions print if `FACTORY_GCD_DEBOUT' (1),
// `FACTORY_GCD_STAT' (2), or `FACTORY_GCD_DEBOUT_PATTERN' (3) is on:
//
// sinclap_divide_content:
// (1) G = <firstCoeff>
// (3) G#= <firstCoeff, pattern>
// (1) h = <nextCoeff>
// (3) h#= <nextCoeff, pattern>
// (2) gcnt: <statistics on gcd as explained above>
// (1) g = <intermediateResult>
// (3) g#= <intermediateResult, pattern>
// (1) h = <nextCoeff>
// (3) h#= <nextCoeff, pattern>
// (2) gcnt: <statistics on gcd as explained above>
//  ...
// (1) h = <lastCoeff>
// (3) h#= <lastCoeff, pattern>
// (1) g = <finalResult>
// (3) g#= <finalResult, pattern>
// (2) gcnt: <statistics on gcd as explained above>
// (2) cont: <statistics on content as explained above>
//
// singclap_alglcm:
// (1) f = <inputPolyF>
// (3) f#= <inputPolyF, pattern>
// (1) g = <inputPolyG>
// (3) g#= <inputPolyG, pattern>
// (1) d = <its gcd>
// (3) d#= <its gcd, pattern>
// (2) alcm: <statistics as explained above>
//
// singclap_algdividecontent:
// (1) f = <inputPolyF>
// (3) f#= <inputPolyF, pattern>
// (1) g = <inputPolyG>
// (3) g#= <inputPolyG, pattern>
// (1) d = <its gcd>
// (3) d#= <its gcd, pattern>
// (2) acnt: <statistics as explained above>
//

#ifdef FACTORY_GCD_STAT
#include "gcd_stat.h"
#define FACTORY_GCDSTAT( tag, f, g, d ) \
  printGcdStat( tag, f, g, d )
#define FACTORY_CONTSTAT( tag, f ) \
  printContStat( tag, f )
#else
#define FACTORY_GCDSTAT( tag, f, g, d )
#define FACTORY_CONTSTAT( tag, f )
#endif

#ifdef FACTORY_GCD_TIMING
#define TIMING
#include "timing.h"
TIMING_DEFINE_PRINT( contentTimer );
TIMING_DEFINE_PRINT( algContentTimer );
TIMING_DEFINE_PRINT( algLcmTimer );
#else
#define TIMING_START( timer )
#define TIMING_END( timer )
#endif

#ifdef FACTORY_GCD_DEBOUT
#include "longalg.h"
#include "febase.h"
// napoly f
#define FACTORY_ALGOUT_POLY( tag, f ) \
  StringSetS( tag ); \
  napWrite( f ); \
  pRINtS(StringAppendS("\n"));
// CanonicalForm f, represents transcendent extension
#define FACTORY_CFTROUT_POLY( tag, f ) \
  { \
    napoly F=convFactoryPSingTr( f ); \
    StringSetS( tag ); \
    napWrite( F ); \
    PrintS(StringAppendS("\n")); \
    napDelete(&F); \
  }
// CanonicalForm f, represents algebraic extension
#define FACTORY_CFAOUT_POLY( tag, f ) \
  { \
    napoly F=convFactoryASingA( f ); \
    StringSetS( tag ); \
    napWrite( F ); \
    PrintS(StringAppendS("\n")); \
    napDelete(&F); \
  }
#else /* ! FACTORY_GCD_DEBOUT */
#define FACTORY_ALGOUT_POLY( tag, f )
#define FACTORY_CFTROUT_POLY( tag, f )
#define FACTORY_CFAOUT_POLY( tag, f )
#endif /* ! FACTORY_GCD_DEBOUT */

#ifdef FACTORY_GCD_DEBOUT_PATTERN
// napoly f
#define FACTORY_ALGOUT_PAT( tag, f ) \
  if (currRing->minpoly!=NULL) \
  { \
    CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z); \
    Variable a=rootOf(mipo); \
    printPolyPattern( tag, convSingAFactoryA( f,a ), rPar( currRing ) ); \
  } \
  else \
  { \
    printPolyPattern( tag, convSingTrFactoryP( f ), rPar( currRing ) ); \
  }
// CanonicalForm f, represents transcendent extension
#define FACTORY_CFTROUT_PAT( tag, f ) printPolyPattern( tag, f, rPar( currRing ) )
// CanonicalForm f, represents algebraic extension
#define FACTORY_CFAOUT_PAT( tag, f ) printPolyPattern( tag, f, rPar( currRing ) )
#else /* ! FACTORY_GCD_DEBOUT_PATTERN */
#define FACTORY_ALGOUT_PAT( tag, f )
#define FACTORY_CFTROUT_PAT( tag, f )
#define FACTORY_CFAOUT_PAT( tag, f )
#endif /* ! FACTORY_GCD_DEBOUT_PATTERN */

// these macors combine both print macros
#define FACTORY_ALGOUT( tag, f ) \
  FACTORY_ALGOUT_POLY( tag " = ", f ); \
  FACTORY_ALGOUT_PAT( tag "#= ", f )
#define FACTORY_CFTROUT( tag, f ) \
  FACTORY_CFTROUT_POLY( tag " = ", f ); \
  FACTORY_CFTROUT_PAT( tag "#= ", f )
#define FACTORY_CFAOUT( tag, f ) \
  FACTORY_CFAOUT_POLY( tag " = ", f ); \
  FACTORY_CFAOUT_PAT( tag "#= ", f )





poly singclap_gcd ( poly f, poly g )
{
  poly res=NULL;

  if (f!=NULL) pCleardenom(f);
  if (g!=NULL) pCleardenom(g);
  else         return pCopy(f); // g==0 => gcd=f (but do a pCleardenom)
  if (f==NULL) return pCopy(g); // f==0 => gcd=g (but do a pCleardenom)

  if (pIsConstantPoly(f) || pIsConstantPoly(g)) return pOne();

  // for now there is only the possibility to handle polynomials over
  // Q and Fp ...
  Off(SW_RATIONAL);
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    CanonicalForm newGCD(const CanonicalForm & A, const CanonicalForm & B);
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPFactoryP( f ) ), G( convSingPFactoryP( g ) );
    //if (nGetChar() > 1 )
    //{
    //  res=convFactoryPSingP( newGCD( F,G ));
    //  if (!nGreaterZero(pGetCoeff(res))) res=pNeg(res);
    //}
    //else
      res=convFactoryPSingP( gcd( F, G ) );
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    if (currRing->minpoly!=NULL)
    {
    #ifdef HAVE_LIBFAC_P
      if ( nGetChar()==1 ) /* Q(a) */
      {
      //  WerrorS( feNotImplemented );
        CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
        //Varlist ord;
        //ord.append(mipo.mvar());
        CFList as(mipo);
        Variable a=rootOf(mipo);
        //CanonicalForm F( convSingAPFactoryAP( f,a ) ), G( convSingAPFactoryAP( g,a ) );
        CanonicalForm F( convSingTrPFactoryP(f) ), G( convSingTrPFactoryP(g) );
        //res= convFactoryAPSingAP( algcd( F, G, as, ord) );
        //res= convFactoryAPSingAP( alg_gcd( F, G, as) );
        res= convFactoryAPSingAP( alg_gcd( F, G, as) );
      }
      else
      #endif
      {
        CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
        Variable a=rootOf(mipo);
        CanonicalForm F( convSingAPFactoryAP( f,a ) ), G( convSingAPFactoryAP( g,a ) );
        res= convFactoryAPSingAP( gcd( F, G ) );
      }
    }
    else
    {
      CanonicalForm F( convSingTrPFactoryP( f ) ), G( convSingTrPFactoryP( g ) );
      res= convFactoryPSingTrP( gcd( F, G ) );
    }
  }
  #if 0
  else if (( nGetChar()>1 )&&(currRing->parameter!=NULL)) /* GF(q) */
  {
    int p=rChar(currRing);
    int n=2;
    int t=p*p;
    while (t!=nChar) { t*=p;n++; }
    setCharacteristic(p,n,'a');
    CanonicalForm F( convSingGFFactoryGF( f ) ), G( convSingGFFactoryGF( g ) );
    res= convFactoryGFSingGF( gcd( F, G ) );
  }
  #endif
  else
    WerrorS( feNotImplemented );

  Off(SW_RATIONAL);
  pDelete(&f);
  pDelete(&g);
  pTest(res);
  return res;
}

/*2 find the maximal exponent of var(i) in poly p*/
int pGetExp_Var(poly p, int i)
{
  int m=0;
  int mm;
  while (p!=NULL)
  {
    mm=pGetExp(p,i);
    if (mm>m) m=mm;
    pIter(p);
  }
  return m;
}

poly singclap_resultant ( poly f, poly g , poly x)
{
  int i=pIsPurePower(x);
  if (i==0)
  {
    WerrorS("3rd argument must be a ring variable");
    return NULL;
  }
  if ((f==NULL) || (g==NULL))
    return NULL;
  // for now there is only the possibility to handle polynomials over
  // Q and Fp ...
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    Variable X(i);
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPFactoryP( f ) ), G( convSingPFactoryP( g ) );
    poly res=convFactoryPSingP( resultant( F, G, X ) );
    Off(SW_RATIONAL);
    return res;
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    poly res;
    Variable X(i+rPar(currRing));
    if (currRing->minpoly!=NULL)
    {
      //Variable X(i);
      CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPFactoryAP( f,a ) ), G( convSingAPFactoryAP( g,a ) );
      res= convFactoryAPSingAP( resultant( F, G, X ) );
    }
    else
    {
      //Variable X(i+rPar(currRing));
      number nf,ng;
      pCleardenom_n(f,nf);pCleardenom_n(g,ng);
      int ef,eg;
      ef=pGetExp_Var(f,i);
      eg=pGetExp_Var(g,i);
      CanonicalForm F( convSingTrPFactoryP( f ) ), G( convSingTrPFactoryP( g ) );
      res= convFactoryPSingTrP( resultant( F, G, X ) );
      if ((nf!=NULL)&&(!nIsOne(nf))&&(!nIsZero(nf)))
      {
        number n=nInvers(nf);
        while(eg>0)
        {
          res=pMult_nn(res,n);
          eg--;
        }
        nDelete(&n);
      }
      nDelete(&nf);
      if ((ng!=NULL)&&(!nIsOne(ng))&&(!nIsZero(ng)))
      {
        number n=nInvers(ng);
        while(ef>0)
        {
          res=pMult_nn(res,n);
          ef--;
        }
        nDelete(&n);
      }
      nDelete(&ng);
    }
    Off(SW_RATIONAL);
    return res;
  }
  else
    WerrorS( feNotImplemented );
  return NULL;
}
//poly singclap_resultant ( poly f, poly g , poly x)
//{
//  int i=pVar(x);
//  if (i==0)
//  {
//    WerrorS("ringvar expected");
//    return NULL;
//  }
//  ideal I=idInit(1,1);
//
//  // get the coeffs von f wrt. x:
//  I->m[0]=pCopy(f);
//  matrix ffi=mpCoeffs(I,i);
//  ffi->rank=1;
//  ffi->ncols=ffi->nrows;
//  ffi->nrows=1;
//  ideal fi=(ideal)ffi;
//
//  // get the coeffs von g wrt. x:
//  I->m[0]=pCopy(g);
//  matrix ggi=mpCoeffs(I,i);
//  ggi->rank=1;
//  ggi->ncols=ggi->nrows;
//  ggi->nrows=1;
//  ideal gi=(ideal)ggi;
//
//  // contruct the matrix:
//  int fn=IDELEMS(fi); //= deg(f,x)+1
//  int gn=IDELEMS(gi); //= deg(g,x)+1
//  matrix m=mpNew(fn+gn-2,fn+gn-2);
//  if(m==NULL)
//  {
//    return NULL;
//  }
//
//  // enter the coeffs into m:
//  int j;
//  for(i=0;i<gn-1;i++)
//  {
//    for(j=0;j<fn;j++)
//    {
//      MATELEM(m,i+1,fn-j+i)=pCopy(fi->m[j]);
//    }
//  }
//  for(i=0;i<fn-1;i++)
//  {
//    for(j=0;j<gn;j++)
//    {
//      MATELEM(m,gn+i,gn-j+i)=pCopy(gi->m[j]);
//    }
//  }
//
//  poly r=mpDet(m);
//
//  idDelete(&fi);
//  idDelete(&gi);
//  idDelete((ideal *)&m);
//  return r;
//}

BOOLEAN singclap_extgcd ( poly f, poly g, poly &res, poly &pa, poly &pb )
{
  // for now there is only the possibility to handle univariate
  // polynomials over
  // Q and Fp ...
  res=NULL;pa=NULL;pb=NULL;
  On(SW_SYMMETRIC_FF);
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPFactoryP( f ) ), G( convSingPFactoryP( g ) );
    CanonicalForm FpG=F+G;
    if (!(FpG.isUnivariate()|| FpG.inCoeffDomain()))
    //if (!F.isUnivariate() || !G.isUnivariate() || F.mvar()!=G.mvar())
    {
      Off(SW_RATIONAL);
      WerrorS("not univariate");
      return TRUE;
    }
    CanonicalForm Fa,Gb;
    On(SW_RATIONAL);
    res=convFactoryPSingP( extgcd( F, G, Fa, Gb ) );
    pa=convFactoryPSingP(Fa);
    pb=convFactoryPSingP(Gb);
    Off(SW_RATIONAL);
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    CanonicalForm Fa,Gb;
    if (currRing->minpoly!=NULL)
    {
      CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPFactoryAP( f,a ) ), G( convSingAPFactoryAP( g,a ) );
      CanonicalForm FpG=F+G;
      if (!(FpG.isUnivariate()|| FpG.inCoeffDomain()))
      //if (!F.isUnivariate() || !G.isUnivariate() || F.mvar()!=G.mvar())
      {
        WerrorS("not univariate");
        return TRUE;
      }
      res= convFactoryAPSingAP( extgcd( F, G, Fa, Gb ) );
      pa=convFactoryAPSingAP(Fa);
      pb=convFactoryAPSingAP(Gb);
    }
    else
    {
      CanonicalForm F( convSingTrPFactoryP( f ) ), G( convSingTrPFactoryP( g ) );
      CanonicalForm FpG=F+G;
      if (!(FpG.isUnivariate()|| FpG.inCoeffDomain()))
      //if (!F.isUnivariate() || !G.isUnivariate() || F.mvar()!=G.mvar())
      {
        Off(SW_RATIONAL);
        WerrorS("not univariate");
        return TRUE;
      }
      res= convFactoryPSingTrP( extgcd( F, G, Fa, Gb ) );
      pa=convFactoryPSingTrP(Fa);
      pb=convFactoryPSingTrP(Gb);
    }
    Off(SW_RATIONAL);
  }
  else
  {
    WerrorS( feNotImplemented );
    return TRUE;
  }
  return FALSE;
}

poly singclap_pdivide ( poly f, poly g )
{
  // for now there is only the possibility to handle polynomials over
  // Q and Fp ...
  poly res=NULL;
  On(SW_RATIONAL);
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPFactoryP( f ) ), G( convSingPFactoryP( g ) );
    res = convFactoryPSingP( F / G );
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    if (currRing->minpoly!=NULL)
    {
      CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPFactoryAP( f,a ) ), G( convSingAPFactoryAP( g,a ) );
      res= convFactoryAPSingAP(  F / G  );
    }
    else
    {
      CanonicalForm F( convSingTrPFactoryP( f ) ), G( convSingTrPFactoryP( g ) );
      res= convFactoryPSingTrP(  F / G  );
    }
  }
  else
    WerrorS( feNotImplemented );
  Off(SW_RATIONAL);
  return res;
}

void singclap_divide_content ( poly f )
{
  if ( f==NULL )
  {
    return;
  }
  else  if ( pNext( f ) == NULL )
  {
    pSetCoeff( f, nInit( 1 ) );
    return;
  }
  else
  {
    if ( nGetChar() == 1 )
      setCharacteristic( 0 );
    else  if ( nGetChar() == -1 )
      return; /* not implemented for R */
    else  if ( nGetChar() < 0 )
      setCharacteristic( -nGetChar() );
    else if (currRing->parameter==NULL) /* not GF(q) */
      setCharacteristic( nGetChar() );
    else
      return; /* not implemented*/

    CFList L;
    CanonicalForm g, h;
    poly p = pNext(f);

    // first attemp: find 2 smallest g:

    number g1=pGetCoeff(f);
    number g2=pGetCoeff(p); // p==pNext(f);
    pIter(p);
    int sz1=nSize(g1);
    int sz2=nSize(g2);
    if (sz1>sz2)
    {
      number gg=g1;
      g1=g2; g2=gg;
      int sz=sz1;
      sz1=sz2; sz2=sz;
    }
    while (p!=NULL)
    {
      int n_sz=nSize(pGetCoeff(p));
      if (n_sz<sz1)
      {
        sz2=sz1;
        g2=g1;
        g1=pGetCoeff(p);
        sz1=n_sz;
        if (sz1<=3) break;
      }
      else if(n_sz<sz2)
      {
        sz2=n_sz;
        g2=pGetCoeff(p);
        sz2=n_sz;
      }
      pIter(p);
    }
    FACTORY_ALGOUT( "G", ((lnumber)g1)->z );
    g = convSingTrFactoryP( ((lnumber)g1)->z );
    g = gcd( g, convSingTrFactoryP( ((lnumber)g2)->z ));

    // second run: gcd's

    p = f;
    TIMING_START( contentTimer );
    while ( (p != NULL) && (g != 1)  && ( g != 0))
    {
      FACTORY_ALGOUT( "h", (((lnumber)pGetCoeff(p))->z) );
      h = convSingTrFactoryP( ((lnumber)pGetCoeff(p))->z );
      pIter( p );
#ifdef FACTORY_GCD_STAT
      // save g
      CanonicalForm gOld = g;
#endif

#ifdef FACTORY_GCD_TEST
      g = CFPrimitiveGcdUtil::gcd( g, h );
#else
      g = gcd( g, h );
#endif

      FACTORY_GCDSTAT( "gcnt:", gOld, h, g );
      FACTORY_CFTROUT( "g", g );
      L.append( h );
    }
    TIMING_END( contentTimer );
    FACTORY_CONTSTAT( "cont:", g );
    if (( g == 1 ) || (g == 0))
    {
      // pTest(f);
      return;
    }
    else
    {
      CFListIterator i;
      for ( i = L, p = f; i.hasItem(); i++, p=pNext(p) )
      {
        lnumber c=(lnumber)pGetCoeff(p);
        napDelete(&c->z);
        c->z=convFactoryPSingTr( i.getItem() / g );
        //nTest((number)c);
        //#ifdef LDEBUG
        //number cn=(number)c;
        //StringSetS(""); nWrite(nt); StringAppend(" ==> ");
        //nWrite(cn);PrintS(StringAppend("\n"));
        //#endif
      }
    }
    // pTest(f);
  }
}

static int primepower(int c)
{
  int p=1;
  int cc=c;
  while(cc!= rInternalChar(currRing)) { cc*=c; p++; }
  return p;
}

BOOLEAN count_Factors(ideal I, intvec *v,int j, poly &f, poly fac)
{
  pTest(f);
  pTest(fac);
  int e=0;
  if (!pIsConstantPoly(fac))
  {
#ifdef FACTORIZE2_DEBUG
    printf("start count_Factors(%d), Fdeg=%d, factor deg=%d\n",j,pTotaldegree(f),pTotaldegree(fac));
    p_wrp(fac,currRing);PrintLn();
#endif
    On(SW_RATIONAL);
    CanonicalForm F, FAC,Q,R;
    Variable a;
    if (( nGetChar() == 0 || nGetChar() > 1 )
    && (currRing->parameter==NULL))
    {
      F=convSingPFactoryP( f );
      FAC=convSingPFactoryP( fac );
    }
    // and over Q(a) / Fp(a)
    else if (( nGetChar()==1 ) /* Q(a) */
    || (nGetChar() <-1))       /* Fp(a) */
    {
      if (currRing->minpoly!=NULL)
      {
        CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
        a=rootOf(mipo);
        F=convSingAPFactoryAP( f,a );
        FAC=convSingAPFactoryAP( fac,a );
      }
      else
      {
        F=convSingTrPFactoryP( f );
        FAC=convSingTrPFactoryP( fac );
      }
    }
    else
      WerrorS( feNotImplemented );

    poly q;
    loop
    {
      Q=F;
      Q/=FAC;
      R=Q;
      R*=FAC;
      R-=F;
      if (R.isZero())
      {
        if (( nGetChar() == 0 || nGetChar() > 1 )
        && (currRing->parameter==NULL))
        {
          q = convFactoryPSingP( Q );
        }
        else if (( nGetChar()==1 ) /* Q(a) */
        || (nGetChar() <-1))       /* Fp(a) */
        {
          if (currRing->minpoly!=NULL)
          {
            q= convFactoryAPSingAP(  Q  );
          }
          else
          {
            q= convFactoryPSingTrP(  Q  );
          }
        }
        e++; pDelete(&f); f=q; q=NULL; F=Q;
      }
      else
      {
        break;
      }
    }
    if (e==0)
    {
      Off(SW_RATIONAL);
      return FALSE;
    }
  }
  else e=1;
  I->m[j]=fac;
  if (v!=NULL) (*v)[j]=e;
  Off(SW_RATIONAL);
  return TRUE;
}

int singclap_factorize_retry;
#ifdef HAVE_LIBFAC_P
extern int libfac_interruptflag;
#endif

ideal singclap_factorize ( poly f, intvec ** v , int with_exps)
{
  pTest(f);
#ifdef FACTORIZE2_DEBUG
  printf("singclap_factorize, degree %d\n",pTotaldegree(f));
#endif
  // with_exps: 3,1 return only true factors, no exponents
  //            2 return true factors and exponents
  //            0 return coeff, factors and exponents
  BOOLEAN save_errorreported=errorreported;

  ideal res=NULL;

  // handle factorize(0) =========================================
  if (f==NULL)
  {
    res=idInit(1,1);
    if (with_exps!=1)
    {
      (*v)=new intvec(1);
      (**v)[0]=1;
    }
    return res;
  }
  // handle factorize(mon) =========================================
  if (pNext(f)==NULL)
  {
    int i=0;
    int n=0;
    int e;
    for(i=pVariables;i>0;i--) if(pGetExp(f,i)!=0) n++;
    if (with_exps==0) n++; // with coeff
    res=idInit(si_max(n,1),1);
    switch(with_exps)
    {
      case 0: // with coef & exp.
        res->m[0]=pOne();
        pSetCoeff(res->m[0],nCopy(pGetCoeff(f)));
        // no break
      case 2: // with exp.
        (*v)=new intvec(si_max(1,n));
        (**v)[0]=1;
        // no break
      case 1: ;
      #ifdef TEST
      default: ;
      #endif
    }
    if (n==0)
    {
      res->m[0]=pOne();
      // (**v)[0]=1; is already done
      return res;
    }
    for(i=pVariables;i>0;i--)
    {
      e=pGetExp(f,i);
      if(e!=0)
      {
        n--;
        poly p=pOne();
        pSetExp(p,i,1);
        pSetm(p);
        res->m[n]=p;
        if (with_exps!=1) (**v)[n]=e;
      }
    }
    return res;
  }
  //PrintS("S:");pWrite(f);PrintLn();
  // use factory/libfac in general ==============================
  Off(SW_RATIONAL);
  On(SW_SYMMETRIC_FF);
  #ifdef HAVE_NTL
  extern int prime_number;
  if(rField_is_Q()) prime_number=0;
  #endif
  CFFList L;
  number N=NULL;
  number NN=NULL;
  number old_lead_coeff=nCopy(pGetCoeff(f));

  if (!rField_is_Zp() && !rField_is_Zp_a()) /* Q, Q(a) */
  {
    //if (f!=NULL) // already tested at start of routine
    {
      number n0=nCopy(pGetCoeff(f));
      if (with_exps==0)
        N=nCopy(n0);
      pCleardenom(f);
      NN=nDiv(n0,pGetCoeff(f));
      nDelete(&n0);
      if (with_exps==0)
      {
        nDelete(&N);
        N=nCopy(NN);
      }
    }
  }
  else if (rField_is_Zp_a())
  {
    //if (f!=NULL) // already tested at start of routine
    if (singclap_factorize_retry==0)
    {
      number n0=nCopy(pGetCoeff(f));
      if (with_exps==0)
        N=nCopy(n0);
      pNorm(f);
      pCleardenom(f);
      NN=nDiv(n0,pGetCoeff(f));
      nDelete(&n0);
      if (with_exps==0)
      {
        nDelete(&N);
        N=nCopy(NN);
      }
    }
  }
  if (rField_is_Q() || rField_is_Zp())
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPFactoryP( f ) );
    if (nGetChar()==0) /* Q */
    {
      L = factorize( F );
    }
    else /* Fp */
    {
#ifdef HAVE_LIBFAC_P
      do
      {
        libfac_interruptflag=0;
        L = Factorize( F );
      }
      while ((libfac_interruptflag!=0) ||(L.isEmpty()));
#else
      goto notImpl;
#endif
    }
  }
  #if 0
  else if (rField_is_GF())
  {
    int c=rChar(currRing);
    setCharacteristic( c, primepower(c) );
    CanonicalForm F( convSingGFFactoryGF( f ) );
    if (F.isUnivariate())
    {
      L = factorize( F );
    }
    else
    {
      goto notImpl;
    }
  }
  #endif
  // and over Q(a) / Fp(a)
  else if (rField_is_Extension())
  {
    if (rField_is_Q_a()) setCharacteristic( 0 );
    else                 setCharacteristic( -nGetChar() );
    if (currRing->minpoly!=NULL)
    {
      CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPFactoryAP( f,a ) );
      L.insert(F);
      if (rField_is_Zp_a() && F.isUnivariate())
      {
        L = factorize( F, a );
      }
      else
      {
        CanonicalForm G( convSingTrPFactoryP( f ) );
#ifdef HAVE_LIBFAC_P
        //  over Q(a) / multivariate over Fp(a)
        do
        {
          libfac_interruptflag=0;
          L=Factorize2(G, mipo);
        }
        while ((libfac_interruptflag!=0) ||(L.isEmpty()));
        #ifdef FACTORIZE2_DEBUG
        printf("while okay\n");
        #endif
        libfac_interruptflag=0;
        //else
        //  L=Factorize(G, mipo);
#else
        WarnS("complete factorization only for univariate polynomials");
        if (rField_is_Q_a() ||(!F.isUnivariate())) /* Q(a) */
        {
          L = factorize( G );
        }
        else
        {
          L = factorize( G, a );
        }
#endif
      }
    }
    else
    {
      CanonicalForm F( convSingTrPFactoryP( f ) );
      if (rField_is_Q_a())
      {
        L = factorize( F );
      }
      else /* Fp(a) */
      {
#ifdef HAVE_LIBFAC_P
        L = Factorize( F );
#else
        goto notImpl;
#endif
      }
    }
  }
  else
  {
    goto notImpl;
  }
  {
    poly ff=pCopy(f); // a copy for the retry stuff
    // the first factor should be a constant
    if ( ! L.getFirst().factor().inCoeffDomain() )
      L.insert(CFFactor(1,1));
    // convert into ideal
    int n = L.length();
    if (n==0) n=1;
    CFFListIterator J=L;
    int j=0;
    if (with_exps!=1)
    {
      if ((with_exps==2)&&(n>1))
      {
        n--;
        J++;
      }
      *v = new intvec( n );
    }
    res = idInit( n ,1);
    for ( ; J.hasItem(); J++, j++ )
    {
      poly p;
      if (with_exps!=1) (**v)[j] = J.getItem().exp();
      if (rField_is_Zp() || rField_is_Q())           /* Q, Fp */
        //count_Factors(res,*v,f, j, convFactoryPSingP( J.getItem().factor() );
        res->m[j] = convFactoryPSingP( J.getItem().factor() );
      #if 0
      else if (rField_is_GF())
        res->m[j] = convFactoryGFSingGF( J.getItem().factor() );
      #endif
      else if (rField_is_Extension())     /* Q(a), Fp(a) */
      {
        intvec *w=NULL;
        if (v!=NULL) w=*v;
        if (currRing->minpoly==NULL)
          count_Factors(res,w,j,ff,convFactoryPSingTrP( J.getItem().factor() ));
        else
          count_Factors(res,w,j,ff,convFactoryAPSingAP( J.getItem().factor() ));
      }
    }
    if (rField_is_Extension() && (!pIsConstantPoly(ff)))
    {
      singclap_factorize_retry++;
      if (singclap_factorize_retry<3)
      {
        int jj;
        #ifdef FACTORIZE2_DEBUG
        printf("factorize_retry\n");
        #endif
        intvec *ww=NULL;
        idTest(res);
        ideal h=singclap_factorize ( ff, &ww , with_exps);
        idTest(h);
        int l=(*v)->length();
        (*v)->resize(l+ww->length());
        for(jj=0;jj<ww->length();jj++)
          (**v)[jj+l]=(*ww)[jj];
        delete ww;
        ideal hh=idInit(IDELEMS(res)+IDELEMS(h),1);
        for(jj=IDELEMS(res)-1;jj>=0;jj--)
        {
          hh->m[jj]=res->m[jj];
          res->m[jj]=NULL;
        }
        for(jj=IDELEMS(h)-1;jj>=0;jj--)
        {
          hh->m[jj+IDELEMS(res)]=h->m[jj];
          h->m[jj]=NULL;
        }
        idDelete(&res);
        idDelete(&h);
        res=hh;
        idTest(res);
        ff=NULL;
      }
      else
      {
        WarnS("problem with factorize");
      }
    }
    pDelete(&ff);
    if (N!=NULL)
    {
      pMult_nn(res->m[0],N);
      nDelete(&N);
      N=NULL;
    }
    // delete constants
    if (res!=NULL)
    {
      int i=IDELEMS(res)-1;
      int j=0;
      for(;i>=0;i--)
      {
        if ((res->m[i]!=NULL)
        && (pNext(res->m[i])==NULL)
        && (pIsConstant(res->m[i])))
        {
          if (with_exps!=0)
          {
            pDelete(&(res->m[i]));
            if ((v!=NULL) && ((*v)!=NULL))
              (**v)[i]=0;
            j++;
          }
          else if (i!=0)
          {
            while ((v!=NULL) && ((*v)!=NULL) && ((**v)[i]>1))
            {
              res->m[0]=pMult(res->m[0],pCopy(res->m[i]));
              (**v)[i]--;
            }
            res->m[0]=pMult(res->m[0],res->m[i]);
            res->m[i]=NULL;
            if ((v!=NULL) && ((*v)!=NULL))
              (**v)[i]=0;
            j++;
          }
        }
      }
      if (j>0)
      {
        idSkipZeroes(res);
        if ((v!=NULL) && ((*v)!=NULL))
        {
          intvec *w=*v;
          int len=si_max(n-j,1);
          *v = new intvec( len /*si_max(n-j,1)*/ );
          for (i=0,j=0;i<si_min(w->length(),len);i++)
          {
            if((*w)[i]!=0)
            {
              (**v)[j]=(*w)[i]; j++;
            }
          }
          delete w;
        }
      }
      if (res->m[0]==NULL)
      {
        res->m[0]=pOne();
      }
    }
  }
  if (rField_is_Q_a() && (currRing->minpoly!=NULL))
  {
    int i=IDELEMS(res)-1;
    int stop=1;
    if (with_exps!=0) stop=0;
    for(;i>=stop;i--)
    {
      pNorm(res->m[i]);
    }
    if (with_exps==0) pSetCoeff(res->m[0],old_lead_coeff);
    else nDelete(&old_lead_coeff);
  }
  else
    nDelete(&old_lead_coeff);
  errorreported=save_errorreported;
notImpl:
  if (res==NULL)
    WerrorS( feNotImplemented );
  if (NN!=NULL)
  {
    nDelete(&NN);
  }
  if (N!=NULL)
  {
    nDelete(&N);
  }
  //if (f!=NULL) pDelete(&f);
  //PrintS("......S\n");
  return res;
}
ideal singclap_sqrfree ( poly f)
{
  pTest(f);
#ifdef FACTORIZE2_DEBUG
  printf("singclap_sqrfree, degree %d\n",pTotaldegree(f));
#endif
  // with_exps: 3,1 return only true factors, no exponents
  //            2 return true factors and exponents
  //            0 return coeff, factors and exponents
  BOOLEAN save_errorreported=errorreported;

  ideal res=NULL;

  // handle factorize(0) =========================================
  if (f==NULL)
  {
    res=idInit(1,1);
    return res;
  }
  // handle factorize(mon) =========================================
  if (pNext(f)==NULL)
  {
    int i=0;
    int n=0;
    int e;
    for(i=pVariables;i>0;i--) if(pGetExp(f,i)!=0) n++;
    n++; // with coeff
    res=idInit(si_max(n,1),1);
    res->m[0]=pOne();
    pSetCoeff(res->m[0],nCopy(pGetCoeff(f)));
    if (n==0)
    {
      res->m[0]=pOne();
      // (**v)[0]=1; is already done
      return res;
    }
    for(i=pVariables;i>0;i--)
    {
      e=pGetExp(f,i);
      if(e!=0)
      {
        n--;
        poly p=pOne();
        pSetExp(p,i,1);
        pSetm(p);
        res->m[n]=p;
      }
    }
    return res;
  }
  //PrintS("S:");pWrite(f);PrintLn();
  // use factory/libfac in general ==============================
  Off(SW_RATIONAL);
  On(SW_SYMMETRIC_FF);
  #ifdef HAVE_NTL
  extern int prime_number;
  if(rField_is_Q()) prime_number=0;
  #endif
  CFFList L;

  if (!rField_is_Zp() && !rField_is_Zp_a()) /* Q, Q(a) */
  {
    //if (f!=NULL) // already tested at start of routine
    {
      pCleardenom(f);
    }
  }
  else if (rField_is_Zp_a())
  {
    //if (f!=NULL) // already tested at start of routine
    if (singclap_factorize_retry==0)
    {
      pNorm(f);
      pCleardenom(f);
    }
  }
  if (rField_is_Q() || rField_is_Zp())
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPFactoryP( f ) );
    L = sqrFree( F );
  }
  #if 0
  else if (rField_is_GF())
  {
    int c=rChar(currRing);
    setCharacteristic( c, primepower(c) );
    CanonicalForm F( convSingGFFactoryGF( f ) );
    if (F.isUnivariate())
    {
      L = factorize( F );
    }
    else
    {
      goto notImpl;
    }
  }
  #endif
  // and over Q(a) / Fp(a)
  else if (rField_is_Extension())
  {
    if (rField_is_Q_a()) setCharacteristic( 0 );
    else                 setCharacteristic( -nGetChar() );
    if (currRing->minpoly!=NULL)
    {
      CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      CanonicalForm F( convSingAPFactoryAP( f,a ) );
      CFFList SqrFreeMV( const CanonicalForm & f , const CanonicalForm & mipo=0) ;

      L = SqrFreeMV( F,mipo );
      //WarnS("L = sqrFree( F,mipo );");
      //L = sqrFree( F );
    }
    else
    {
      CanonicalForm F( convSingTrPFactoryP( f ) );
      L = sqrFree( F );
    }
  }
  else
  {
    goto notImpl;
  }
  {
    poly ff=pCopy(f); // a copy for the retry stuff
    // convert into ideal
    int n = L.length();
    if (n==0) n=1;
    CFFListIterator J=L;
    int j=0;
    res = idInit( n ,1);
    for ( ; J.hasItem(); J++, j++ )
    {
      poly p;
      if (rField_is_Zp() || rField_is_Q())           /* Q, Fp */
        //count_Factors(res,*v,f, j, convFactoryPSingP( J.getItem().factor() );
        res->m[j] = convFactoryPSingP( J.getItem().factor() );
      #if 0
      else if (rField_is_GF())
        res->m[j] = convFactoryGFSingGF( J.getItem().factor() );
      #endif
      else if (rField_is_Extension())     /* Q(a), Fp(a) */
      {
        if (currRing->minpoly==NULL)
          res->m[j]=convFactoryPSingTrP( J.getItem().factor() );
        else
          res->m[j]=convFactoryAPSingAP( J.getItem().factor() );
      }
    }
    if (res->m[0]==NULL)
    {
      res->m[0]=pOne();
    }
  }
  errorreported=save_errorreported;
notImpl:
  if (res==NULL)
    WerrorS( feNotImplemented );
  return res;
}
matrix singclap_irrCharSeries ( ideal I)
{
#ifdef HAVE_LIBFAC_P
  if (idIs0(I)) return mpNew(1,1);

  // for now there is only the possibility to handle polynomials over
  // Q and Fp ...
  matrix res=NULL;
  int i;
  Off(SW_RATIONAL);
  On(SW_SYMMETRIC_FF);
  CFList L;
  ListCFList LL;
  if (((nGetChar() == 0) || (nGetChar() > 1) )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    for(i=0;i<IDELEMS(I);i++)
    {
      poly p=I->m[i];
      if (p!=NULL)
      {
        p=pCopy(p);
        pCleardenom(p);
        L.append(convSingPFactoryP(p));
      }
    }
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    for(i=0;i<IDELEMS(I);i++)
    {
      poly p=I->m[i];
      if (p!=NULL)
      {
        p=pCopy(p);
        pCleardenom(p);
        L.append(convSingTrPFactoryP(p));
      }
    }
  }
  else
  {
    WerrorS( feNotImplemented );
    return res;
  }

  // a very bad work-around --- FIX IT in libfac
  // should be fixed as of 2001/6/27
  int tries=0;
  int m,n;
  ListIterator<CFList> LLi;
  loop
  {
    LL=IrrCharSeries(L);
    m= LL.length(); // Anzahl Zeilen
    n=0;
    for ( LLi = LL; LLi.hasItem(); LLi++ )
    {
      n = si_max(LLi.getItem().length(),n);
    }
    if ((m!=0) && (n!=0)) break;
    tries++;
    if (tries>=5) break;
  }
  if ((m==0) || (n==0))
  {
    Warn("char_series returns %d x %d matrix from %d input polys (%d)",
      m,n,IDELEMS(I)+1,LL.length());
    iiWriteMatrix((matrix)I,"I",2,0);
    m=si_max(m,1);
    n=si_max(n,1);
  }
  res=mpNew(m,n);
  CFListIterator Li;
  for ( m=1, LLi = LL; LLi.hasItem(); LLi++, m++ )
  {
    for (n=1, Li = LLi.getItem(); Li.hasItem(); Li++, n++)
    {
      if ( (nGetChar() == 0) || (nGetChar() > 1) )
        MATELEM(res,m,n)=convFactoryPSingP(Li.getItem());
      else
        MATELEM(res,m,n)=convFactoryPSingTrP(Li.getItem());
    }
  }
  Off(SW_RATIONAL);
  return res;
#else
  return NULL;
#endif
}

char* singclap_neworder ( ideal I)
{
#ifdef HAVE_LIBFAC_P
  int i;
  Off(SW_RATIONAL);
  On(SW_SYMMETRIC_FF);
  CFList L;
  if (((nGetChar() == 0) || (nGetChar() > 1) )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    for(i=0;i<IDELEMS(I);i++)
    {
      L.append(convSingPFactoryP(I->m[i]));
    }
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    for(i=0;i<IDELEMS(I);i++)
    {
      L.append(convSingTrPFactoryP(I->m[i]));
    }
  }
  else
  {
    WerrorS( feNotImplemented );
    return NULL;
  }

  List<int> IL=neworderint(L);
  ListIterator<int> Li;
  StringSetS("");
  Li = IL;
  int offs=rPar(currRing);
  int* mark=(int*)omAlloc0((pVariables+offs)*sizeof(int));
  int cnt=pVariables+offs;
  loop
  {
    if(! Li.hasItem()) break;
    BOOLEAN done=TRUE;
    i=Li.getItem()-1;
    mark[i]=1;
    if (i<offs)
    {
      done=FALSE;
      //StringAppendS(currRing->parameter[i]);
    }
    else
    {
      StringAppendS(currRing->names[i-offs]);
    }
    Li++;
    cnt--;
    if(cnt==0) break;
    if (done) StringAppendS(",");
  }
  for(i=0;i<pVariables+offs;i++)
  {
    BOOLEAN done=TRUE;
    if(mark[i]==0)
    {
      if (i<offs)
      {
        done=FALSE;
        //StringAppendS(currRing->parameter[i]);
      }
      else
      {
        StringAppendS(currRing->names[i-offs]);
      }
      cnt--;
      if(cnt==0) break;
      if (done) StringAppendS(",");
    }
  }
  char * s=omStrDup(StringAppendS(""));
  if (s[strlen(s)-1]==',') s[strlen(s)-1]='\0';
  return s;
#else
  return NULL;
#endif
}

BOOLEAN singclap_isSqrFree(poly f)
{
  BOOLEAN b=FALSE;
  Off(SW_RATIONAL);
  //  Q / Fp
  if (((nGetChar() == 0) || (nGetChar() > 1) )
  &&(currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CanonicalForm F( convSingPFactoryP( f ) );
    if((nGetChar()>1)&&(!F.isUnivariate()))
      goto err;
    b=(BOOLEAN)isSqrFree(F);
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    //if (currRing->minpoly!=NULL)
    //{
    //  CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
    //  Variable a=rootOf(mipo);
    //  CanonicalForm F( convSingAPFactoryAP( f,a ) );
    //  ...
    //}
    //else
    {
      CanonicalForm F( convSingTrPFactoryP( f ) );
      b=(BOOLEAN)isSqrFree(F);
    }
    Off(SW_RATIONAL);
  }
  else
  {
err:
    WerrorS( feNotImplemented );
  }
  return b;
}

poly singclap_det( const matrix m )
{
  int r=m->rows();
  if (r!=m->cols())
  {
    Werror("det of %d x %d matrix",r,m->cols());
    return NULL;
  }
  poly res=NULL;
  if (( nGetChar() == 0 || nGetChar() > 1 )
  && (currRing->parameter==NULL))
  {
    setCharacteristic( nGetChar() );
    CFMatrix M(r,r);
    int i,j;
    for(i=r;i>0;i--)
    {
      for(j=r;j>0;j--)
      {
        M(i,j)=convSingPFactoryP(MATELEM(m,i,j));
      }
    }
    res= convFactoryPSingP( determinant(M,r) ) ;
  }
  // and over Q(a) / Fp(a)
  else if (( nGetChar()==1 ) /* Q(a) */
  || (nGetChar() <-1))       /* Fp(a) */
  {
    if (nGetChar()==1) setCharacteristic( 0 );
    else               setCharacteristic( -nGetChar() );
    CFMatrix M(r,r);
    poly res;
    if (currRing->minpoly!=NULL)
    {
      CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
      Variable a=rootOf(mipo);
      int i,j;
      for(i=r;i>0;i--)
      {
        for(j=r;j>0;j--)
        {
          M(i,j)=convSingAPFactoryAP(MATELEM(m,i,j),a);
        }
      }
      res= convFactoryAPSingAP( determinant(M,r) ) ;
    }
    else
    {
      int i,j;
      for(i=r;i>0;i--)
      {
        for(j=r;j>0;j--)
        {
          M(i,j)=convSingTrPFactoryP(MATELEM(m,i,j));
        }
      }
      res= convFactoryPSingTrP( determinant(M,r) );
    }
  }
  else
    WerrorS( feNotImplemented );
  Off(SW_RATIONAL);
  return res;
}

int singclap_det_i( intvec * m )
{
  setCharacteristic( 0 );
  CFMatrix M(m->rows(),m->cols());
  int i,j;
  for(i=m->rows();i>0;i--)
  {
    for(j=m->cols();j>0;j--)
    {
      M(i,j)=IMATELEM(*m,i,j);
    }
  }
  int res= convFactoryISingI( determinant(M,m->rows())) ;
  Off(SW_RATIONAL);
  return res;
}
napoly singclap_alglcm ( napoly f, napoly g )
{
 FACTORY_ALGOUT( "f", f );
 FACTORY_ALGOUT( "g", g );

 // over Q(a) / Fp(a)
 if (nGetChar()==1) setCharacteristic( 0 );
 else               setCharacteristic( -nGetChar() );
 napoly res;

 if (currRing->minpoly!=NULL)
 {
   CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
   Variable a=rootOf(mipo);
   CanonicalForm F( convSingAFactoryA( f,a ) ), G( convSingAFactoryA( g,a ) );
   CanonicalForm GCD;

   TIMING_START( algLcmTimer );
   // calculate gcd
#ifdef FACTORY_GCD_TEST
   GCD = CFPrimitiveGcdUtil::gcd( F, G );
#else
   GCD = gcd( F, G );
#endif
   TIMING_END( algLcmTimer );

   FACTORY_CFAOUT( "d", GCD );
   FACTORY_GCDSTAT( "alcm:", F, G, GCD );

   // calculate lcm
   res= convFactoryASingA( (F/GCD)*G );
 }
 else
 {
   CanonicalForm F( convSingTrFactoryP( f ) ), G( convSingTrFactoryP( g ) );
   CanonicalForm GCD;
   TIMING_START( algLcmTimer );
   // calculate gcd
#ifdef FACTORY_GCD_TEST
   GCD = CFPrimitiveGcdUtil::gcd( F, G );
#else
   GCD = gcd( F, G );
#endif
   TIMING_END( algLcmTimer );

   FACTORY_CFTROUT( "d", GCD );
   FACTORY_GCDSTAT( "alcm:", F, G, GCD );

   // calculate lcm
   res= convFactoryPSingTr( (F/GCD)*G );
 }

 Off(SW_RATIONAL);
 return res;
}

void singclap_algdividecontent ( napoly f, napoly g, napoly &ff, napoly &gg )
{
 FACTORY_ALGOUT( "f", f );
 FACTORY_ALGOUT( "g", g );

 // over Q(a) / Fp(a)
 if (nGetChar()==1) setCharacteristic( 0 );
 else               setCharacteristic( -nGetChar() );
 ff=gg=NULL;
 On(SW_RATIONAL);

 if (currRing->minpoly!=NULL)
 {
   CanonicalForm mipo=convSingTrFactoryP(((lnumber)currRing->minpoly)->z);
   Variable a=rootOf(mipo);
   CanonicalForm F( convSingAFactoryA( f,a ) ), G( convSingAFactoryA( g,a ) );
   CanonicalForm GCD;

   TIMING_START( algContentTimer );
#ifdef FACTORY_GCD_TEST
   GCD=CFPrimitiveGcdUtil::gcd( F, G );
#else
   GCD=gcd( F, G );
#endif
   TIMING_END( algContentTimer );

   FACTORY_CFAOUT( "d", GCD );
   FACTORY_GCDSTAT( "acnt:", F, G, GCD );

   if ((GCD!=1) && (GCD!=0))
   {
     ff= convFactoryASingA( F/ GCD );
     gg= convFactoryASingA( G/ GCD );
   }
 }
 else
 {
   CanonicalForm F( convSingTrFactoryP( f ) ), G( convSingTrFactoryP( g ) );
   CanonicalForm GCD;

   TIMING_START( algContentTimer );
#ifdef FACTORY_GCD_TEST
   GCD=CFPrimitiveGcdUtil::gcd( F, G );
#else
   GCD=gcd( F, G );
#endif
   TIMING_END( algContentTimer );

   FACTORY_CFTROUT( "d", GCD );
   FACTORY_GCDSTAT( "acnt:", F, G, GCD );

   if ((GCD!=1) && (GCD!=0))
   {
     ff= convFactoryPSingTr( F/ GCD );
     gg= convFactoryPSingTr( G/ GCD );
   }
 }

 Off(SW_RATIONAL);
}

#if 0
lists singclap_chineseRemainder(lists x, lists q)
{
  //assume(x->nr == q->nr);
  //assume(x->nr >= 0);
  int n=x->nr+1;
  if ((x->nr<0) || (x->nr!=q->nr))
  {
    WerrorS("list are empty or not of equal length");
    return NULL;
  }
  lists res=(lists)omAlloc0Bin(slists_bin);
  CFArray X(1,n), Q(1,n);
  int i;
  for(i=0; i<n; i++)
  {
    if (x->m[i-1].Typ()==INT_CMD)
    {
      X[i]=(int)x->m[i-1].Data();
    }
    else if (x->m[i-1].Typ()==NUMBER_CMD)
    {
      number N=(number)x->m[i-1].Data();
      X[i]=convSingNFactoryN(N);
    }
    else
    {
      WerrorS("illegal type in chineseRemainder");
      omFreeBin(res,slists_bin);
      return NULL;
    }
    if (q->m[i-1].Typ()==INT_CMD)
    {
      Q[i]=(int)q->m[i-1].Data();
    }
    else if (q->m[i-1].Typ()==NUMBER_CMD)
    {
      number N=(number)x->m[i-1].Data();
      Q[i]=convSingNFactoryN(N);
    }
    else
    {
      WerrorS("illegal type in chineseRemainder");
      omFreeBin(res,slists_bin);
      return NULL;
    }
  }
  CanonicalForm r, prod;
  chineseRemainder( X, Q, r, prod );
  res->Init(2);
  res->m[0].rtyp=NUMBER_CMD;
  res->m[1].rtyp=NUMBER_CMD;
  res->m[0].data=(char *)convFactoryNSingN( r );
  res->m[1].data=(char *)convFactoryNSingN( prod );
  return res;
}
#endif
#endif
