// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtSurfaceNormalExpression.C                        //
// ************************************************************************* //

#include <avtSurfaceNormalExpression.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkGeometryFilter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItPolyDataNormals.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtSurfaceNormalExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
//  Modifications:
//
//    Alister Maguire, Fri Oct  9 11:46:22 PDT 2020
//    Set canApplyToDirectDatabaseQOT to false.
//
// ****************************************************************************

avtSurfaceNormalExpression::avtSurfaceNormalExpression()
{
    isPoint            = true;
    zonesHaveBeenSplit = false;
    canApplyToDirectDatabaseQOT = false;
}


// ****************************************************************************
//  Method: avtSurfaceNormalExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2005
//
// ****************************************************************************

avtSurfaceNormalExpression::~avtSurfaceNormalExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtSurfaceNormalExpression::DeriveVariable
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
//  Programmer:   Hank Childs
//  Creation:     September 22, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Sep 24 10:18:38 PDT 2010
//    Add support for rectilinear grids.
//
//    Mark C. Miller, Wed Aug 22 09:29:42 PDT 2012
//    Fixed leak of 'n' on early return due to EXCEPTION.
//
//    Mark C. Miller, Sat Aug 25 22:06:48 PDT 2012
//    Changed leak fix and put n->Delete() both in exception block at at end
//    of routine rather than ahead of arr->Register().
//
//    Kathleen Biagas, Fri Jan 25 16:28:49 PST 2013
//    Call 'Update' on filter instead of data object.
//
//    Brad Whitlock, Tue Dec  5 16:30:57 PST 2023
//    Execute vtkGeometryFilter on input dataset if it is not polydata. The
//    old recommendation in the code did not work for vtkUnstructuredGrids
//    that contained surfaces.
//
// ****************************************************************************

vtkDataArray *
avtSurfaceNormalExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        return RectilinearDeriveVariable((vtkRectilinearGrid *) in_ds);
    }

    // Execute a geometry filter if the input dataset is not already polydata.
    vtkPolyData *pd = NULL;
    vtkGeometryFilter *geom = NULL;
    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        geom = vtkGeometryFilter::New();
        geom->SetInputData(in_ds);
        geom->Update();
        pd = (vtkPolyData *) geom->GetOutput();
    }
    else
    {
        pd = (vtkPolyData *) in_ds;
    }

    vtkVisItPolyDataNormals *n = vtkVisItPolyDataNormals::New();
    n->SetSplitting(false);
    if (isPoint)
        n->SetNormalTypeToPoint();
    else
        n->SetNormalTypeToCell();
    n->SetInputData(pd);
    n->Update();
    vtkPolyData *out = n->GetOutput();

    if (n->GetStripsHaveBeenDecomposed())
    {
        zonesHaveBeenSplit = true;
    }

    vtkDataArray *arr = NULL;
    if (isPoint)
        arr = out->GetPointData()->GetNormals();
    else
        arr = out->GetCellData()->GetNormals();
    

    if (arr == NULL)
    {
        n->Delete();
        geom->Delete();
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "An internal error occurred where "
                   "the surface normals could not be calculated.  Please "
                   "contact a VisIt developer.");
    }

    arr->Register(NULL);
    n->Delete();
    geom->Delete();

    return arr;
}



// ****************************************************************************
//  Method: avtSurfaceNormalExpression::RectilinearDeriveVariable
//
//  Purpose:
//      A method that does special handling for rectilinear generation.
//
//  Programmer: Hank Childs
//  Creation:   September 24, 2010
//
// ****************************************************************************

vtkDataArray *
avtSurfaceNormalExpression::RectilinearDeriveVariable(vtkRectilinearGrid *rgrid)
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
                   "surface normals for a 3D data set.");
    }
    if (nMatch > 1)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "Can not determine "
                   "surface normals for lines and vertices.");
    }
    vtkDataArray *n = CreateArrayFromMesh(rgrid);
    n->SetNumberOfComponents(3);
    vtkIdType ntuples = (isPoint ? rgrid->GetNumberOfPoints() 
                           : rgrid->GetNumberOfCells());
    n->SetNumberOfTuples(ntuples);
    double norm[3] = { 0, 0, 0 };
    if (doX)  
        norm[0] = 1.0;
    if (doY)  
        norm[1] = 1.0;
    if (doZ)  
        norm[2] = 1.0;
    for (vtkIdType i = 0 ; i < ntuples ; i++)
    {
        n->SetTuple(i, norm);
    }

    return n;
}


// ****************************************************************************
//  Method: avtSurfaceNormalExpression::UpdateDataObjectInfo
//
//  Purpose:
//    Update the data object information.
//
//  Programmer: Alister Maguire
//  Creation:   April 27, 2020
//
// ****************************************************************************

void
avtSurfaceNormalExpression::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inputAtts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();

    SetExpressionAttributes(inputAtts, outAtts);

    //
    // We need to inform VisIt if the PolyDataNormalsFilter has split
    // cells.
    //
    if (zonesHaveBeenSplit)
    {
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
        GetOutput()->GetInfo().GetValidity().ZonesSplit();
    }
}
