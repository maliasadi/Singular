/* Copyright 1996 Michael Messollen. All rights reserved. */
////////////////////////////////////////////////////////////
// emacs edit mode for this file is -*- C++ -*-
////////////////////////////////////////////////////////////
// $Id: alg_factor.h,v 1.1.1.1 1997-09-11 10:58:22 Singular Exp $
////////////////////////////////////////////////////////////

#ifndef INCL_NEW_ALGFACTOR_H
#define INCL_NEW_ALGFACTOR_H

#include <factory.h>
#include <tmpl_inst.h>  // for typedef's

// missing class: IntGenerator:
class IntGenerator : public CFGenerator
{
private:
    int current;
public:
    IntGenerator() : current(0) {}
    ~IntGenerator() {}
    bool hasItems() const;
    void reset() { current = 0; }
    CanonicalForm item() const;
    void next();
    void operator++ () { next(); }
    void operator++ ( int ) { next(); }
};

CFFList newfactoras( const CanonicalForm & f, const CFList & as, int success);
CFFList newcfactor(const CanonicalForm & f, const CFList & as, int success );
/*BEGINPUBLIC*/
/*ENDPUBLIC*/

#endif /* INCL_ALGFACTOR_H */
