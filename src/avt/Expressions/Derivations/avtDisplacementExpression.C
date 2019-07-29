// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDisplacementExpression.C                         //
// ************************************************************************* //

#include <avtDisplacementExpression.h>
#include <math.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtDisplacementExpression constructor
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:28:33 PST 2006
//
// ****************************************************************************

avtDisplacementExpression::avtDisplacementExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDisplacementExpression destructor
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:28:33 PST 2006
//
// ****************************************************************************

avtDisplacementExpression::~avtDisplacementExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtDisplacementExpression::DeriveVariable
//
//  Purpose:
//      Determines the displacement at nodes given the current 
//      geometry and initial configuration.
//
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:28:33 PST 2006
//
//  Modifications:
//    Kathleen Biagas, Wed Apr 4 11:50:12 PDT 2012
//    Set return data type same as input data type.
//
// ****************************************************************************

vtkDataArray *
avtDisplacementExpression::DeriveVariable (vtkDataSet *in_ds, int currentDomainsIndex)
{
    char msg[1024];
    double vals[3];
    double out2[3];
    double x0, y0, z0;               // Initial element geom.
    
    // Let's get the points from the input dataset.
    if (in_ds->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "The displacement expression "
                   "only operates on unstructured grids.");
    }
    vtkUnstructuredGrid *in_usg = vtkUnstructuredGrid::SafeDownCast(in_ds);
    int nPoints = in_usg->GetNumberOfPoints();
    vtkDataArray *coord_data = in_ds->GetPointData()->GetArray(varnames[1]);
    if (coord_data == NULL)
    {
        sprintf(msg, "The displacement expression "
            "could not extract the data array for: %s", varnames[1]);
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }
    vtkDataArray *out = coord_data->NewInstance();
    out->SetNumberOfComponents(3);
    out->SetNumberOfTuples(nPoints);
    for (int i = 0; i < nPoints; i++)
    {   // Get initial element geometry points
        coord_data->GetTuple(i, vals);
        x0 = vals[0];
        y0 = vals[1];
        z0 = vals[2];
        // Get current element geometry points
        in_usg->GetPoint(i, vals);
        out2[0] = vals[0] - x0;
        out2[1] = vals[1] - y0;
        out2[2] = vals[2] - z0;
        out->SetTuple(i, out2);
    }
    return out;
}
// ****************************************************************************
//  Method: avtDisplacementExpression::GetVariableDimension
//
//  Purpose:
//      Determines what the variable dimension of the output is.
//      
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 14:28:33 PST 2006
//  
// ****************************************************************************
    
int
avtDisplacementExpression::GetVariableDimension(void)
{
    if (*(GetInput()) == NULL)
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    if (varnames.size() != 2)
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (! atts.ValidVariable(varnames[0]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int ncomp1 = atts.GetVariableDimension(varnames[0]);
    
    if (! atts.ValidVariable(varnames[1]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int ncomp2 = atts.GetVariableDimension(varnames[1]);
    
    return GetNumberOfComponentsInOutput(ncomp1, ncomp2);
}
