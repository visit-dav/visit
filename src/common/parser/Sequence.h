// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SEQUENCE_H
#define SEQUENCE_H
#include <parser_exports.h>

#include "Symbol.h"
#include "SymbolSet.h"

class Rule;

// ****************************************************************************
//  Class:  Sequence
//
//  Purpose:
//    A series of terminals and nonterminals.  Used as the RHS of a
//    production (Rule).
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov  3 12:16:49 PST 2004
//    Added prototypes before the friend declarations.
//
//    Brad Whitlock, Tue Jan 4 14:37:51 PST 2005
//    Added API macros to the friend function declarations.
//
// ****************************************************************************

// Some compilers (xlC) require prototypes before friend declarations
class Sequence;
PARSER_API Sequence operator+(const Symbol&, const Symbol&);
PARSER_API Sequence operator+(const Sequence&, const Symbol&);

class PARSER_API Sequence
{
  public:
    Sequence();
    Sequence(const Sequence&);
    Sequence(const Symbol&);

    void AddSymbol(const Symbol&);

    int           Length()          const { return len;        }
    bool          Empty()           const { return len == 0;   }
    const Symbol *operator[](int i) const { return symbols[i]; }

    void Print(ostream &o, int pos = -1) const;
    bool operator==(const Sequence &s) const;

    bool          IsNullable(const std::vector<const Rule*> &rules) const;
    SymbolSet     GetFirstSet(const std::vector<const Rule*> &rules) const;
    Sequence      GetSubsequence(int pos) const;
    const Symbol *GetLastTerminal() const;

    friend PARSER_API Sequence operator+(const Symbol&, const Symbol&);
    friend PARSER_API Sequence operator+(const Sequence&, const Symbol&);
  protected:
    int len;
    std::vector<const Symbol*> symbols;
};

#endif
