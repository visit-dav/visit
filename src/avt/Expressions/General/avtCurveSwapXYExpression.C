// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtCurveSwapXYExpression.C                          //
// ************************************************************************* //

#include <avtCurveSwapXYExpression.h>

#include <math.h>
#include <float.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <vtkVisItUtility.h>


// ****************************************************************************
//  Method: avtCurveSwapXYExpression constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 27, 2012
//
// ****************************************************************************

avtCurveSwapXYExpression::avtCurveSwapXYExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurveSwapXYExpression destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 27, 2012
//
// ****************************************************************************

avtCurveSwapXYExpression::~avtCurveSwapXYExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCurveSwapXYExpression::ExecuteData
//
//  Purpose:
//      The code to swap the x and y coordinates of the curve.
//
//  Arguments:
//      in_ds     The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer: Eric Brugger
//  Creation:   August 27, 2012
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtCurveSwapXYExpression::ExecuteData(vtkDataSet *in_ds, int index,
                                      std::string label)
{
    //
    // Get the inputs.
    //
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(in_ds);
    vtkDataArray *xval = rgrid->GetXCoordinates();
    vtkDataArray *yval = in_ds->GetPointData()->GetArray(activeVariable);
    vtkIdType npts = xval->GetNumberOfTuples();

    //
    // Determine the number of valid points.
    //
    int nptsValid = 1;
    double ymax = yval->GetTuple1(0);
    for (vtkIdType i = 1; i < npts; ++i)
    {
        if (yval->GetTuple1(i) > ymax)
        {
            nptsValid++;
            ymax = yval->GetTuple1(i);
        }
    }

    //
    // Create the output data set.
    //
    vtkRectilinearGrid *rv = vtkVisItUtility::Create1DRGrid(nptsValid, xval->GetDataType());

    vtkDataArray *newX = rv->GetXCoordinates();
    vtkDataArray *newY = yval->NewInstance();
    newY->SetNumberOfTuples(nptsValid);
    newY->SetName(GetOutputVariableName());
    rv->GetPointData()->SetScalars(newY);
    newY->Delete();

    //
    // Set the x and y values.
    //
    newX->SetTuple1(0, yval->GetTuple1(0));
    newY->SetTuple1(0, xval->GetTuple1(0));
    nptsValid = 1;
    ymax = yval->GetTuple1(0);
    for (vtkIdType i = 1; i < npts; ++i)
    {
        if (yval->GetTuple1(i) > ymax)
        {
            newX->SetTuple1(nptsValid, yval->GetTuple1(i));
            newY->SetTuple1(nptsValid, xval->GetTuple1(i));
            nptsValid++;
            ymax = yval->GetTuple1(i);
        }
    }

    return rv;
}
