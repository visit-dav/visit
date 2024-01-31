// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtZoneCentersExpression.C                         //
// ************************************************************************* //

#include <avtZoneCentersExpression.h>

#include <avtDataAttributes.h>
#include <avtDataObjectInformation.h>
#include <avtDataset.h>
#include <ExpressionException.h>

#include <vtkCellCenters.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

// ****************************************************************************
// Method: avtZoneCenters Constructor
//
// Purpose:
//  Construct avtZoneCentersExpression
//
// Programmer: Chris Laganella
// Creation:   Mon Jan 31 16:05:01 EST 2022
//
// Modifications:
//
// ****************************************************************************
avtZoneCentersExpression::avtZoneCentersExpression()
{

}

// ****************************************************************************
// Method: avtZoneCenters Destructor
//
// Purpose:
//  Destroy avtZoneCentersExpression
//
// Programmer: Chris Laganella
// Creation:   Mon Jan 31 16:05:01 EST 2022
//
// Modifications:
//
// ****************************************************************************
avtZoneCentersExpression::~avtZoneCentersExpression()
{

}

// ****************************************************************************
// Method: avtZoneCenters Destructor
//
// Purpose:
//  Calculate the center of each cell in the given vtkDataSet
//
// Programmer: Chris Laganella
// Creation:   Mon Jan 31 16:05:01 EST 2022
//
// Modifications:
//
// ****************************************************************************
vtkDataArray *
avtZoneCentersExpression::DeriveVariable(vtkDataSet *ds, int currentDomainsIndex)
{
    // Invoke vtkCellCenters filter
    vtkSmartPointer<vtkCellCenters> cellCenters = vtkCellCenters::New();
    cellCenters->SetInputData(ds);
    cellCenters->Update();

    // Retrieve output
    vtkPolyData *polyData = cellCenters->GetOutput();
    vtkDataArray *out = NULL;
    if(polyData)
    {
        vtkPoints *points = polyData->GetPoints();
        if(points)
        {
            out = points->GetData();
        }
    }

    // Report error if there was no output
    if(!out)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "An internal error occurred where "
                   "the cell centers could not be calculated.  Please "
                   "contact a VisIt developer.");
        return out;
    }
    out->Register(NULL);
    return out;
}
