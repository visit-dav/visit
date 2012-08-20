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
//                     avtCurveIntegrateExpression.C                         //
// ************************************************************************* //

#include <avtCurveIntegrateExpression.h>

#include <math.h>
#include <float.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <vtkVisItUtility.h>


// ****************************************************************************
//  Method: avtCurveIntegrateExpression constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2012
//
// ****************************************************************************

avtCurveIntegrateExpression::avtCurveIntegrateExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurveIntegrateExpression destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2012
//
// ****************************************************************************

avtCurveIntegrateExpression::~avtCurveIntegrateExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurveIntegrateExpression::ExecuteData
//
//  Purpose:
//      The code to integrate the curve.
//
//  Arguments:
//      in_ds     The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2012
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtCurveIntegrateExpression::ExecuteData(vtkDataSet *in_ds, int index,
                                         std::string label)
{
    vtkRectilinearGrid *curve = vtkRectilinearGrid::SafeDownCast(in_ds);
    vtkDataArray *xvals = curve->GetXCoordinates();
    vtkDataArray *yvals = curve->GetPointData()->GetArray(varnames[0]);

    vtkIdType npts = yvals->GetNumberOfTuples();
    vtkRectilinearGrid *rv = vtkVisItUtility::Create1DRGrid(npts, yvals->GetDataType());

    vtkDataArray *newXvals = rv->GetXCoordinates();
    vtkDataArray *newYvals = yvals->NewInstance();
    newYvals->SetNumberOfTuples(npts);
    newYvals->SetName(GetOutputVariableName());
    rv->GetPointData()->SetScalars(newYvals);
    newYvals->Delete();

    double sum = 0.;
    newXvals->SetTuple1(0, xvals->GetTuple1(0));
    newYvals->SetTuple1(0, sum);
    for (vtkIdType i = 1; i < npts; ++i)
    {
        double dx = xvals->GetTuple1(i) - xvals->GetTuple1(i-1);
        double dy = (yvals->GetTuple1(i-1) + yvals->GetTuple1(i)) / 2.;
        sum += dx  * dy;
        newXvals->SetTuple1(i, xvals->GetTuple1(i));
        newYvals->SetTuple1(i, sum);
    }

    return rv;
}
