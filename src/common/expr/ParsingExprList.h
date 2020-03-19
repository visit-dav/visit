// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PARSING_EXPR_LIST_H
#define PARSING_EXPR_LIST_H
#include <expr_exports.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <SimpleObserver.h>
#include <VisItParser.h>
#include <string> 

// Forward declarations.
class ExprNode;

// ****************************************************************************
// Class: ParsingExprList
//
// Purpose:
//   This class contains a list of expressions, and the ability to parse them.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 14:17:12 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Fri Aug 15 11:33:25 PDT 2003
//    Added a new form of GetExpressionTree that takes an Expression.
//
//    Jeremy Meredith, Wed Nov 24 12:27:15 PST 2004
//    Refactored and changed parse tree classes around.
//
//    Jeremy Meredith, Tue Dec 14 14:09:51 PST 2004
//    I added code to get the first real (database, non-expression) variable
//    for a given variable (which may or may not be an expression).
//
//    Hank Childs, Mon Jan  8 10:35:38 PST 2007
//    Added GetExpressionTypeFromAVT.
//
//    Brad Whitlock, Tue Jan 20 15:49:56 PST 2009
//    I removed some methods that caused AVT to bleed into this library.
//
// ****************************************************************************

class EXPR_API ParsingExprList : public SimpleObserver
{
public:
    ParsingExprList();
    ParsingExprList(Parser *p);
    ~ParsingExprList();

    static ParsingExprList *Instance();
    static Expression const * GetExpression(const char *varname);
    static Expression const * GetExpression(std::string varname)
        { return GetExpression(varname.c_str()); }

    ExpressionList *GetList() {return &exprList;}

    virtual void Update(Subject *);

    static ExprNode *GetExpressionTree(Expression const *);
    static ExprNode *GetExpressionTree(const char *varname);
    static ExprNode *GetExpressionTree(std::string varname)
        { return GetExpressionTree(varname.c_str()); }

    static std::string GetRealVariable(const std::string &var);

protected:

    Parser* GetParser() { return parser; }

private:
    ExpressionList exprList;
    Parser *parser;

    static ParsingExprList *instance;
};

#endif
