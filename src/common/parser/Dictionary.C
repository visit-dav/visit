#include "Dictionary.h"

#include "Symbol.h"

// ****************************************************************************
//  Constructor:  Dictionary::Dictionary
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2005
//
// ****************************************************************************
Dictionary::Dictionary()
{
    nsymbols = 0;
}

// ****************************************************************************
//  Method:  Dictionary::AddTerminal
//
//  Purpose:
//    Adds a terminal symbol to the dictionary.
//
//  Arguments:
//    s          the symbol
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2005
//
// ****************************************************************************
int
Dictionary::AddTerminal(Symbol *s)
{
    int index = nsymbols++;
    if (nsymbols > MAXSYMBOLS)
    {
        cerr << "Too many symbols! Increase MAXSYMBOLS" << endl;
        exit(-1);
    }
    allterminals[s->GetTerminalType()] = s;
    return index;
}

// ****************************************************************************
//  Method:  Dictionary::AddNonTerminal
//
//  Purpose:
//    Adds a nonterminal symbol to the dictionary.
//
//  Arguments:
//    s          the symbol
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2005
//
// ****************************************************************************
int
Dictionary::AddNonTerminal(Symbol *s)
{
    int index = nsymbols++;
    if (nsymbols > MAXSYMBOLS)
    {
        cerr << "Too many symbols! Increase MAXSYMBOLS" << endl;
        exit(-1);
    }
    allnonterminals[s->GetDisplayString()] = s;
    return index;
}

// ****************************************************************************
//  Method:  Dictionary::Get
//
//  Purpose:
//    Finds a terminal by the token type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2005
//
// ****************************************************************************
Symbol*
Dictionary::Get(int tt)
{
    return allterminals[tt];
}

// ****************************************************************************
//  Method:  Dictionary::Get
//
//  Purpose:
//    Finds a nonterminal by its name.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2005
//
// ****************************************************************************
Symbol*
Dictionary::Get(const std::string &s)
{
    return allnonterminals[s];
}

