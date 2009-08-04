/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <avtExprNode.h>

#include <avtAbsValExpression.h>
#include <avtArccosExpression.h>
#include <avtArcsinExpression.h>
#include <avtArctan2Expression.h>
#include <avtArctanExpression.h>
#include <avtBase10LogExpression.h>
#include <avtCeilingExpression.h>
#include <avtCosExpression.h>
#include <avtCoshExpression.h>
#include <avtDegreeToRadianExpression.h>
#include <avtExpExpression.h>
#include <avtFloorExpression.h>
#include <avtModuloExpression.h>
#include <avtNaturalLogExpression.h>
#include <avtRadianToDegreeExpression.h>
#include <avtRandomExpression.h>
#include <avtRoundExpression.h>
#include <avtSinExpression.h>
#include <avtSinhExpression.h>
#include <avtSquareExpression.h>
#include <avtSquareRootExpression.h>
#include <avtTanExpression.h>
#include <avtTanhExpression.h>


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
// ****************************************************************************

avtExpressionFilter *
avtFunctionExpr::CreateMathFilters(const string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "sin")
        f = new avtSinExpression();
    else if (functionName == "sinh")
        f = new avtSinhExpression();
    else if (functionName == "cos")
        f = new avtCosExpression();
    else if (functionName == "cosh")
        f = new avtCoshExpression();
    else if (functionName == "tan")
        f = new avtTanExpression();
    else if (functionName == "tanh")
        f = new avtTanhExpression();
    else if (functionName == "atan")
        f = new avtArctanExpression();
    else if (functionName == "atan2")
        f = new avtArctan2Expression();
    else if (functionName == "asin")
        f = new avtArcsinExpression();
    else if (functionName == "acos")
        f = new avtArccosExpression();
    else if (functionName == "deg2rad")
        f = new avtDegreeToRadianExpression();
    else if (functionName == "rad2deg")
        f = new avtRadianToDegreeExpression();
    else if (functionName == "abs")
        f = new avtAbsValExpression();
    else if (functionName == "ln")
        f = new avtNaturalLogExpression();
    else if (functionName == "exp")
        f = new avtExpExpression();
    else if ((functionName == "log") || (functionName == "log10"))
        f = new avtBase10LogExpression();
    else if (functionName == "sqrt")
        f = new avtSquareRootExpression();
    else if ((functionName == "sq") || (functionName == "sqr"))
        f = new avtSquareExpression();
    else if (functionName == "mod" || functionName == "modulo")
        f = new avtModuloExpression();
    else if (functionName == "ceil")
        f = new avtCeilingExpression();
    else if (functionName == "floor")
        f = new avtFloorExpression();
    else if (functionName == "round")
        f = new avtRoundExpression();
    else if ((functionName == "random") || (functionName == "rand"))
        f = new avtRandomExpression();

    return f;
}
