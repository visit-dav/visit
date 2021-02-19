// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtEdgeNormalExpression.C                         //
// ************************************************************************* //

#include <avtEdgeNormalExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItPolyDataNormals2D.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtEdgeNormalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer: Jeremy Meredith
//  Creation:   March 10, 2014
//
//  Modifications:
//
//    Alister Maguire, Fri Oct  9 11:46:22 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtEdgeNormalExpression::avtEdgeNormalExpression()
{
    isPoint = true;
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtEdgeNormalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer: Jeremy Meredith
//  Creation:   March 10, 2014
//
// ****************************************************************************

avtEdgeNormalExpression::~avtEdgeNormalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtEdgeNormalExpression::DeriveVariable
//
//  Purpose:
//      Assigns the zone ID to each variable.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer:   Jeremy Meredith
//  Creation:     March 10, 2014
//
//  Modifications:
//
// ****************************************************************************

vtkDataArray *
avtEdgeNormalExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        return RectilinearDeriveVariable((vtkRectilinearGrid *) in_ds);
    }

    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The Edge normal expression "
                   "can only be calculated on edges.");
    }

    vtkPolyData *pd = (vtkPolyData *) in_ds;

    vtkVisItPolyDataNormals2D *n = vtkVisItPolyDataNormals2D::New();
    if (isPoint)
        n->SetNormalTypeToPoint();
    else
        n->SetNormalTypeToCell();
    n->SetInputData(pd);
    n->Update();
    vtkPolyData *out = n->GetOutput();

    vtkDataArray *arr = NULL;
    if (isPoint)
        arr = out->GetPointData()->GetNormals();
    else
        arr = out->GetCellData()->GetNormals();
    

    if (arr == NULL)
    {
        n->Delete();
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "An internal error occurred where "
                   "the edge normals could not be calculated.  Please "
                   "contact a VisIt developer.");
    }

    arr->Register(NULL);
    n->Delete();

    return arr;
}



// ****************************************************************************
//  Method: avtEdgeNormalExpression::RectilinearDeriveVariable
//
//  Purpose:
//      A method that does special handling for rectilinear generation.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer: Jeremy Meredith
//  Creation:   March 10, 2014
//
// ****************************************************************************

vtkDataArray *
avtEdgeNormalExpression::RectilinearDeriveVariable(vtkRectilinearGrid *rgrid)
{
    int dims[3];
    rgrid->GetDimensions(dims);
    int nMatch = 0;
    bool doX = (dims[0] == 1);
    if (doX)
        nMatch++;
    bool doY = (dims[1] == 1);
    if (doY)
        nMatch++;
    bool doZ = (dims[2] == 1);
    if (doZ)
        nMatch++;
    if (nMatch == 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Can not determine "
                   "edge normals for a 3D data set.");
    }
    if (nMatch == 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Can not determine "
                   "edge normals for polygons.");
    }
    if (nMatch == 3)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Can not determine "
                   "edge normals for vertices.");
    }
    vtkDataArray *n = CreateArrayFromMesh(rgrid);
    n->SetNumberOfComponents(3);
    vtkIdType ntuples = (isPoint ? rgrid->GetNumberOfPoints() 
                           : rgrid->GetNumberOfCells());
    n->SetNumberOfTuples(ntuples);
    double norm[3] = { 0, 0, 0 };
    if (doY)
        norm[1] = 1.0;
    else
        norm[0] = 1.0;
    for (vtkIdType i = 0 ; i < ntuples ; i++)
    {
        n->SetTuple(i, norm);
    }

    return n;
}


