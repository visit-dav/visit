#ifndef PARSERBASE_H
#define PARSERBASE_H
#include <parser_exports.h>

#include <vector>
#include <visitstream.h>

#include <ParserInterface.h>
#include <Token.h>
#include <ExprGrammarNode.h>
#include <Symbol.h>
#include <State.h>
#include <Grammar.h>

// ****************************************************************************
//  Class:  ParseElem
//
//  Purpose:
//    An element of a parse stack.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
struct PARSER_API ParseElem
{
    const Symbol *sym;
    ExprGrammarNode *node;

    ParseElem(Token * t) { sym = Symbol::Get(t->GetType()); node = t; }
    ParseElem(const Symbol * s, ExprGrammarNode * n) { sym = s; node = n; }
};

// ****************************************************************************
//  Class:  ParserBase
//
//  Purpose:
//    Abstract base for a parser.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
class PARSER_API ParserBase: public ParserInterface
{
public:
    ParserBase();
    virtual ~ParserBase() {}
    void    Init();
    void    ParseOneToken(Token *);
    bool    Accept() { return accept; }
    ExprGrammarNode *GetParseTree() { return elems[0].node; }
    void    SetGrammar(Grammar * g) { G = g; }

protected:
    Grammar *G;
    std::vector<int> states;
    std::vector<ParseElem> elems;
    bool    accept;

protected:
    void    Shift(Token *, int);
    void    Reduce(int);
    virtual ExprGrammarNode *ApplyRule(const Symbol &, const Rule *,
                                std::vector < ExprGrammarNode * >&, Pos) = 0;
    void    PrintState(ostream &);
};

#endif
