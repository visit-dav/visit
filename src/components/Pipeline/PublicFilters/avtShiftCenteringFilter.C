// ************************************************************************* //
//                        avtShiftCenteringFilter.C                          //
// ************************************************************************* //

#include <avtShiftCenteringFilter.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataSet.h>
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
        cd2pd->SetOutput(outDS);
        cd2pd->Update();
        cd2pd->Delete();

        // We want to preserve knowledge of ghost zones
        vtkDataArray *ghosts = inDS->GetCellData()->GetArray("vtkGhostLevels");
        if (ghosts)
        {
            outDS->GetCellData()->AddArray(ghosts);
            // Only want ghost cell-data, not ghost point-data.
            outDS->GetPointData()->RemoveArray("vtkGhostLevels");
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
    else if (centeringInstruction == 2 && centering == AVT_NODECENT)
    {
        //
        //  User requested zone-centered but our data is node-centered,
        //  create the cell data from point data.
        //
        vtkPointDataToCellData *pd2cd = vtkPointDataToCellData::New();
     
        pd2cd->SetInput(inDS);
        pd2cd->SetOutput(outDS);
        pd2cd->Update();
        pd2cd->Delete();

        // We want to preserve knowledge of ghost zones
        vtkDataArray *ghosts = inDS->GetCellData()->GetArray("vtkGhostLevels");
        if (ghosts)
        {
            outDS->GetCellData()->AddArray(ghosts);
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
// ****************************************************************************

void
avtShiftCenteringFilter::RefashionDataObjectInfo(void)
{
    if (centeringInstruction == 1 || centeringInstruction == 2)
    {
        avtCentering centering = (centeringInstruction == 1 ? AVT_NODECENT
                                                            : AVT_ZONECENT);
        GetOutput()->GetInfo().GetAttributes().SetCentering(centering);
    }
}


