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


// ****************************************************************************
//  Method: avtShiftCenteringFilter constructor
//
//  Arguments:
//      ci      The centering instructions
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
// ****************************************************************************

avtShiftCenteringFilter::avtShiftCenteringFilter(int ci)
{
    centeringInstruction = ci;
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
//      inDS       The input dataset.
//      <unused>   The domain number.
//
//  Returns:       The output dataset. 
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
// ****************************************************************************

vtkDataSet *
avtShiftCenteringFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkDataSet *newDS = (vtkDataSet *) inDS->NewInstance();
    newDS->ShallowCopy(inDS);
    vtkDataSet *outDS = newDS;

    avtCentering centering
                        = GetInput()->GetInfo().GetAttributes().GetCentering();
    if (centeringInstruction == 1 && centering == AVT_ZONECENT)
    {
        //
        //  User requested node-centered but our data is zone-centered,
        //  create the point data from cell data.
        //
        vtkCellDataToPointData *cd2pd = vtkCellDataToPointData::New();
        cd2pd->SetInput(inDS);
        cd2pd->GetExecutive()->SetOutputData(0, outDS);
        cd2pd->Update();
        cd2pd->Delete();

        // We want to preserve knowledge of ghost zones
        vtkDataArray *ghosts = inDS->GetCellData()->GetArray("avtGhostZones");
        if (ghosts)
        {
            outDS->GetCellData()->AddArray(ghosts);
            // Only want ghost cell-data, not ghost point-data.
            outDS->GetPointData()->RemoveArray("avtGhostZones");
        }
        if (inDS->GetPointData()->GetArray("avtGhostNodes") != NULL)
        {
            outDS->GetPointData()->AddArray(
                              inDS->GetPointData()->GetArray("avtGhostNodes"));
        }
        // We want to preserve knowledge of original cells 
        vtkDataArray *origCells = 
                       inDS->GetCellData()->GetArray("avtOriginalCellNumbers");
        if (origCells)
        {
            outDS->GetCellData()->AddArray(origCells);
            // Only want origCells cell-data, not origCells point-data.
            outDS->GetPointData()->RemoveArray("avtOriginalCellNumbers");
        }
    }
    else if (centeringInstruction == 2 && centering == AVT_NODECENT)
    {
        //
        //  User requested zone-centered but our data is node-centered,
        //  create the cell data from point data.
        //
        vtkPointDataToCellData *pd2cd = vtkPointDataToCellData::New();
     
        pd2cd->SetInput(inDS);
        pd2cd->GetExecutive()->SetOutputData(0, outDS);
        pd2cd->Update();
        pd2cd->Delete();

        // We want to preserve knowledge of ghost zones
        vtkDataArray *ghosts = inDS->GetCellData()->GetArray("avtGhostZones");
        if (ghosts)
        {
            outDS->GetCellData()->AddArray(ghosts);
        }
        vtkDataArray *gn = inDS->GetPointData()->GetArray("avtGhostNodes");
        if (gn != NULL)
        {
            outDS->GetPointData()->AddArray(gn);
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

    if (outDS == newDS)
    {
        ManageMemory(outDS);
    }
    newDS->Delete();

    return outDS;
}


// ****************************************************************************
//  Method: avtShiftCenteringFilter::RefashionDataObjectInfo
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
// ****************************************************************************

void
avtShiftCenteringFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &in_atts = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &out_atts = GetOutput()->GetInfo().GetAttributes();
    if (!in_atts.ValidActiveVariable())
    {
        // We don't have enough information to figure out which variable
        // we are shifting the centering of.
        return;
    }

    if (centeringInstruction == 1 || centeringInstruction == 2)
    {
        avtCentering centering = (centeringInstruction == 1 ? AVT_NODECENT
                                                            : AVT_ZONECENT);
        out_atts.SetCentering(centering);
    }
}


// ****************************************************************************
//  Method: avtShiftCenteringFilter::PerformRestriction
//
//  Purpose:
//      Tell the database that we will need ghost zones.
//
//  Programmer: Hank Childs
//  Creation:   August 11, 2004
//
// ****************************************************************************

avtPipelineSpecification_p
avtShiftCenteringFilter::PerformRestriction(avtPipelineSpecification_p in_spec)
{
    avtPipelineSpecification_p spec = new avtPipelineSpecification(in_spec);
    spec->GetDataSpecification()->SetDesiredGhostDataType(GHOST_ZONE_DATA);
    return spec;
}


