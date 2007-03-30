// ************************************************************************* //
//                          avtFeatureEdgesFilter.C                          //
// ************************************************************************* //

#include <avtFeatureEdgesFilter.h>

#include <vtkDataSet.h>
#include <vtkFeatureEdges.h>
#include <vtkPolyData.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>


using     std::string;


// ****************************************************************************
//  Method: avtFeatureEdgesFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the FeatureEdges filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output unstructured grid.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 13 15:18:09 PST 2002
//    Made it only use feature-angle detection on spatially 3D meshes.
//
//    Hank Childs, Tue Sep 10 15:43:53 PDT 2002
//    Renamed to ExecuteData and handled memory issues entirely from within
//    routine.
//
// ****************************************************************************

vtkDataSet *
avtFeatureEdgesFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        // We only work on surface data
        EXCEPTION1(VisItException, "avtFeatureEdgesFilter::ExecuteDataTree "
                                   "-- Did not get polydata");
    }

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
    {
        return inDS;
    }

    //
    // Set up and apply the filter
    //
    vtkFeatureEdges *featureEdgesFilter = vtkFeatureEdges::New();
    featureEdgesFilter->SetInput((vtkPolyData*)inDS);
    vtkPolyData *newDS = vtkPolyData::New();
    featureEdgesFilter->SetOutput(newDS);

    featureEdgesFilter->BoundaryEdgesOn();
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 3)
    {
        featureEdgesFilter->FeatureEdgesOn();
        featureEdgesFilter->SetFeatureAngle(60.0);
    }
    else
    {
        featureEdgesFilter->FeatureEdgesOff();
    }
    featureEdgesFilter->NonManifoldEdgesOff();
    featureEdgesFilter->ManifoldEdgesOff();
    featureEdgesFilter->ColoringOff();

    newDS->Update();

    vtkDataSet *outDS = NULL;
    if (newDS->GetNumberOfCells() > 0)
    {
        outDS = newDS;
    }

    ManageMemory(outDS);
    newDS->Delete();
    featureEdgesFilter->Delete();

    return outDS;
}


// ****************************************************************************
//  Method: avtFeatureEdgesFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 12, 2001
//
//  Modifications:
// ****************************************************************************

void
avtFeatureEdgesFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


