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

// ************************************************************************* //
//                        avtSymmTransformExpression.h                       //
// ************************************************************************* //

#include <avtSymmTransformExpression.h>

#include <stdio.h>

#include <Expression.h>
#include <avtExpressionTypeConversions.h>


// ****************************************************************************
//  Method: avtSymmTransformExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtSymmTransformExpression::avtSymmTransformExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSymmTransformExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

avtSymmTransformExpression::~avtSymmTransformExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSymmTransformExpression::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      the Laplacian.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
//  Modifications:
//
//    Hank Childs, Mon Jan  8 10:31:22 PST 2007
//    Do not assume that the expression created is a scalar.
//
//    Brad Whitlock, Tue Jan 20 16:24:40 PST 2009
//    I changed a conversion function name.
//
// ****************************************************************************

void
avtSymmTransformExpression::GetMacro(std::vector<std::string> &args, 
                                   std::string &ne, Expression::ExprType &type)
{
    char new_expr[1024];
    sprintf(new_expr, "%s - eval_transform(%s, %s, %s)",
                 args[0].c_str(), args[0].c_str(), args[0].c_str(),
                 args[1].c_str());
    ne = new_expr;
    avtVarType et = DetermineVariableType(args[0]);
    type = avtVarType_To_ExprType(et);
    if (type == Expression::Unknown)
        type = Expression::ScalarMeshVar;
}


