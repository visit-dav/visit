// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_PARSER_H
#define VISIT_PARSER_H
#include <parser_exports.h>

#include <vector>
#include <visitstream.h>

#include <Token.h>
#include <ParseTreeNode.h>
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
//  Modifications:
//    Jeremy Meredith, Wed Nov 24 11:59:11 PST 2004
//    Major refactoring.  Added storage of Token here because it no longer
//    derives directly from a parse tree node.
//
//    Jeremy Meredith, Wed Jun  8 16:32:35 PDT 2005
//    Changed token based constructor to take the symbol as passed in.
//    This was to remove all static data and have the Parser look up
//    the symbol in a dictionary.
//
//    Jeremy Meredith, Mon Jun 13 15:59:20 PDT 2005
//    Removed TokenParseTreeNode -- it had become superfluous and was keeping
//    an extra pointer to Tokens around, preventing good memory management.
//    Added pos here to prevent having to look in the node or token members.
//
// ****************************************************************************
struct PARSER_API ParseElem
{
    const Symbol   *sym;
    ParseTreeNode  *node;
    Token          *token;
    Pos             pos;

    ParseElem(const Symbol *s, Token * t)
    {
        sym = s;
        node = NULL;
        token = t;
        pos = t->GetPos();
    }
    ParseElem(const Symbol *s, ParseTreeNode *n)
    {
        sym = s;
        node = n;
        token = NULL;
        pos = n->GetPos();
    }
};

// ****************************************************************************
//  Class:  Parser
//
//  Purpose:
//    Abstract base for a parser.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 24 11:59:55 PST 2004
//    Removed the ParserInterface base class (it wasn't doing much good).
//    Renamed this class Parser for consistency.  Added list of tokens
//    to the rule reduction method to make life easier for the implementor.
//
//    Jeremy Meredith, Mon Nov 17 17:04:43 EST 2008
//    Don't use the element stack to store the final parse tree; it's not
//    really a parse element, because there's not really a lhs other than
//    START, which doesn't make much sense.  Instead, we just store it in
//    a new data member, which is more direct and more clear.
//
//    Tom Fogal, Wed Apr 29 15:36:42 MDT 2009
//    Check for empty `elems' so we don't deref an empty vector.
//
//    Kathleen Bonnell, Tue May  5 17:23:42 PDT 2009
//    Revert GetParseTree changes back to Jeremy's fix from Nov 17, 2008.
//
// ****************************************************************************
class PARSER_API Parser
{
public:
    Parser();
    virtual ~Parser();
    void    Init();
    void    ParseOneToken(Token *);
    bool    Accept() { return accept; }
    virtual ParseTreeNode *Parse(const std::string &) = 0;
    ParseTreeNode *GetParseTree() { return parseTree; }
    void    SetGrammar(Grammar * g) { G = g; }

protected:
    Grammar *G;
    std::vector<int> states;
    std::vector<ParseElem> elems;
    bool    accept;
    ParseTreeNode *parseTree;

protected:
    void    Shift(Token *, int);
    void    Reduce(int);
    virtual ParseTreeNode *ApplyRule(const Symbol &, const Rule *,
                                   std::vector<ParseTreeNode*>&,
                                   std::vector<Token*>&,
                                   Pos) = 0;
    void    PrintState(ostream &);
};

#endif
