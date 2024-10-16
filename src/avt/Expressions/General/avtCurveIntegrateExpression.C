// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Method: avtCurveIntegrateExpression::DoOperation
//
//  Purpose:
//      The code to integrate the curve.
//
//  Arguments:
//      in        The input data array.
//      out       The output data array.
//      <unused>  The number of components.
//      ntuples   The number of tuples in the data arrays.
//
//  Returns:      The output dataset.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2012
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveIntegrateExpression::DoOperation(vtkDataArray *in, vtkDataArray *out,
                                         int ncomponents, int ntuples, vtkDataSet *in_ds)
{
    vtkRectilinearGrid *curve = vtkRectilinearGrid::SafeDownCast(cur_mesh);
    vtkDataArray *xcoords = curve->GetXCoordinates();

    double sum = 0.;
    out->SetTuple1(0, sum);
    for (vtkIdType i = 1; i < ntuples; ++i)
    {
        double dx = xcoords->GetTuple1(i) - xcoords->GetTuple1(i-1);
        double dy = (in->GetTuple1(i-1) + in->GetTuple1(i)) / 2.;
        sum += dx * dy;
        out->SetTuple1(i, sum);
    }
}
