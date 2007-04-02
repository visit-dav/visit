/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                          avtSymmPointExpression.h                         //
// ************************************************************************* //

#include <avtSymmPointExpression.h>

#include <stdio.h>

#include <Expression.h>
#include <ParsingExprList.h>


// ****************************************************************************
//  Method: avtSymmPointExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

avtSymmPointExpression::avtSymmPointExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSymmPointExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

avtSymmPointExpression::~avtSymmPointExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSymmPointExpression::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      the symmetry around a point.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
//  Modifications:
//
//    Hank Childs, Mon Jan  8 10:31:22 PST 2007
//    Do not assume that the expression created is a scalar.
//
// ****************************************************************************

void
avtSymmPointExpression::GetMacro(std::vector<std::string> &args, 
                                 std::string &ne, Expression::ExprType &type)
{
    char new_expr[1024];
    sprintf(new_expr, "%s - eval_point(%s, %s, %s)",
                 args[0].c_str(), args[0].c_str(), args[0].c_str(),
                 args[1].c_str());
    ne = new_expr;
    avtVarType et = DetermineVariableType(args[0]);
    type = ParsingExprList::GetExpressionTypeFromAVT(et);
    if (type == Expression::Unknown)
        type = Expression::ScalarMeshVar;
}


