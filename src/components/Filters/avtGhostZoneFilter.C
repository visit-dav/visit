// ************************************************************************* //
//                            avtGhostZoneFilter.C                           //
// ************************************************************************* //

#include <avtGhostZoneFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkPointData.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtGhostZoneFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtGhostZoneFilter::avtGhostZoneFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtGhostZoneFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtGhostZoneFilter::~avtGhostZoneFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtGhostZoneFilter::ExecuteDataTree
//
//  Purpose:
//      Sends the specified input and output through the GhostZone filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      domain     The domain number.
//      <unused>   The label associated with this dataset.
//
//  Returns:       The output dataset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 1, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jun 26 18:57:57 PDT 2001
//    Handle case where input has no cells a little better.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001
//    Added string arguments so that label will get passed to output. 
//
//    Hank Childs, Mon Apr 15 11:32:52 PDT 2002
//    Address memory leaks.
//
//    Hank Childs, Tue Sep 10 12:54:01 PDT 2002
//    Renamed to ExecuteData.  Added support for memory management.
//
//    Hank Childs, Sun Jun 27 09:45:20 PDT 2004
//    Add support for ghost nodes as well.
//
//    Hank Childs, Fri Aug 27 16:02:58 PDT 2004
//    Rename ghost data array.  Also remove SetGhostLevel call.
//
// ****************************************************************************

vtkDataSet *
avtGhostZoneFilter::ExecuteData(vtkDataSet *in_ds, int domain, std::string)
{
    if (in_ds->GetNumberOfCells() == 0)
    {
        debug5 << "No Cells in input! domain:  " << domain << endl;
        return in_ds;
    }

    bool haveGhostZones = 
                    (in_ds->GetCellData()->GetArray("avtGhostZones") != NULL);
    bool haveGhostNodes = (in_ds->GetDataObjectType() == VTK_POLY_DATA) &&
                    (in_ds->GetPointData()->GetArray("avtGhostNodes") != NULL);
    if (!haveGhostZones && !haveGhostNodes)
    {
        //
        //  No ghost cells, no need to use this filter.
        //
        debug5 << "No Ghost Zones present! domain:  " << domain << endl;
        return in_ds;
    }

    debug5 << "Using vtkDataSetRemoveGhostCells" << endl;

    vtkDataSetRemoveGhostCells *filter = vtkDataSetRemoveGhostCells::New();
    filter->SetInput(in_ds);

    //
    // Set up filter so that all cells with ghost level >=1
    // will be removed.  Currently our 'real' zones have
    // ghostlevel ==0, 'ghost' have ghostlevel ==1.
    //
    vtkDataSet *outDS = filter->GetOutput();
    outDS->Update();

    if (outDS->GetNumberOfCells() == 0)
    {
        outDS = NULL;
    }

    ManageMemory(outDS);
    filter->Delete();

    return outDS;
}


// ****************************************************************************
//  Method: avtGhostZoneFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicate that the zones are invalidated after this operation.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 1, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Sep 30 09:55:26 PDT 2002
//    Tell the output that it does not contain any ghost zones.
//
// ****************************************************************************

void
avtGhostZoneFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
    GetOutput()->GetInfo().GetAttributes().
                                          SetContainsGhostZones(AVT_NO_GHOSTS);
}


