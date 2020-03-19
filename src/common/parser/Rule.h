// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef RULE_H
#define RULE_H
#include <parser_exports.h>

#include "Symbol.h"
#include "Sequence.h"

// ****************************************************************************
//  Class:  Rule
//
//  Purpose:
//    A production rule in a grammar.  It has a nonterminal on the left
//    and a sequence on the right.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
class PARSER_API Rule
{
  public:
    Rule();
    Rule(int, const Symbol&);

    Rule &operator>>(const Sequence&);

    bool  operator==(const Rule &r) const {return lhs == r.lhs && rhs == r.rhs; }
    void  operator=(const Rule &r) { index = r.index; lhs = r.lhs; rhs = r.rhs; }

    void  Print(ostream &o, int pos = -1) const;
    void  PrintNoColor(ostream &o, int pos = -1) const;
    bool  IsNullable(const std::vector<const Rule*> &rules) const;

    const Symbol   *GetLHS()        const { return lhs; }
    const Sequence &GetRHS()        const { return rhs; }
    int             GetID()         const { return id; }
    int             GetPrec()       const { return prec; }
    int             GetIndex()      const { return index; }
    void            SetPrec(int p)        { prec = p; }
    void            SetIndex(int i)       { index = i; }
  private:
    const Symbol  *lhs;
    Sequence       rhs;
    int            id;     // client-specified identifier, used by same client
    int            prec;   // precision of rule
    int            index;  // index among all rules
};

PARSER_API ostream &operator<<(ostream &o, const Rule &r);


#endif
