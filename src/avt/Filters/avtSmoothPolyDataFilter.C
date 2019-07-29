// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtSmoothPolyDataFilter.C                         //
// ************************************************************************* //

#include <avtSmoothPolyDataFilter.h>

#include <vtkDataSet.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkPolyData.h>
#include <vtkGeometryFilter.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>


using     std::string;


// ****************************************************************************
//  Method: avtSmoothPolyDataFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSmoothPolyDataFilter::avtSmoothPolyDataFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtSmoothPolyDataFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtSmoothPolyDataFilter::~avtSmoothPolyDataFilter()
{
    ;
}


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
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
//
//  Programmer: Jeremy Meredith
//  Creation:   December  6, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Feb 23 10:41:33 EST 2009
//    If we didn't get poly data, don't throw an error, convert it.
//    Also, do that check after our no-op checks to save time.
//
//    Jeremy Meredith, Mon Feb 23 16:58:50 EST 2009
//    Added deletion of geometry filter.
//
//    Kathleen Biagas, Fri Jan 25 16:04:46 PST 2013
//    Call Update on the filter, not the data object.
//
//    Eric Brugger, Tue Jul 22 08:24:19 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eric Brugger, Tue Aug 25 10:28:27 PDT 2015
//    Modified the routine to return NULL if the output data set had no
//    geometry.
//
// ****************************************************************************

avtDataRepresentation *
avtSmoothPolyDataFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    // Detect no-ops
    if (smoothingLevel == 0 ||
        GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 2 ||
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension()     != 3)
    {
        return inDR;
    }

    // We only work on surface data
    vtkGeometryFilter *geom = NULL;
    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        geom = vtkGeometryFilter::New();
        geom->SetInputData(inDS);
        inDS = geom->GetOutput();
    }

    //
    // Set up and apply the filter
    //
    vtkSmoothPolyDataFilter *smoothPolyData = vtkSmoothPolyDataFilter::New();
    smoothPolyData->SetInputData((vtkPolyData*)inDS);
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

    smoothPolyData->Update();

    avtDataRepresentation *outDR = NULL;
    if (newDS->GetNumberOfCells() > 0)
    {
        outDR = new avtDataRepresentation(newDS,
            inDR->GetDomain(), inDR->GetLabel());
    }

    newDS->Delete();
    smoothPolyData->Delete();
    if (geom)
        geom->Delete();

    return outDR;
}
