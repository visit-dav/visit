#ifndef SYMBOLSET_H
#define SYMBOLSET_H
#include <viewer_parser_exports.h>

#include "Token.h"
#include "Symbol.h"

// ****************************************************************************
//  Class:  SymbolSet
//
//  Purpose:
//    Acts much as a set<Symbol*>, except it is a fixed size and one may
//    reference elements which don't exist in the set.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
struct VIEWER_PARSER_API SymbolSet
{
    const Symbol *set[MAXSYMBOLS];

  public:
    SymbolSet()
    {
        memset(set, 0, sizeof(const Symbol*)*MAXSYMBOLS);
    }
    int  size() const
    {
        return MAXSYMBOLS;
    }
    void insert(const Symbol *s)
    {
        set[s->GetIndex()] = s;
    }
    void merge(const SymbolSet &t)
    {
        for (int i=0; i<MAXSYMBOLS; i++)
            if (t.set[i]) set[i]=t.set[i];
    }
    bool contains(const Symbol *s) const
    {
        return set[s->GetIndex()];
    }
    bool operator==(const SymbolSet &t) const
    {
        for (int i=0; i<MAXSYMBOLS; i++)
            if (t.set[i] != set[i])
                return false;
        return true;
    }
};


#endif
