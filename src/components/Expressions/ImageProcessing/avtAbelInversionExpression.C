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
//                          avtAbelInversionExpression.C                     //
// ************************************************************************* //

#include <avtAbelInversionExpression.h>

#include <math.h>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>


// ****************************************************************************
//  Method: avtAbelInversionExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2006
//
// ****************************************************************************

avtAbelInversionExpression::avtAbelInversionExpression()
{
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtAbelInversionExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2006
//
// ****************************************************************************

avtAbelInversionExpression::~avtAbelInversionExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtAbelInversionExpression::PreExecute
//
//  Purpose:
//      Initialize the haveIssuedWarning flag.
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2006
//
// ****************************************************************************

void
avtAbelInversionExpression::PreExecute(void)
{
    avtUnaryMathExpression::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtAbelInversionExpression::DoOperation
//
//  Purpose:
//      Calculates the mean.
//
//  Arguments:
//      in1           The first input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   October 6, 2006
//
// ****************************************************************************

void
avtAbelInversionExpression::DoOperation(vtkDataArray *in1, vtkDataArray *out, 
                           int ncomponents,int ntuples)
{
    if (cur_mesh->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The inverse abel transform expression "
                                      "only operates on rectilinear grids.");
            haveIssuedWarning = true;
        }
        return;
    }

    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) cur_mesh;
    int dims[3];
    rgrid->GetDimensions(dims);

    if (dims[2] > 1)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The inverse abel transform expression "
                                      "only operates on 2D grids.");
            haveIssuedWarning = true;
        }
    }

    bool nodeCentered = true;
    if (dims[0]*dims[1] != ntuples)
        nodeCentered = false;

    if (!nodeCentered)
    {
        dims[0] -= 1;
        dims[1] -= 1;
    }

    vtkDataArray *yCoord = rgrid->GetYCoordinates();
    for (int i = 0 ; i < dims[0] ; i++)
    {
        for (int j = 0 ; j < dims[1] ; j++)
        {
            double integral = 0;
            for (int k = j ; k < dims[1]-1 ; k++)
            {
                double grad;
                
                int idx = k*dims[0]+i;
                int idx_p1 = (k+1)*dims[0]+i;
                int idx_m1 = (k-1)*dims[0]+i;
                if (k == dims[1]-1)
                {
                    grad = in1->GetTuple1(idx) - in1->GetTuple1(idx_m1);
                    grad /= yCoord->GetTuple1(k) - yCoord->GetTuple1(k-1);
                }
                else if (k == 0)
                {
                    grad = in1->GetTuple1(idx_p1) - in1->GetTuple1(idx);
                    grad /= yCoord->GetTuple1(k+1) - yCoord->GetTuple1(k);
                }
                else 
                {
                    grad = in1->GetTuple1(idx_p1) - in1->GetTuple1(idx_m1);
                    grad /= yCoord->GetTuple1(k+1) - yCoord->GetTuple1(k-1);
                }

                double y1 = yCoord->GetTuple1(k);
                double y2 = yCoord->GetTuple1(k+1);
                double r = yCoord->GetTuple1(j);
                double yMid = (y1+y2)/2.;
                integral += grad / sqrt(yMid*yMid - r*r) * (y2-y1);
            }
            integral *= -1./M_PI;
            int idx = j*dims[0]+i;
            out->SetTuple1(idx, integral);
        }
    }
}


