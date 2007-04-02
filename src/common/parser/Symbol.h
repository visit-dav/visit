#ifndef SYMBOL_H
#define SYMBOL_H
#include <parser_exports.h>

#include <vector>
#include <string>
#include <map>
#include <visitstream.h>

#define MAXSYMBOLS 64
struct SymbolSet;
class  Rule;
class  Dictionary;

// ****************************************************************************
//  Class:  Symbol
//
//  Purpose:
//    Used for terminals and nonterminals when expressing and
//    parsing a grammar.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 24 12:04:23 PST 2004
//    Added a new constructor due to some major refactoring.
//
//    Jeremy Meredith, Wed Jun  8 11:28:01 PDT 2005
//    Moved static data to a new Dictionary object.
//
// ****************************************************************************
class PARSER_API Symbol
{
  public:
    enum Type { Terminal, NonTerminal };

  public:
    Symbol(Dictionary&,int tt);
    Symbol(Dictionary&,int tt, const std::string &s);
    Symbol(Dictionary&,const std::string &s);

    bool      operator==(const Symbol &rhs) const;
    bool      IsNullable(const std::vector<const Rule*>&)  const;
    SymbolSet GetFirstSet(const std::vector<const Rule*>&) const;

    int  GetIndex()      const { return index;               }
    bool IsTerminal()    const { return type == Terminal;    }
    bool IsNonTerminal() const { return type == NonTerminal; }

    friend ostream &operator<<(ostream&, const Symbol&);

    int         GetTerminalType()  const { return terminaltype;  }
    std::string GetDisplayString() const { return displaystring; }
  private:
    Type         type;
    int          terminaltype;
    std::string  displaystring;
    int          index;
};

#endif
