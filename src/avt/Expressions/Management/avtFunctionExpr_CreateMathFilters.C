// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtExprNode.h>

#include <avtAbsValExpression.h>
#include <avtArccosExpression.h>
#include <avtArcsinExpression.h>
#include <avtArctan2Expression.h>
#include <avtArctanExpression.h>
#include <avtBase10LogExpression.h>
#include <avtBase10LogWithMinExpression.h>
#include <avtCeilingExpression.h>
#include <avtCosExpression.h>
#include <avtCoshExpression.h>
#include <avtDegreeToRadianExpression.h>
#include <avtBinaryDivideExpression.h>
#include <avtExpExpression.h>
#include <avtFloorExpression.h>
#include <avtModuloExpression.h>
#include <avtNaturalLogExpression.h>
#include <avtRadianToDegreeExpression.h>
#include <avtRandomExpression.h>
#include <avtRoundExpression.h>
#include <avtSinExpression.h>
#include <avtSinhExpression.h>
#include <avtSmartDivideExpression.h>
#include <avtSquareExpression.h>
#include <avtSquareRootExpression.h>
#include <avtTanExpression.h>
#include <avtTanhExpression.h>

#include <DebugStream.h>

#include <string>

// ****************************************************************************
// Method: avtFunctionExpr::CreateMathFilters
//
// Purpose: 
//   Creates math filters.
//
// Arguments:
//   functionName : The name of the expression filter to create.
//
// Returns:    An expression filter or 0 if one could not be created.
//
// Note:       
//
// Programmer: 
// Creation:   Thu May 21 08:55:58 PDT 2009
//
// Modifications:
//   
//   Hank Childs, Thu May 20 21:26:47 PDT 2010
//   Add log10withmin.  This is possible with if, gt, and log, but more
//   efficiently implemented in a single filter.
//
//   Eddie Rusu, Wed Sep 11 08:59:52 PDT 2019
//   Alphabetically ordered by functionName for easier reading.
//   Added "divide" filter.
//
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateMathFilters(const std::string &functionName) const
{
    debug5 << "Entering avtFunctionExpr::CreateMathFilters(const string&)" << std::endl;
    avtExpressionFilter *f = 0;

    if (functionName == "abs")
        f = new avtAbsValExpression();
    else if (functionName == "acos")
        f = new avtArccosExpression();
    else if (functionName == "asin")
        f = new avtArcsinExpression();
    else if (functionName == "atan")
        f = new avtArctanExpression();
    else if (functionName == "atan2")
        f = new avtArctan2Expression();
    else if (functionName == "ceil")
        f = new avtCeilingExpression();
    else if (functionName == "cos")
        f = new avtCosExpression();
    else if (functionName == "cosh")
        f = new avtCoshExpression();
    else if (functionName == "deg2rad")
        f = new avtDegreeToRadianExpression();
    else if (functionName == "divide")
        f = new avtSmartDivideExpression();
    else if (functionName == "exp")
        f = new avtExpExpression();
    else if (functionName == "floor")
        f = new avtFloorExpression();
    else if (functionName == "ln")
        f = new avtNaturalLogExpression();
    else if ((functionName == "log") || (functionName == "log10"))
        f = new avtBase10LogExpression();
    else if (functionName == "log10withmin") 
        f = new avtBase10LogWithMinExpression();
    else if (functionName == "mod" || functionName == "modulo")
        f = new avtModuloExpression();
    else if (functionName == "rad2deg")
        f = new avtRadianToDegreeExpression();
    else if ((functionName == "random") || (functionName == "rand"))
        f = new avtRandomExpression();
    else if (functionName == "round")
        f = new avtRoundExpression();
    else if (functionName == "sin")
        f = new avtSinExpression();
    else if (functionName == "sinh")
        f = new avtSinhExpression();
    else if ((functionName == "sq") || (functionName == "sqr"))
        f = new avtSquareExpression();
    else if (functionName == "sqrt")
        f = new avtSquareRootExpression();
    else if (functionName == "tan")
        f = new avtTanExpression();
    else if (functionName == "tanh")
        f = new avtTanhExpression();

    debug5 << "Exiting  avtFunctionExpr::CreateMathFilters(const string&)" << std::endl;
    return f;
}
