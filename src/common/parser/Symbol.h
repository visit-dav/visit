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
// ****************************************************************************
class PARSER_API Symbol
{
  public:
    enum Type { Terminal, NonTerminal };

  public:
                   Symbol(int tt);
                   Symbol(const std::string &s);
    static Symbol *Get(int tt);
    static Symbol *Get(const std::string &s);

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
    void InitStatic();

    static std::map<int,Symbol*>         *allterminals;
    static std::map<std::string,Symbol*> *allnonterminals;
    static int                            nsymbols;

    Type         type;
    int          terminaltype;
    std::string  displaystring;
    int          index;
};

#endif
