#include "Symbol.h"
#include "Rule.h"
#include "Token.h"
using std::vector;
using std::string;
using std::map;


map<int,Symbol*>    *Symbol::allterminals    = NULL;
map<string,Symbol*> *Symbol::allnonterminals = NULL;
int                  Symbol::nsymbols        = 0;

// ****************************************************************************
//  Constructor:  Symbol::Symbol
//
//  Purpose:
//    Creates a terminal from the token type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Symbol::Symbol(int tt)
    : type(Terminal), terminaltype(tt), displaystring(GetTokenTypeString(tt))
{
    InitStatic();
    if (nsymbols>=MAXSYMBOLS) { cerr << "Too many symbols! Increase MAXSYMBOLS\n"; exit(-1); }
    index=nsymbols++;
    (*allterminals)[tt] = this;
}

// ****************************************************************************
//  Constructor:  Symbol::Symbol
//
//  Purpose:
//    Creates a terminal from the token type, overriding the display
//    string with a known one.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 24, 2004
//
// ****************************************************************************
Symbol::Symbol(int tt, const string &s)
    : type(Terminal), terminaltype(tt), displaystring(s)
{
    InitStatic();
    if (nsymbols>=MAXSYMBOLS) { cerr << "Too many symbols! Increase MAXSYMBOLS\n"; exit(-1); }
    index=nsymbols++;
    (*allterminals)[tt] = this;
}

// ****************************************************************************
//  Constructor:  Symbol::Symbol
//
//  Purpose:
//    Creates a nonterminal from its name.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Symbol::Symbol(const string &s)
    : type(NonTerminal), terminaltype(0), displaystring(s)
{
    InitStatic();
    if (nsymbols>=MAXSYMBOLS) { cerr << "Too many symbols! Increase MAXSYMBOLS\n"; exit(-1); }
    index=nsymbols++;
    (*allnonterminals)[s] = this;
}

// ****************************************************************************
//  Method:  Symbol::Get
//
//  Purpose:
//    Finds a terminal by the token type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Symbol*
Symbol::Get(int tt)
{
    return (*allterminals)[tt];
}

// ****************************************************************************
//  Method:  Symbol::Get
//
//  Purpose:
//    Finds a nonterminal by its name.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
Symbol*
Symbol::Get(const string &s)
{
    return (*allnonterminals)[s];
}

// ****************************************************************************
//  Method:  Symbol::operator==
//
//  Purpose:
//    Tests for equality.  The displaystrings should always be unique.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
Symbol::operator==(const Symbol &rhs) const
{
    return displaystring == rhs.displaystring;
}

// ****************************************************************************
//  Method:  Symbol::InitStatic
//
//  Purpose:
//    Inits all the static data if it hasn't been done yet.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
void
Symbol::InitStatic()
{
    if (!allterminals)
    {
        allterminals    = new map<int,Symbol*>;
        allnonterminals = new map<string,Symbol*>;
        nsymbols = 0;
    }
}

// ****************************************************************************
//  Method:  operator<<(ostream, Symbol)
//
//  Purpose:
//    Prints the symbol.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ostream &operator<<(ostream &o, const Symbol &s)
{
    o << s.displaystring;
    return o;
}

// ****************************************************************************
//  Method:  Symbol::IsNullable
//
//  Purpose:
//    Checks if a symbol can produce, even indirectly, the null string.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
bool
Symbol::IsNullable(const vector<const Rule*> &rules) const
{
    if (type == Terminal)
        return false;

    for (int i=0; i<rules.size(); i++)
    {
        if (rules[i]->GetLHS() == this)
        {
            if (rules[i]->IsNullable(rules))
                return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Method:  Symbol::GetFirstSet
//
//  Purpose:
//    Gets the first set for any production of this symbol.
//    For terminals, the first set is the identity operation.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
SymbolSet
Symbol::GetFirstSet(const vector<const Rule*> &rules) const
{
    SymbolSet first;
    if (type == Terminal)
    {
        first.insert(this);
    }
    else
    {
        for (int i=0; i<rules.size(); i++)
        {
            if (rules[i]->GetLHS() == this &&
                // Try to avoid infinite recursion -- this should be improved!
                (rules[i]->GetRHS().Empty() || rules[i]->GetRHS()[0] != this))
            {
                first.merge( rules[i]->GetRHS().GetFirstSet(rules) );
            }
        }
    }
    return first;
}
