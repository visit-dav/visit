// ************************************************************************* //
//                            avtGhostZoneFilter.C                           //
// ************************************************************************* //

#include <avtGhostZoneFilter.h>

#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>

#include <DebugStream.h>


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
// ****************************************************************************

vtkDataSet *
avtGhostZoneFilter::ExecuteData(vtkDataSet *in_ds, int domain, std::string)
{
    if (in_ds->GetNumberOfCells() == 0)
    {
        debug5 << "No Cells in input! domain:  " << domain << endl;
        return in_ds;
    }

    if (in_ds->GetCellData()->GetArray("vtkGhostLevels") == NULL)
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
    filter->SetGhostLevel(1);
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


