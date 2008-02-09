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
//                               avtCurlExpression.h                             //
// ************************************************************************* //

#include <avtCurlExpression.h>

#include <snprintf.h>
#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtCurlExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtCurlExpression::avtCurlExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurlExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

avtCurlExpression::~avtCurlExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDivergenceExpression::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to curl.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2004
//
//  Modifications:
//
//    Hank Childs, Mon Jun  6 11:25:51 PDT 2005
//    Add error checking for 2D.
//
//    Hank Childs, Fri Aug 12 13:48:51 PDT 2005
//    Add support for 2D curl (which is a scalar).
//
//    Hank Childs, Fri Aug 19 09:20:24 PDT 2005
//    If we are creating a scalar, then make sure the expression type is a 
//    scalar as well.
//
//    Cyrus Harrison, Sat Aug 11 18:15:50 PDT 2007
//    Add second argument for gradient algorithm selection
//
// ****************************************************************************

void
avtCurlExpression::GetMacro(std::vector<std::string> &args, std::string &ne, 
                        Expression::ExprType &type)
{
    bool do3D = true;
    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        if (atts.GetTopologicalDimension() < 3)
        {
            do3D = false;
        }
    }

    int nargs = args.size();

    char new_expr[2048];
    if (do3D)
    {
        if(nargs == 1)
        {
            SNPRINTF(new_expr,2048, 
                    "{gradient(%s[2])[1]-gradient(%s[1])[2],"
                    "gradient(%s[0])[2]-gradient(%s[2])[0],"
                    "gradient(%s[1])[0]-gradient(%s[0])[1]}",
                    args[0].c_str(), args[0].c_str(), args[0].c_str(),
                    args[0].c_str(), args[0].c_str(), args[0].c_str());
        }
        else if(nargs > 1)
        {
            SNPRINTF(new_expr,2048, 
                    "{gradient(%s[2],%s)[1]-gradient(%s[1],%s)[2],"
                    "gradient(%s[0],%s)[2]-gradient(%s[2],%s)[0],"
                    "gradient(%s[1],%s)[0]-gradient(%s[0],%s)[1]}",
                    args[0].c_str(), args[1].c_str(),
                    args[0].c_str(), args[1].c_str(),
                    args[0].c_str(), args[1].c_str(),
                    args[0].c_str(), args[1].c_str(),
                    args[0].c_str(), args[1].c_str(),
                    args[0].c_str(), args[1].c_str());
        }
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName, " invalid curl syntax. "
                        "Expected arguments: "
                        "vector_var, gradient_algorithm\n"
                        "[gradient_algorithm is optional]");
        }

        type = Expression::VectorMeshVar;
    }
    else
    {
        if(nargs == 1)
        {
            SNPRINTF(new_expr,2048,
                    "gradient(%s[1])[0]-gradient(%s[0])[1]",
                    args[0].c_str(), args[0].c_str());
        }
        else if(nargs > 1)
        {
            SNPRINTF(new_expr,2048,
                    "gradient(%s[1],%s)[0]-gradient(%s[0],%s)[1]",
                    args[0].c_str(), args[1].c_str(), 
                    args[0].c_str(), args[1].c_str());
        }
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName, " invalid curl syntax. "
                        "Expected arguments: "
                        "vector_var, gradient_algorithm\n"
                        "[gradient_algorithm is optional]");
        }

        type = Expression::ScalarMeshVar;
    }
    ne = new_expr;
}


// ****************************************************************************
//  Method: avtDivergenceExpression::GetVariableDimension
//
//  Purpose:
//      Gets the variable dimension -- scalar for 2D datasets, vector for 3D.
//
//  Programmer: Hank Childs
//  Creation:   August 19, 2005
//
// ****************************************************************************

int
avtCurlExpression::GetVariableDimension(void)
{
    bool do3D = true;
    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        if (atts.GetTopologicalDimension() < 3)
        {
            do3D = false;
        }
    }

    return (do3D ? 3 : 1);
}


