#ifndef EXPRPARSER_H
#define EXPRPARSER_H
#include <expr_exports.h>

#include <VisItParser.h>
#include <ExprScanner.h>
#include <string>
#include <ExprNode.h>
#include <ExprNodeFactory.h>

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
//    Jeremy Meredith, Fri Aug 15 09:25:04 PDT 2003
//    Renamed EMT_VIEWER to EMT_COMPONENT and added EMT_EXCEPTION.
//
//    Jeremy Meredith, Wed Nov 24 11:55:13 PST 2004
//    Refactored.  There's a new base class for the ExprParser and the
//    return types became more general.
//
//    Jeremy Meredith, Tue Dec 28 11:18:09 PST 2004
//    Added the current parse text as a data member to help with macro
//    support.
//
// ****************************************************************************

class EXPR_API ExprParser : public Parser
{
  public:
    ExprParser(ExprNodeFactory *f);
    ParseTreeNode *Parse(const std::string &);

    enum ErrorMessageTarget
    {
        EMT_NONE,
        EMT_CONSOLE,
        EMT_COMPONENT,
        EMT_EXCEPTION
    };
    static void SetErrorMessageTarget(const ErrorMessageTarget emt)
    {
        errorMessageTarget = emt;
    }

  protected:
    ParseTreeNode *ApplyRule(const Symbol&, const Rule*,
                           std::vector<ParseTreeNode*>&,
                           std::vector<Token*>&, Pos);

  private:
    std::string text; // the current parse text

    ExprScanner scanner;
    ExprNodeFactory *factory;
    static ErrorMessageTarget errorMessageTarget;
};

#endif
