#ifndef EXPRPARSER_H
#define EXPRPARSER_H
#include <viewer_exports.h>

#include <ParserBase.h>
#include <Scanner.h>
#include <string>
#include <ExprNode.h>

// ****************************************************************************
//  Class:  ExprParser
//
//  Purpose:
//    Implementation of a parser for the Expression grammar.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Jun 28 15:20:36 PST 2002
//    Changed the name of the base class.
//
// ****************************************************************************

class VIEWER_API ExprParser : private ParserBase
{
  public:
    ExprParser();
    ExprNode *Parse(const std::string &);

  protected:
    ExprGrammarNode *ApplyRule(const Symbol&, const Rule*,
                               std::vector<ExprGrammarNode*>&, Pos);

  private:
    Scanner scanner;
};

#endif
