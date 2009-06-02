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
#include <avtExprNode.h>

#include <avtConditionalExpression.h>
#include <avtLogicalAndExpression.h>
#include <avtLogicalNegationExpression.h>
#include <avtLogicalOrExpression.h>
#include <avtTestEqualToExpression.h>
#include <avtTestGreaterThanExpression.h>
#include <avtTestGreaterThanOrEqualToExpression.h>
#include <avtTestLessThanExpression.h>
#include <avtTestLessThanOrEqualToExpression.h>
#include <avtTestNotEqualToExpression.h>

// ****************************************************************************
// Method: avtFunctionExpr::CreateConditionalFilters
//
// Purpose: 
//   Creates conditional and logic expression filters.
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
avtFunctionExpr::CreateConditionalFilters(const string &functionName) const
{
    avtExpressionFilter *f = 0;

    if (functionName == "if")
        f = new avtConditionalExpression();
    else if (functionName == "and")
        f = new avtLogicalAndExpression();
    else if (functionName == "or")
        f = new avtLogicalOrExpression();
    else if (functionName == "not")
        f = new avtLogicalNegationExpression();
    else if (functionName == "le" || functionName == "lte")
        f = new avtTestLessThanOrEqualToExpression();
    else if (functionName == "ge" || functionName == "gte")
        f = new avtTestGreaterThanOrEqualToExpression();
    else if (functionName == "lt")
        f = new avtTestLessThanExpression();
    else if (functionName == "gt")
        f = new avtTestGreaterThanExpression();
    else if (functionName == "eq" || functionName == "equal" || 
             functionName == "equals")
        f = new avtTestEqualToExpression();
    else if (functionName == "ne" || functionName == "neq" ||
             functionName == "notequal" || functionName == "notequals")
        f = new avtTestNotEqualToExpression();

    return f;
}
