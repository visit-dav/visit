// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtShiftCenteringFilter.C                          //
// ************************************************************************* //

#include <avtShiftCenteringFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataSet.h>
#include <vtkExecutive.h>
#include <vtkPointData.h>
#include <vtkPointDataToCellData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <DebugStream.h>

#include <string>
#include <vector>


// ****************************************************************************
//  Method: avtShiftCenteringFilter constructor
//
//  Arguments:
//      target      The target centering.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 19, 2001.
//
//  Modifications:
//
//    Hank Childs, Fri Feb  8 18:33:39 PST 2002
//    Removed all references to PC atts.
//
//    Hank Childs, Tue Sep 10 15:00:49 PDT 2002
//    Removed data members for shifting centering.
//
//    Sean Ahern, Wed Sep 10 13:13:18 EDT 2008
//    For ease of reading code, I forced the argument to be
//    avtCentering, not integer.
//
// ****************************************************************************

avtShiftCenteringFilter::avtShiftCenteringFilter(avtCentering target)
{
    centeringTarget = target;
}


// ****************************************************************************
//  Method: avtShiftCenteringFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtShiftCenteringFilter::~avtShiftCenteringFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtShiftCenteringFilter::ExecuteData
//
//  Purpose:
//      Shifts the centering of the data from point to cell or vice-versa if
//      necessary.
//
//  Arguments:
//      inDR       The input data representation.
//
//  Returns:       The output data representation. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 19, 2001 
//
//  Modifications:
//
//    Hank Childs, Thu Apr 26 14:51:39 PDT 2001
//    Memory leak.
//
//    Kathleen Bonnell, Wed Jun 13 11:46:02 PDT 2001
//    Ensure that ghost-cell information is passed on.
//
//    Hank Childs, Wed Jan  9 14:07:42 PST 2002
//    Remove unnecessary code in favor of MakeObject.
//
//    Hank Childs, Fri Jan 25 10:39:07 PST 2002
//    Fix memory leak.
//
//    Hank Childs, Fri Feb  8 18:33:39 PST 2002
//    Remove unnecessary references to PC atts.
//
//    Hank Childs, Tue Sep 10 15:00:49 PDT 200
//    Made filters that were data members become automatic variables.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Use NewInstance instead of MakeObject, to match new vtk api. 
//
//    Kathleen Bonnell, Thu Mar 13 10:23:37 PST 2003   
//    Ensure that avtOriginalCellNumbers remains a cell-data array.
//
//    Hank Childs, Fri Aug 27 16:02:58 PDT 2004
//    Rename ghost data array.  Also add support for ghost nodes.
//
//    Kathleen Bonnell, Wed May 17 15:08:39 PDT 2006 
//    Can no longer use SetOutput on a vtk filter, must grab the filters' 
//    executive and SetOuputData. 
//
//    Hank Childs, Thu Aug  7 09:39:40 PDT 2008
//    Add support for non-floating point arrays.
//
//    Cyrus Harrison, Mon Aug 11 13:48:25 PDT 2008
//    Fixed indexing problem with non-floating point array support. 
//    Switched to use array name and exclude special avt arrays:
//    avtGhostZones, avtGhostNode, avtOriginalCellNumber from int-float-int
//    conversion.
//
//    Sean Ahern, Wed Sep 10 13:13:18 EDT 2008
//    For ease of reading code, I forced the target to be of type
//    avtCentering, not integer.
//
//    Kathleen Biagas, Thu Sep 6 15:18:27 MST 2012
//    Preserve data type.
//
//    Eric Brugger, Tue Jul 22 08:01:18 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Kevin Griffin, Tue Feb  4 17:03:46 PST 2020
//    Removed code converting Int arrays to Float arrays and back. This
//    doesn't appear to be needed anymore.
//
// ****************************************************************************

avtDataRepresentation *
avtShiftCenteringFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();
    vtkDataSet *outDS = (vtkDataSet *) inDS->NewInstance();
    outDS->ShallowCopy(inDS);
    
    if (centeringTarget == AVT_NODECENT)
    {
        //
        //  User requested node-centered but our data is zone-centered,
        //  create the point data from cell data.
        //
        vtkCellDataToPointData *cd2pd = vtkCellDataToPointData::New();
        cd2pd->SetInputData(inDS);
        cd2pd->GetExecutive()->SetOutputData(0, outDS);
        cd2pd->Update();
        cd2pd->Delete();
        
        // We want to preserve knowledge of ghost zones
        vtkDataArray *ghostZones = inDS->GetCellData()->GetArray("avtGhostZones");
        if (ghostZones)
        {
            outDS->GetCellData()->AddArray(ghostZones);
            // Only want ghost cell-data, not ghost point-data.
            outDS->GetPointData()->RemoveArray("avtGhostZones");
        }
        
        // We want to preserve knowledge of original cells
        vtkDataArray *origCells = inDS->GetCellData()->GetArray("avtOriginalCellNumbers");
        if (origCells)
        {
            outDS->GetCellData()->AddArray(origCells);
            // Only want origCells cell-data, not origCells point-data.
            outDS->GetPointData()->RemoveArray("avtOriginalCellNumbers");
        }
    }
    else if (centeringTarget == AVT_ZONECENT)
    {
        //
        //  User requested zone-centered but our data is node-centered,
        //  create the cell data from point data.
        //
        vtkPointDataToCellData *pd2cd = vtkPointDataToCellData::New();
     
        pd2cd->SetInputData(inDS);
        pd2cd->GetExecutive()->SetOutputData(0, outDS);
        pd2cd->Update();
        pd2cd->Delete();

        vtkDataArray *ghostNodes = inDS->GetPointData()->GetArray("avtGhostNodes");
        if (ghostNodes)
        {
            outDS->GetPointData()->AddArray(ghostNodes);
            outDS->GetCellData()->RemoveArray("avtGhostNodes");
        }
    }
    else
    {
        //
        //  We don't need to do anything to our data. 
        //
        outDS = inDS;
    }

    avtDataRepresentation *outDR = new avtDataRepresentation(outDS, inDR->GetDomain(), inDR->GetLabel());
    outDS->Delete();

    return outDR;
}


// ****************************************************************************
//  Method: avtShiftCenteringFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Tells its meta-data object what kind of centering it has.
//
//  Programmer: Hank Childs
//  Creation:   February 27, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Feb 26 08:15:12 PST 2004
//    Account for multiple variables.
//
//    Sean Ahern, Wed Sep 10 13:13:18 EDT 2008
//    For ease of reading code, I forced the target to be of type
//    avtCentering, not integer.
//
// ****************************************************************************

void
avtShiftCenteringFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &in_atts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &out_atts = GetOutput()->GetInfo().GetAttributes();
    if (!in_atts.ValidActiveVariable())
    {
        // We don't have enough information to figure out which variable
        // we are shifting the centering of.
        return;
    }

    if (centeringTarget == AVT_NODECENT || centeringTarget == AVT_ZONECENT)
        out_atts.SetCentering(centeringTarget);
}


// ****************************************************************************
//  Method: avtShiftCenteringFilter::ModifyContract
//
//  Purpose:
//      Tell the database that we will need ghost zones.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2004
//
//  Modifications:
//
//    Kevin Griffin, Wed Feb  5 08:15:32 PST 2020
//    Removed explicit request for ghost zones since there are cases where
//    ghost nodes are needed. The desired ghost data type should already
//    be set by other filters in the pipeline.
//
// ****************************************************************************

avtContract_p
avtShiftCenteringFilter::ModifyContract(avtContract_p in_spec)
{
    return in_spec;
}


