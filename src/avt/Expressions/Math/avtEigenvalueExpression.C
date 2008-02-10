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
//                            avtEigenvalueExpression.C                          //
// ************************************************************************* //

#include <avtEigenvalueExpression.h>

#include <vtkDataArray.h>
#include <vtkMath.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtEigenvalueExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEigenvalueExpression::avtEigenvalueExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEigenvalueExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtEigenvalueExpression::~avtEigenvalueExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEigenvalueExpression::DoOperation
//
//  Purpose:
//      Calculates the eigenvalues of a matrix.
//
//  Arguments:
//      in            The input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
// ****************************************************************************
 
void
avtEigenvalueExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                 int ncomponents, int ntuples)
{
    if (ncomponents == 9)
    {
        for (int i = 0 ; i < ntuples ; i++)
        {
            double *vals = in->GetTuple9(i);
            double *input[3];
            double row1[3];
            double row2[3];
            double row3[3];
            input[0] = row1;
            input[1] = row2;
            input[2] = row3;
            input[0][0] = vals[0];
            input[0][1] = vals[1];
            input[0][2] = vals[2];
            input[1][0] = vals[3];
            input[1][1] = vals[4];
            input[1][2] = vals[5];
            input[2][0] = vals[6];
            input[2][1] = vals[7];
            input[2][2] = vals[8];
            double *eigenvecs[3];
            double outrow1[3];
            double outrow2[3];
            double outrow3[3];
            eigenvecs[0] = outrow1;
            eigenvecs[1] = outrow2;
            eigenvecs[2] = outrow3;
            double eigenvals[3];
            vtkMath::Jacobi(input, eigenvals, eigenvecs);
            out->SetTuple(i, eigenvals);
        }
    }
    else
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Cannot determine tensor type");
    }
}


