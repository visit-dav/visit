/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                       avtCurveDomainExpression.C                          //
// ************************************************************************* //

#include <avtCurveDomainExpression.h>

#include <math.h>
#include <float.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <vtkVisItUtility.h>


// ****************************************************************************
//  Method: avtCurveDomainExpression constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 5, 2009
//
// ****************************************************************************

avtCurveDomainExpression::avtCurveDomainExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurveDomainExpression destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 5, 2009
//
// ****************************************************************************

avtCurveDomainExpression::~avtCurveDomainExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurveDomainExpression::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      in_ds     The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     March 5, 2009
//
//  Modifications:
//    Eric Brugger, Mon Aug 20 11:29:34 PDT 2012
//    I modified the routine to delete points where the coordinates weren't
//    monotonically increasing.
//
// ****************************************************************************

vtkDataSet *
avtCurveDomainExpression::ExecuteData(vtkDataSet *in_ds, int index,
                                      std::string label)
{
    vtkDataArray *yval = in_ds->GetPointData()->GetArray(varnames[0]);
    vtkDataArray *xval = in_ds->GetPointData()->GetArray(varnames[1]);

    vtkIdType npts = xval->GetNumberOfTuples();
    int nptsValid = 1;
    double xmax = xval->GetTuple1(0);
    for (vtkIdType i = 1; i < npts; ++i)
    {
        if (xval->GetTuple1(i) > xmax)
        {
            nptsValid++;
            xmax = xval->GetTuple1(i);
        }
    }
    vtkRectilinearGrid *rv = vtkVisItUtility::Create1DRGrid(nptsValid, xval->GetDataType());

    vtkDataArray *newX = rv->GetXCoordinates();
    vtkDataArray *newY = yval->NewInstance();
    newY->SetNumberOfTuples(nptsValid);
    newY->SetName(GetOutputVariableName());
    rv->GetPointData()->SetScalars(newY);
    newY->Delete();

    newX->SetTuple1(0, xval->GetTuple1(0));
    newY->SetTuple1(0, yval->GetTuple1(0));
    nptsValid = 1;
    xmax = xval->GetTuple1(0);
    for (vtkIdType i = 1; i < npts; ++i)
    {
        if (xval->GetTuple1(i) > xmax)
        {
            newX->SetTuple1(nptsValid, xval->GetTuple1(i));
            newY->SetTuple1(nptsValid, yval->GetTuple1(i));
            nptsValid++;
            xmax = xval->GetTuple1(i);
        }
    }

    return rv;
}
