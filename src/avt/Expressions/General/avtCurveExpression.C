// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtCurveExpression.C                             //
// ************************************************************************* //
#include <cstring>

#include <avtCurveExpression.h>

#include <snprintf.h>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <avtExprNode.h>

#include <string>
#include <vector>

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

int
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

int
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

bool
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
    size_t nargs = arguments->size();
    if (nargs == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                  "avtCurveExpression: No arguments given.");
    }
    // First arg should be a function name.
    ArgExpr *first_arg = (*arguments)[0];
    ExprParseTreeNode *first_tree = first_arg->GetExpr();
    std::string arg_type = first_tree->GetTypeName();
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
