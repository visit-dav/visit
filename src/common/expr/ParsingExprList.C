#include <stdio.h>
#include <ParsingExprList.h>
#include <ExpressionList.h>
#include <Expression.h>
#include <ExprNode.h>
#include <ExprNodeFactory.h>
#include <ExprParser.h>

ParsingExprList * ParsingExprList::instance = 0;

// ****************************************************************************
// Method: ParsingExprList::ParsingExprList
//
// Purpose: 
//   Default constructor for the ParsingExprList class.
//
// Programmer: Sean Ahern
// Creation:   Wed Feb  5 13:53:33 PST 2003
//
// Modifications:
//    Jeremy Meredith, Wed Nov 24 12:27:15 PST 2004
//    Refactored and changed parser base classes around.
//
// ****************************************************************************
ParsingExprList::ParsingExprList() : SimpleObserver()
{
    // Make this object observe the expression list.
    exprList.Attach(this);

    // Make a default parser.
    parser = new ExprParser(new ExprNodeFactory());

    // Set the instance variable.
    instance = this;
}

// ****************************************************************************
// Method: ParsingExprList::ParsingExprList
//
// Purpose: 
//   Constructor for the ParsingExprList class that includes a parser
//   interface.
//
// Programmer: Sean Ahern
// Creation:   Wed Feb  5 13:53:26 PST 2003
//
// Modifications:
//    Jeremy Meredith, Wed Nov 24 12:27:15 PST 2004
//    Refactored and changed parser base classes around.
//
// ****************************************************************************
ParsingExprList::ParsingExprList(Parser *p) : SimpleObserver(),
    parser(p)
{
    // Make this object observe the expression list.
    exprList.Attach(this);

    // Set the instance variable.
    instance = this;
}

// ****************************************************************************
// Method: ParsingExprList::~ParsingExprList
//
// Purpose: 
//   Destructor for the ParsingExprList class.
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 15:23:46 PDT 2001
//
// Modifications:
//
// ****************************************************************************
ParsingExprList::~ParsingExprList()
{
    delete parser;
}

// ****************************************************************************
// Method: ParsingExprList::Instance
//
// Purpose: 
//   Return a pointer to the sole instance of the ParsingExprList class.
//
// Programmer: Sean Ahern
// Creation:   Fri Jun 28 11:16:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
ParsingExprList*
ParsingExprList::Instance()
{
    // If the sole instance hasn't been instantiated, then instantiate it.
    if (instance == 0)
        instance = new ParsingExprList;

    return instance;
}

// ****************************************************************************
// Method: ParsingExprList::Update
//
// Purpose: 
//   This method is called when the expression list is updated from
//   somewhere else.
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 15:23:48 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
ParsingExprList::Update(Subject *)
{
}

// ****************************************************************************
// Method: ParsingExprList::GetAVTType
//
// Purpose: 
//   Returns the avtVarType for the expression type.
//
// Arguments:
//   type : The expression type
//
// Returns:    The avtVarType of the expression or AVT_UNKNOWN_TYPE if the type
//             cannot be determined.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 18 14:45:25 PST 2002
//
// Modifications:
//      Sean Ahern, Wed Dec 11 16:29:52 PST 2002
//      Changed the interface so that it works with ExprTypes.
//
//      Sean Ahern, Mon Mar 17 23:01:02 America/Los_Angeles 2003
//      Changed the expression type names.
//   
// ****************************************************************************
avtVarType
ParsingExprList::GetAVTType(const Expression::ExprType type)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check to see if the variable is an expression.
    switch (type)
    {
    case Expression::ScalarMeshVar:
        retval = AVT_SCALAR_VAR;
        break;
    case Expression::VectorMeshVar:
        retval = AVT_VECTOR_VAR;
        break;
    case Expression::TensorMeshVar:
        retval = AVT_TENSOR_VAR;
        break;
    case Expression::SymmetricTensorMeshVar:
        retval = AVT_SYMMETRIC_TENSOR_VAR;
        break;
    case Expression::Mesh:
        retval = AVT_MESH;
        break;
    case Expression::Material:
        retval = AVT_MATERIAL;
        break;
    case Expression::Species:
        retval = AVT_MATSPECIES;
        break;
    case Expression::Unknown:
    default:
        retval = AVT_UNKNOWN_TYPE;
    }

    return retval;
}

// ****************************************************************************
// Method: ParsingExprList::GetExpression
//
// Purpose: 
//   Returns the Expression for the given variable name.
//
// Arguments:
//   varname : The name of the variable.
//
// Returns: A pointer to the Expression corresponding to the varname, if it
//          exists.  NULL if not.
//
// Note:       
//
// Programmer: Sean Ahern
// Creation:   Wed Dec 11 17:05:14 PST 2002
//
// Modifications:
//   
// ****************************************************************************
Expression*
ParsingExprList::GetExpression(const char *varname)
{
    return Instance()->GetList()->operator[](varname);
}

// ****************************************************************************
// Method: ParsingExprList::GetExpressionTree
//
// Purpose: 
//   Returns the expression tree for the specified expression or NULL if
//   the expression does not exist.
//
// Arguments:
//   varname : The name of the expression for which to get the tree.
//
// Returns:    The expression tree for the specified expression or NULL if
//   the expression does not exist.
//
// Note:       Moved from ViewerEngineManager.C
//
// Programmer: Sean Ahern
// Creation:   Wed Feb  5 13:52:35 PST 2003
//
// Modifications:
//    Jeremy Meredith, Wed Nov 24 12:27:15 PST 2004
//    Refactored and changed parse tree classes and return types around.
//   
// ****************************************************************************
ExprNode *
ParsingExprList::GetExpressionTree(const char *varname)
{
    ExprNode *tree = 0;

    // Get the expression tree for the expression.
    //Expression *exp = const_cast<ExpressionList&>(exprList)[varname];
    Expression *exp = GetExpression(varname);
    if (exp != NULL)
    {
        ParseTreeNode *t=Instance()->GetParser()->Parse(exp->GetDefinition());
        tree = (ExprNode*)t;
    }
    else
        tree = NULL;

    return tree;
}

// ****************************************************************************
// Method: ParsingExprList::GetExpressionTree
//
// Purpose: 
//   Returns the expression tree for the specified expression or NULL if
//   the expression does not exist.
//
// Arguments:
//   expr : The expression for which to get the tree.
//
// Returns:    The expression tree for the specified expression or NULL if
//   the expression does not exist.
//
// Programmer: Jeremy Meredith
// Creation:   August 14, 2003
//
// Modifications:
//    Jeremy Meredith, Wed Nov 24 12:27:15 PST 2004
//    Refactored and changed parse tree classes and return types around.
//   
// ****************************************************************************
ExprNode *
ParsingExprList::GetExpressionTree(Expression *expr)
{
    if (!expr)
        return NULL;

    ParseTreeNode *t = Instance()->GetParser()->Parse(expr->GetDefinition());
    return (ExprNode*)t;
}
