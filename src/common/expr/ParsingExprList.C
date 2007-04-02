/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <stdio.h>
#include <ParsingExprList.h>
#include <ExpressionList.h>
#include <Expression.h>
#include <ExprNode.h>
#include <ExprNodeFactory.h>
#include <ExprParser.h>
#include <string>
#include <set>
#include <vector>
#include <ImproperUseException.h>
#include <RecursiveExpressionException.h>
using std::set;
using std::string;
using std::vector;

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
//      Hank Childs, Tue Jul 19 13:39:12 PDT 2005
//      Added array expression.
//
//      Kathleen Bonnell, Wed Aug  2 17:20:11 PDT 2006 
//      Added curve expression.
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
    case Expression::ArrayMeshVar:
        retval = AVT_ARRAY_VAR;
        break;
    case Expression::CurveMeshVar:
        retval = AVT_CURVE;
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
// Method: ParsingExprList::GetExpressionTypeFromAVT
//
// Purpose: 
//   Returns the expression type for the AVT variable type.
//
// Arguments:
//   type : The AVT variable type
//
// Returns:    The expression type or UNKNOWN if the type cannot be determined.
//
// Note:       
//
// Programmer: Hank Childs
// Creation:   January 8, 2007
//
// ****************************************************************************

Expression::ExprType
ParsingExprList::GetExpressionTypeFromAVT(avtVarType type)
{
    Expression::ExprType retval = Expression::Unknown;

    // Check to see if the variable is an expression.
    switch (type)
    {
      case AVT_SCALAR_VAR:
        retval = Expression::ScalarMeshVar;
        break;
      case AVT_VECTOR_VAR:
        retval = Expression::VectorMeshVar;
        break;
      case AVT_TENSOR_VAR:
        retval =  Expression::TensorMeshVar;
        break;
      case AVT_SYMMETRIC_TENSOR_VAR:
        retval =  Expression::SymmetricTensorMeshVar;
        break;
      case AVT_ARRAY_VAR:
        retval =  Expression::ArrayMeshVar;
        break;
      case AVT_CURVE:
        retval =  Expression::CurveMeshVar;
        break;
      case AVT_MESH:
        retval =  Expression::Mesh;
        break;
      case AVT_MATERIAL:
        retval =  Expression::Material;
        break;
      case AVT_MATSPECIES:
        retval =  Expression::Species;
        break;
      case AVT_UNKNOWN_TYPE:
      default:
        retval =  Expression::Unknown;
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


// ****************************************************************************
//  Function:  ParsingExprList::GetRealVariableHelper
//
//  Purpose:
//    Determine the first true database variable for any given
//    expression.  Throw an ImproperUseException if there were
//    no real variables in an expression, throw a
//    RecursiveExpressionException if the expression is recursive,
//    and return the empty string if an unknown error occurred
//    during parsing.
//
//  Arguments:
//    var        the original variable name
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  9, 2005
//
//  Modifications:
//    Brad Whitlock, Thu Feb 24 16:07:13 PST 2005
//    I made it iterate over varLeaves as a set instead of first creating
//    a vector of strings because the vector constructor was not taking
//    the set container's iterators successfully on MSVC 6.0.
//
//    Jeremy Meredith, Mon Jun 13 15:51:50 PDT 2005
//    Delete the parse tree when we're done with it.  This fixes leaks.
//
// ****************************************************************************

static string
GetRealVariableHelper(const string &var, set<string> expandedVars)
{
    // Check for recursion in expanded vars
    if (expandedVars.count(var))
    {
        EXCEPTION1(RecursiveExpressionException, var);
    }
    expandedVars.insert(var);

    // If this variable is not an expression, then it is real
    Expression *expr = ParsingExprList::GetExpression(var);
    if (!expr)
    {
        // Found the real variable
        return var;
    }

    // Otherwise, descend into it; get the expression tree
    ExprNode *tree = ParsingExprList::GetExpressionTree(expr);
    if (!tree)
    {
        // We won't normally get here because error
        // conditions will usually throw exceptions.
        // Otherwise, every expression should have
        // a tree.
        return "";
    }

    // Get the leaves for this expression
    const set<string> &varLeaves = tree->GetVarLeaves();
    if (varLeaves.empty())
    {
        delete tree;
        return "";
    }

    // For each leaf, look for a real variable
#if defined(_WIN32) && defined(USING_MSVC6)
    // Don't use const iterator on win32 MSVC 6.
    for (std::set<std::string>::iterator it = varLeaves.begin();
         it != varLeaves.end(); ++it)
#else
    for (std::set<std::string>::const_iterator it = varLeaves.begin();
         it != varLeaves.end(); ++it)
#endif
    {
        string realvar = GetRealVariableHelper(*it, expandedVars);

        // If we found a real variable, return it!
        if (!realvar.empty())
        {
            delete tree;
            return realvar;
        }
    }

    // Didn't find any real variables
    delete tree;
    return "";
}

// ****************************************************************************
//  Method:  ParsingExprList::GetRealVariable
//
//  Purpose:
//    Determine the first true database variable for any given
//    expression.  Throw an ImproperUseException if there were
//    no real variables in an expression, throw a
//    RecursiveExpressionException if the expression is recursive,
//    and return the empty string if an unknown error occurred
//    during parsing.
//
//    Most of the work is done inside the recursion helper function.
//    This merely makes sure we actually have a real variable before
//    returning to the original caller, as well as setting up an
//    empty expanded variables set for the helper function.
//
//  Arguments:
//    var        the original variable name
//
//  Programmer:  Jeremy Meredith
//  Creation:    January  9, 2005
//
// ****************************************************************************
string
ParsingExprList::GetRealVariable(const string &var)
{
    set<string> emptySet;
    string realvar = GetRealVariableHelper(var, emptySet);

    if (realvar.empty())
    {
        EXCEPTION1(ImproperUseException,
                   "After parsing, expression has no real variables.");
    }

    return realvar;
}
