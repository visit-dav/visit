#include <stdio.h>
#include <ViewerExpressionList.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <ExprNode.h>
#include <ExprParser.h>

ViewerExpressionList * ViewerExpressionList::instance = 0;

// ****************************************************************************
// Method: ViewerExpressionList::ViewerExpressionList
//
// Purpose: 
//   Constructor for the ViewerExpressionList class.
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 15:23:43 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 14:35:18 PST 2002
//   Moved the parser object to this class.
//
// ****************************************************************************

ViewerExpressionList::ViewerExpressionList() : SimpleObserver()
{
    // Make a new ExpressionList object.
    exprList = new ExpressionList;

    // Make this object observe it.
    exprList->Attach(this);

    // Add all of the default expressions and update so everyone is
    // updated.
    InitializeExpressionList();
    exprList->Notify();

    // Make a parser.
    parser = new ExprParser;
}

// ****************************************************************************
// Method: ViewerExpressionList::~ViewerExpressionList
//
// Purpose: 
//   Destructor for the ViewerExpressionList class.
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 15:23:46 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 14:36:49 PST 2002
//   Added code to delete the parser.
//
// ****************************************************************************

ViewerExpressionList::~ViewerExpressionList()
{
    delete exprList;
    exprList = NULL;

    delete parser;
}

// ****************************************************************************
// Method: ViewerExpressionList::Instance
//
// Purpose: 
//   Return a pointer to the sole instance of the ViewerExpressionList class.
//
// Programmer: Sean Ahern
// Creation:   Fri Jun 28 11:16:01 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
ViewerExpressionList*
ViewerExpressionList::Instance()
{
    // If the sole instance hasn't been instantiated, then instantiate it.
    if (instance == 0)
        instance = new ViewerExpressionList;

    return instance;
}

// ****************************************************************************
// Method: ViewerExpressionList::Update
//
// Purpose: 
//   This method is called when the expression list is updated from
//   the GUI.
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 15:23:48 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
ViewerExpressionList::Update(Subject *)
{
#if 0
    int     nexp = exprList->GetNumExpressions();

    // Print out the new expression list
    for (int i = 0; i < exprList->GetNumExpressions(); i++)
        cerr << "    " <<
            (*exprList)[i].GetName() << " = " <<
            (*exprList)[i].GetDefinition() << " " <<
            (*exprList)[i].GetType() << " " <<
            (*exprList)[i].GetHidden() << endl;
#endif
}

// ****************************************************************************
// Method: ViewerExpressionList::InitializeExpressionList
//
// Purpose: 
//   Adds the default expressions to the expression list.
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 15:23:54 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
ViewerExpressionList::InitializeExpressionList()
{
#if 0
    Expression *e = new Expression();
    e->SetName("neg_u");
    e->SetDefinition("-u");
    exprList->AddExpression(*e);

    e->SetName("sin_u");
    e->SetDefinition("sin(u)");
    e->SetHidden(true);
    exprList->AddExpression(*e);

    delete e;

    exprList->Notify();
#endif
}

// ****************************************************************************
// Method: ViewerExpressionList::VariableIsExpression
//
// Purpose: 
//   Tells whether or not a variable name is an expression.
//
// Arguments:
//   varname : The name of the variable that we want to check.
//
// Returns:    True if the variable name is an expression name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 18 14:13:42 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerExpressionList::VariableIsExpression(const char *varname) const
{
    bool found = false;

    // Check to see if the variable is an expression.
    std::string var(varname);
    for (int i = 0; i < exprList->GetNumExpressions(); i++)
    {
        if ((*exprList)[i].GetName() == var)
        {
            found = true;
            break;
        }
    }

    return found;
}

// ****************************************************************************
// Method: ViewerExpressionList::GetExpressionType
//
// Purpose: 
//   Returns the expression type for the expression variable.
//
// Arguments:
//   varname : The name of the expression variable.
//
// Returns:    The type of the expression or AVT_UNKNOWN_TYPE if the type
//             cannot be determined.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 18 14:45:25 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue May 20 14:07:03 PST 2003
//   I made it use the regenerated version of Expression.
//
// ****************************************************************************

avtVarType
ViewerExpressionList::GetExpressionType(const char *varname) const
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check to see if the variable is an expression.
    std::string var(varname);
    for (int i = 0; i < exprList->GetNumExpressions(); i++)
    {
        if ((*exprList)[i].GetName() == var)
        {
            switch ((*exprList)[i].GetType())
            {
            case Expression::ScalarMeshVar:
                retval = AVT_SCALAR_VAR;
                break;
            case Expression::VectorMeshVar:
                retval = AVT_VECTOR_VAR;
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
            break;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerExpressionList::GetExpressionTree
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
// Programmer: Brad Whitlock
// Creation:   Thu Jul 18 14:27:52 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ExprNode *
ViewerExpressionList::GetExpressionTree(const char *varname) const
{
    ExprNode *tree = 0;

    // Get the expression tree for the expression.
    std::string var(varname);
    for (int i = 0; i < exprList->GetNumExpressions(); i++)
    {
        if ((*exprList)[i].GetName() == var)
        {
            std::string exprDef = (*exprList)[i].GetDefinition();
            tree = parser->Parse(exprDef);
            break;
        }
    }

    return tree;
}

