#ifndef PARSING_EXPR_LIST_H
#define PARSING_EXPR_LIST_H
#include <parser_exports.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <SimpleObserver.h>
#include <string> 
#include <avtTypes.h>

// Forward declarations.
class ExprNode;
class ParserInterface;

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
//
// ****************************************************************************

class PARSER_API ParsingExprList : public SimpleObserver
{
public:
    ParsingExprList();
    ParsingExprList(ParserInterface *p);
    ~ParsingExprList();

    static ParsingExprList *Instance();
    static avtVarType GetAVTType(Expression::ExprType);
    static Expression* GetExpression(const char *varname);
    static Expression* GetExpression(std::string varname)
        { return GetExpression(varname.c_str()); }

    ExpressionList *GetList() {return &exprList;}

    virtual void Update(Subject *);

    static ExprNode *GetExpressionTree(const char *varname);
    static ExprNode *GetExpressionTree(std::string varname)
        { return GetExpressionTree(varname.c_str()); }

protected:

    ParserInterface* GetParser() { return parser; }

private:
    ExpressionList exprList;
    ParserInterface *parser;

    static ParsingExprList *instance;
};

#endif
