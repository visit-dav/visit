// ************************************************************************* //
//                           avtNeighborFilter.C                             //
// ************************************************************************* //

#include <avtNeighborFilter.h>

#include <math.h>
#include <float.h>

#include <vtkCellArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkVertex.h>

#include <avtExtents.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

const char * avtNeighborFilter::variableName = "neighbor";

// ****************************************************************************
//  Method: avtNeighborFilter constructor
//
//  Programmer: Akira Haddox
//  Creation:   June 27, 2002
//
// ****************************************************************************

avtNeighborFilter::avtNeighborFilter()
{

}


// ****************************************************************************
//  Method: avtNeighborFilter destructor
//
//  Programmer: Akira Haddox
//  Creation:   June 27, 2002
//
// ****************************************************************************

avtNeighborFilter::~avtNeighborFilter()
{
}

// ****************************************************************************
//  Method: avtNeighborFilter::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Akira Haddox
//  Creation:     June 27, 2002
//
// ****************************************************************************
 
vtkDataSet *
avtNeighborFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    // Let's get the points from the input dataset.
    vtkPoints *pts = NULL;
    switch (in_ds->GetDataObjectType())
    {
      // Easily done, just grab them.
      case VTK_UNSTRUCTURED_GRID:
      case VTK_POLY_DATA:
      case VTK_STRUCTURED_GRID:
        pts = ((vtkPointSet *) in_ds)->GetPoints();
        break;
 
      // If they're any other type of grid, this filter shouldn't be used
      default:
        EXCEPTION0(ImproperUseException);
    }
 
    int nPoints = pts->GetNumberOfPoints();

    // A neighbor filter would require the existance of neighbors
    if (nPoints < 2)
        EXCEPTION0(ImproperUseException);

    // Now that we've done all of our checks for improper use,
    // let's allocate for our needs
    vtkPolyData *results = vtkPolyData::New();
    vtkCellArray *verts = vtkCellArray::New();
    vtkFloatArray *data = vtkFloatArray::New();

    results->SetPoints(pts);

    data->SetNumberOfComponents(1);
    data->SetNumberOfValues(nPoints);

    float bounds[6];
    in_ds->GetBounds(bounds);

    // Create the point locator
    vtkPointLocator *ptLoc = vtkPointLocator::New();
    ptLoc->SetDataSet(in_ds);
    ptLoc->BuildLocator();

    for (vtkIdType id = 0; id < nPoints; id++)
    {
        // Build the vertex list
        vtkVertex *v = vtkVertex::New();
        v->Initialize( 1, &id, pts);
        verts->InsertNextCell(v);
        v->Delete();

        // And at the same time, set the distance data
        float *coords = pts->GetPoint(id);

        // Find the closest 2 points, since the closest is itself of course.
        vtkIdList *closeId = vtkIdList::New();
        ptLoc->FindClosestNPoints(2, coords, closeId);

        float *nearCoords = pts->GetPoint(closeId->GetId(1));

        float distance = (coords[0]-nearCoords[0])*(coords[0]-nearCoords[0]) +
                         (coords[1]-nearCoords[1])*(coords[1]-nearCoords[1]) +  
                         (coords[2]-nearCoords[2])*(coords[2]-nearCoords[2]);
        distance=sqrt(distance);

        data->SetValue(id, distance);
        closeId->Delete();
    }
 
    data->SetName("neighbor");
    results->GetPointData()->AddArray(data);
    results->GetPointData()->SetActiveScalars("neighbor");

    results->SetVerts(verts);


    //
    // Make our best attempt at maintaining our extents.
    //
    double exts[2];
    float range[2];
    data->GetRange(range, 0);
    exts[0] = range[0];
    exts[1] = range[1];

    GetOutput()->GetInfo().GetAttributes().GetTrueDataExtents()->Merge(exts);

    data->Delete();
    verts->Delete();
    pts->Delete();

    ManageMemory(results);
    results->Delete();
    return results;    
}

// ****************************************************************************
//  Method: avtNeighborFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Tells our output that we now have a variable, and that our mesh has
//      drastically changed.
//
//  Programmer: Akira Haddox
//  Creation:   June 27, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Feb 20 15:51:54 PST 2004
//    Have base class do more of the work in setting up output.
//
// ****************************************************************************
 
void
avtNeighborFilter::RefashionDataObjectInfo(void)
{
    avtSingleInputExpressionFilter::RefashionDataObjectInfo();

    // And we're now dealing with vertexes
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(0);
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}
