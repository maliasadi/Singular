#ifndef GRING_SA_CACHEHASH_H
#define GRING_SA_CACHEHASH_H
/*****************************************
 *  Computer Algebra System SINGULAR     *
 *****************************************/
/* $Id: ncSACache.h,v 1.4 2008-07-21 00:05:09 motsak Exp $ */

// #include <ncSACache.h> // for CCacheHash etc classes

#include <structs.h>
#include <febase.h> // for Print!
// //////////////////////////////////////////////////////////////////////// //
//

const int iMaxCacheSize = 20;

template <typename CExponent>
class CCacheHash
{
  private:
    ring m_basering;
    int m_NVars;
    
  public:
    CCacheHash(ring r): m_basering(r), m_NVars(r->N){};

    const ring GetBasering() const { return m_basering; };
    inline int NVars() const { return m_NVars; }
    
    virtual ~CCacheHash(){};

    
    enum EHistoryType {
      MULT_LOOKUP = 0,
      MULT_STORE  = 1
    };

    struct CCacheItem
    {
      union{
        CExponent aExponent;
        poly aMonom;
      } a;

      union{
        CExponent bExponent;
        poly bMonom;
      } b;

      poly pProduct;

      int iPairType;
      long lHits;
    };

    
    // -1 means no hits!
    int LookupEE(CExponent a, CExponent b, CCacheItem*& pItems)
    {
/*
      Print("//////////////////////////////////////////////////////////////////////////////////////////////");
      PrintLn();
      Print("CCacheHash::LookupEE(a, b, *results)!");
      PrintLn();
*/
      History(MULT_LOOKUP, a, b);
      
      pItems = NULL;
      return -1;
    }

    bool StoreEE(CExponent a, CExponent b, poly pProduct)
    {
/*
      Print("CCacheHash::StoreEE(a, b, Product)!");
      PrintLn();
*/
      
      History(MULT_STORE, a, b, pProduct);

/*
      Print("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\");
      PrintLn();
*/
      
      return false; // the pair was not stored!
    };
    
    virtual void History(const EHistoryType t, const CExponent a, const CExponent b, const poly p = NULL)
    {
      Print("CCacheHash::History(a, b, [p])!");
      PrintLn();
    }

  private: // no copy constuctors!
    CCacheHash(const CCacheHash&);
    CCacheHash& operator=(const CCacheHash&);
};



class CGlobalCacheHash: public CCacheHash<poly>
{
  public:
    typedef poly CExponent;

    CGlobalCacheHash(ring r): CCacheHash<poly>(r) {};

    virtual ~CGlobalCacheHash() {};

  protected:
    virtual void History(const EHistoryType t, const CExponent a, const CExponent b, const poly p = NULL);
};

class CSpecialPairCacheHash: public CCacheHash<int>
{
  public:
    typedef int CExponent;

    CSpecialPairCacheHash(ring r): CCacheHash<int>(r) {};

    virtual ~CSpecialPairCacheHash() {};

  protected:
    virtual void History(const EHistoryType t, const CExponent a, const CExponent b, const poly p = NULL);
};



#endif // GRING_SA_CACHEHASH_H


