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
//    Jeremy Meredith, Mon Jul 28 16:54:47 PDT 2003
//    Add the ability to set the error reporting message target to 
//    the console, the viewer error message mechanism, or neither.
//
// ****************************************************************************

class VIEWER_API ExprParser : private ParserBase
{
  public:
    ExprParser();
    ExprNode *Parse(const std::string &);

    enum ErrorMessageTarget
    {
        EMT_NONE,
        EMT_CONSOLE,
        EMT_VIEWER
    };
    static void SetErrorMessageTarget(const ErrorMessageTarget emt)
    {
        errorMessageTarget = emt;
    }

  protected:
    ExprGrammarNode *ApplyRule(const Symbol&, const Rule*,
                               std::vector<ExprGrammarNode*>&, Pos);

  private:
    Scanner scanner;
    static ErrorMessageTarget errorMessageTarget;
};

#endif
