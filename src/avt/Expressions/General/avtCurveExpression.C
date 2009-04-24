/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                          avtCurveExpression.C                             //
// ************************************************************************* //
#include <cstring>

#include <avtCurveExpression.h>

#include <snprintf.h>
#include <avtExprNode.h>
#include <DebugStream.h>
#include <ExpressionException.h>


static const char* yFuncs[] = {
"sin",
"asin",
"cos",
"acos",
"tan",
"atan",
"abs",
"ln",
"exp",
"log10",
"sqr",
"sqrt"
};

static int nFuncs = 12;

static const char* xFuncs[] = {
"sinx",
"asinx",
"cosx",
"acosx",
"tanx",
"atanx",
"absx",
"lnx",
"expx",
"log10x",
"sqrx",
"sqrtx"
};


// ****************************************************************************
//  Method: avtCurveExpression constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 5, 2009
//
// ****************************************************************************

avtCurveExpression::avtCurveExpression()
{
    xvar = -1;
}


// ****************************************************************************
//  Method: avtCurveExpression destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 5, 2009
//
// ****************************************************************************

avtCurveExpression::~avtCurveExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDivergenceExpression::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to curl.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 5, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveExpression::GetMacro(std::vector<std::string> &args, std::string &ne, 
                        Expression::ExprType &type)
{
    char new_expr[2048];
    if (-1 == xvar)
    {
        SNPRINTF(new_expr,2048, "%s(%s)", function.c_str(), args[0].c_str());    
    }
    else
    {
        SNPRINTF(new_expr, 2048, "curve_domain(%s, %s(coord(%s)[0]))", 
                 args[0].c_str(), yFuncs[xvar], args[0].c_str());    
    }
    type = Expression::CurveMeshVar;
    ne = new_expr;
}


// ****************************************************************************
//  Method: avtCurveExpression::IsYFunc
//
//  Purpose:
//    Determines if the passed function name is a supported y-coordinate
//    function.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 9, 2009
//
//  Modifications:
//
// ****************************************************************************

const int
avtCurveExpression::IsYFunc(const char *func) const
{
    for (int i = 0; i < nFuncs; ++i)
       if (strcmp(func, yFuncs[i]) == 0)
           return i;
    return -1;
}

// ****************************************************************************
//  Method: avtCurveExpression::IsXFunc
//
//  Purpose:
//    Determines if the passed function name is a supported x-coordinate
//    function.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 9, 2009
//
//  Modifications:
//
// ****************************************************************************

const int
avtCurveExpression::IsXFunc(const char *func) const
{
    for (int i = 0; i < nFuncs; ++i)
       if (strcmp(func, xFuncs[i]) == 0)
           return i;
    return -1;
}

// ****************************************************************************
//  Method: avtCurveExpression::ValidFunctionName
//
//  Purpose:
//    Determines if the passed function name is a supported x-coordinate
//    or y-coordinate function.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 9, 2009
//
//  Modifications:
//
// ****************************************************************************

const bool
avtCurveExpression::ValidFunctionName(const char *func)
{
    int index = IsYFunc(func);
    if (-1  == index)
    {
        index = IsXFunc(func);
        if (-1 != index)
            xvar = index;
    }
    return index != -1;    
}


// ****************************************************************************
//  Method: avtCurveExpression::ProcessArguments
//
//  Purpose:
//    Walks through the arguments and creates filters.
//    Checks to ensure the first argument is the needed function name.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 9, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveExpression::ProcessArguments(ArgsExpr *args, 
                                     ExprPipelineState *state)
{
    if (args == NULL)
    {
        debug4 << "Warning: null arguments given to expression." << endl;
        return;
    }

    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                  "avtCurveExpression: No arguments given.");
    }
    // First arg should be a function name.
    ArgExpr *first_arg = (*arguments)[0];
    ExprParseTreeNode *first_tree = first_arg->GetExpr();
    string arg_type = first_tree->GetTypeName();
    if (arg_type != "StringConst")
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                  "avtCurveExpression: Invalid first argument."
                  "Expected a function name.");
    }

    function = dynamic_cast<StringConstExpr*>(first_tree)->GetValue(); 
    if (!ValidFunctionName(function.c_str()))
    {
        EXCEPTION2(ExpressionException, function.c_str(), 
                  "avtCurveExpression: Unsupported curve function.");
    }

    for (int i = 1; i < nargs; ++i)
    {
        expression_arguments.push_back((*arguments)[i]->GetText());
        ExprParseTreeNode *n = (*arguments)[i]->GetExpr();
        avtExprNode *expr_node = dynamic_cast<avtExprNode*>
            ((*arguments)[i]->GetExpr());
        if (expr_node == NULL)
        {
            // Probably a list or some other construct that doesn't need
            // to create filters.
            continue;
        }
        expr_node->CreateFilters(state);
    }
}




