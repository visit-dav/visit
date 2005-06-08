#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <parser_exports.h>

#include <vector>
#include <string>
#include <map>

class Symbol;

// ****************************************************************************
//  Class:  Dictionary
//
//  Purpose:
//    Used to keep track of all the symbols in a grammar.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  8, 2005
//
// ****************************************************************************
class PARSER_API Dictionary
{
  public:
            Dictionary();
    Symbol *Get(int tt);
    Symbol *Get(const std::string &s);

    int AddTerminal(Symbol *s);
    int AddNonTerminal(Symbol *s);

  private:
    std::map<int,Symbol*>          allterminals;
    std::map<std::string,Symbol*>  allnonterminals;
    int                            nsymbols;
};

#endif
