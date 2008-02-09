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
//                          avtVectorCrossProductExpression.C                    //
// ************************************************************************* //

#include <avtVectorCrossProductExpression.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtVectorCrossProductExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorCrossProductExpression::avtVectorCrossProductExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorCrossProductExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVectorCrossProductExpression::~avtVectorCrossProductExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorCrossProductExpression::DoOperation
//
//  Purpose:
//      Takes the cross product of the two inputs
//
//  Arguments:
//      in1           The first input vector
//      in2           The second input vector
//      out           The output vector.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)  Must be 3 for this operation.
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Notes: a x b = (a2 b3 - a3 b2)i + (a3 b1 - a1 b3)j + (a1 b2 - a2 b1)k
//
//  Programmer: Sean Ahern
//  Creation:   Wed Jun 11 13:32:55 PDT 2003
//
//  Modifications:
//
// ****************************************************************************
void
avtVectorCrossProductExpression::DoOperation(vtkDataArray *in1,
                                vtkDataArray *in2, vtkDataArray *out,
                                int ncomponents, int ntuples)
{
    if (ncomponents != 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "you cannot take the cross product of data which are not 3-component vectors.");
    }

    for (int i = 0 ; i < ntuples ; i++)
    {
        float a1 = in1->GetComponent(i, 0);
        float a2 = in1->GetComponent(i, 1);
        float a3 = in1->GetComponent(i, 2);
        float b1 = in2->GetComponent(i, 0);
        float b2 = in2->GetComponent(i, 1);
        float b3 = in2->GetComponent(i, 2);

        out->SetComponent(i, 0, a2*b3 - a3*b2);
        out->SetComponent(i, 1, a3*b1 - a1*b3);
        out->SetComponent(i, 2, a1*b2 - a2*b1);
    }
}
