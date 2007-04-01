// ************************************************************************* //
//                         avtSmoothPolyDataFilter.C                         //
// ************************************************************************* //

#include <avtSmoothPolyDataFilter.h>

#include <vtkDataSet.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkPolyData.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>


using     std::string;

// ****************************************************************************
//  Method:  avtSmoothPolyDataFilter::SetSmoothingLevel
//
//  Purpose:
//    Set the smoothing level (0, 1, or 2)
//
//  Arguments:
//    sl         the smoothing level
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2002
//
// ****************************************************************************

void
avtSmoothPolyDataFilter::SetSmoothingLevel(int sl)
{
    smoothingLevel = sl;
}

// ****************************************************************************
//  Method: avtSmoothPolyDataFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the SmoothPolyData filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output geometry.
//
//  Programmer: Jeremy Meredith
//  Creation:   December  6, 2002
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtSmoothPolyDataFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    // We only work on surface data
    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION1(VisItException, "avtSmoothPolyDataFilter::ExecuteDataTree "
                                   "-- Did not get polydata");
    }

    if (smoothingLevel == 0 ||
        GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 2 ||
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension()     != 3)
    {
        return inDS;
    }

    //
    // Set up and apply the filter
    //
    vtkSmoothPolyDataFilter *smoothPolyData = vtkSmoothPolyDataFilter::New();
    smoothPolyData->SetInput((vtkPolyData*)inDS);
    vtkPolyData *newDS = vtkPolyData::New();
    smoothPolyData->SetOutput(newDS);

    smoothPolyData->SetConvergence(0.0);   // always do requested # iterations
    smoothPolyData->SetFeatureAngle(60.0);
    smoothPolyData->SetEdgeAngle(25.0);
    smoothPolyData->SetFeatureEdgeSmoothing(1);
    smoothPolyData->SetBoundarySmoothing(1);

    //
    // Right now we support 3 different levels -- off, fast, and nice/high
    //
    switch (smoothingLevel)
    {
      case 0:
        smoothPolyData->SetNumberOfIterations(0);
        break;
      case 1:
        smoothPolyData->SetRelaxationFactor(1.0);
        smoothPolyData->SetNumberOfIterations(1);
        break;
      case 2:
        smoothPolyData->SetRelaxationFactor(0.05);
        smoothPolyData->SetNumberOfIterations(100);
        break;
    }

    newDS->Update();

    vtkDataSet *outDS = NULL;
    if (newDS->GetNumberOfCells() > 0)
    {
        outDS = newDS;
    }

    ManageMemory(outDS);
    newDS->Delete();
    smoothPolyData->Delete();

    return outDS;
}
