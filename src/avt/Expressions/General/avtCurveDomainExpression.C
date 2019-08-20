// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
