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
//                            avtMatErrorExpression.h                          //
// ************************************************************************* //

#include <avtMatErrorExpression.h>

#include <stdio.h>


// ****************************************************************************
//  Method: avtMatErrorExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

avtMatErrorExpression::avtMatErrorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMatErrorExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

avtMatErrorExpression::~avtMatErrorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtMatErrorExpression::GetMacro
//
//  Purpose:
//      Applies the macro to create a new expression corresponding to
//      material error.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2005
//
// ****************************************************************************

void
avtMatErrorExpression::GetMacro(std::vector<std::string> &args, std::string &ne,
                            Expression::ExprType &type)
{
    const char *matname  = args[0].c_str();
    const char *mats     = args[1].c_str();

    //
    // So this is a bit unfortunate.  GetMacro can be called before the input
    // is set and we can determine what the topological dimension is.  No
    // matter -- we really just want to get it right when we do the actual
    // execute (when we will have a valid input).  So, if the input is NULL,
    // claim we want "volume".  If the input is not NULL, choose between
    // "volume" and "area".
    //
    // A similar problem happens with the meshname.
    //
    const char *volume = "volume";
    const char *meshname = "";
    if (*(GetInput()) != NULL)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        volume   = (atts.GetTopologicalDimension() == 3 ? "volume" : "area");
        meshname = atts.GetMeshname().c_str();
    }
    char new_expr[1024];
    sprintf(new_expr, "relative_difference(matvf(%s, %s), "
                                      "mirvf(%s, zoneid(%s), %s(%s), %s))",
                           matname, mats, matname, meshname, volume,
                           meshname, mats);
    ne = new_expr;
    type = Expression::ScalarMeshVar;
}


