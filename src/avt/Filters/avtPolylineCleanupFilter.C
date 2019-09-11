// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtPolylineCleanupFilter.C                        //
// ************************************************************************* //

#include <avtPolylineCleanupFilter.h>

#include <vtkCleanPolyData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

// ****************************************************************************
//  Method: avtPolylineCleanupFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Nov  7 11:21:19 PST 2016
//
// ****************************************************************************

avtPolylineCleanupFilter::avtPolylineCleanupFilter() : avtDataTreeIterator()
{
}

// ****************************************************************************
//  Method: avtPolylineCleanupFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Nov  7 11:21:19 PST 2016
//
// ****************************************************************************

avtPolylineCleanupFilter::~avtPolylineCleanupFilter()
{
}

// ****************************************************************************
//  Method: avtPolylineCleanupFilter::ExecuteData
//
//  Purpose:
//    Cleans duplicate points from a poly line using the
//    vtkCleanPolyData filter.
//
//  Arguments:
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
//
//  Note: The cell data copying is untested.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Mon Nov  7 11:21:19 PST 2016
//
//  Modifications:
//    Kathleen Biagas, Thu Jun 20 11:17:52 PDT 2019
//    Remove unnecessary handling of active scalars, vtkCleanPolyData
//    doesn't modify them.
//
// ****************************************************************************

avtDataRepresentation *
avtPolylineCleanupFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1)
    {
        return inDR;
    }

    // Clean duplicate points from the polydata.
    vtkCleanPolyData *cleanFilter = vtkCleanPolyData::New();

    cleanFilter->SetInputData(vtkPolyData::SafeDownCast(inDS));
    cleanFilter->Update();

    // Get the output.
    vtkPolyData *outPD = cleanFilter->GetOutput();
    outPD->Register(NULL);

    // Create the output data rep.
    avtDataRepresentation *outDR =
        new avtDataRepresentation(outPD, inDR->GetDomain(), inDR->GetLabel());

    return outDR;
}


// ****************************************************************************
//  Method: avtPolylineCleanupFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Mon Nov  7 11:21:19 PST 2016
//
// ****************************************************************************

void
avtPolylineCleanupFilter::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
}
