#ifndef VIEWER_EXPRESSION_LIST_H
#define VIEWER_EXPRESSION_LIST_H
#include <viewer_exports.h>
#include <SimpleObserver.h>
#include <avtTypes.h>

// Forward declarations.
class ExpressionList;
class ExprNode;
class ExprParser;

// ****************************************************************************
// Class: ViewerExpressionList
//
// Purpose:
//   This class contains the list of expressions that are seen in the GUI.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 14:17:12 PDT 2001
//
// Modifications:
//    Sean Ahern, Fri Jun 28 11:13:49 PDT 2002
//    Made this be a singleton, accesible from anywhere.
// 
//    Brad Whitlock, Thu Jul 18 14:09:32 PST 2002
//    I added a few convenience methods.
//
// ****************************************************************************

class VIEWER_API ViewerExpressionList : public SimpleObserver
{
public:
    ViewerExpressionList();
    ~ViewerExpressionList();

    static ViewerExpressionList *Instance();

    ExpressionList *GetList() {return exprList;};

    virtual void Update(Subject *);

    bool VariableIsExpression(const char *varname) const;
    ExprNode *GetExpressionTree(const char *varname) const;
    avtVarType GetExpressionType(const char *varname) const;
protected:
    void InitializeExpressionList();
private:
    ExpressionList *exprList;
    ExprParser     *parser;

    static ViewerExpressionList *instance;
};

#endif
