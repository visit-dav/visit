#ifndef PARSING_EXPR_LIST_H
#define PARSING_EXPR_LIST_H
#include <expr_exports.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <SimpleObserver.h>
#include <VisItParser.h>
#include <string> 
#include <avtTypes.h>

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
// ****************************************************************************

class EXPR_API ParsingExprList : public SimpleObserver
{
public:
    ParsingExprList();
    ParsingExprList(Parser *p);
    ~ParsingExprList();

    static ParsingExprList *Instance();
    static avtVarType GetAVTType(Expression::ExprType);
    static Expression* GetExpression(const char *varname);
    static Expression* GetExpression(std::string varname)
        { return GetExpression(varname.c_str()); }

    ExpressionList *GetList() {return &exprList;}

    virtual void Update(Subject *);

    static ExprNode *GetExpressionTree(Expression*);
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
