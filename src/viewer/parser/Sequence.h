#ifndef SEQUENCE_H
#define SEQUENCE_H
#include <viewer_parser_exports.h>

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
// ****************************************************************************
class VIEWER_PARSER_API Sequence
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

    friend Sequence operator+(const Symbol&, const Symbol&);
    friend Sequence operator+(const Sequence&, const Symbol&);
  protected:
    int len;
    std::vector<const Symbol*> symbols;
};

#endif
