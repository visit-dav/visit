/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtGhostZoneFilter.C                           //
// ************************************************************************* //

#include <avtGhostZoneFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>

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
//  Modifications:
//
//    Hank Childs, Wed Dec 20 09:25:42 PST 2006
//    Initialize ghostDataMustBeRemoved.
//
// ****************************************************************************

avtGhostZoneFilter::avtGhostZoneFilter()
{
    ghostDataMustBeRemoved = false;
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
//    Hank Childs, Thu Mar  2 14:15:29 PST 2006
//    Change the way we access the vtkDataSetRemoveGhostCells filter, since
//    it can change type of output.
//
//    Hank Childs, Tue Dec 19 09:52:33 PST 2006
//    Allow rectilinear grids to pass through.
//
//    Hank Childs, Wed Dec 27 10:14:27 PST 2006
//    Allow curvilinear grids to pass through.
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

    //
    // Check to see if the data is all ghost.  If so, then we don't need
    // to go any further.
    //
    if (haveGhostZones)
    {
        vtkUnsignedCharArray *ghost_zones = (vtkUnsignedCharArray *)
                               in_ds->GetCellData()->GetArray("avtGhostZones");
        unsigned char *gz = ghost_zones->GetPointer(0);
        bool allGhost = true;
        const int nCells = in_ds->GetNumberOfCells();
        for (int i = 0 ; i < nCells ; i++)
        {
            if (gz[i] == '\0')
            {
                allGhost = false;
                break;
            }
        }

        if (allGhost)
        {
            debug5 << "Domain " << domain << " contains only ghosts.  Removing"
                   << endl;
            return NULL;
        }
    }
    if (haveGhostNodes)
    {
        vtkUnsignedCharArray *ghost_nodes = (vtkUnsignedCharArray *)
                               in_ds->GetPointData()->GetArray("avtGhostNodes");
        unsigned char *gn = ghost_nodes->GetPointer(0);
        bool allGhost = true;
        const int nCells = in_ds->GetNumberOfCells();
        for (int i = 0 ; i < nCells ; i++)
        {
            if (gn[i] == '\0')
            {
                allGhost = false;
                break;
            }
        }

        if (allGhost)
        {
            debug5 << "Domain " << domain << " contains only ghosts.  Removing"
                   << endl;
            return NULL;
        }
    }

    if (in_ds->GetDataObjectType() == VTK_RECTILINEAR_GRID && 
        !ghostDataMustBeRemoved)
    {
        debug5 << "Allow rectilinear grid to travel through with ghost data;"
               << " depending on mapper to remove ghost data during render." 
               << endl;
        return in_ds;
    }

    if (in_ds->GetDataObjectType() == VTK_STRUCTURED_GRID && 
        !ghostDataMustBeRemoved)
    {
        debug5 << "Allow structured grid to travel through with ghost data;"
               << " depending on mapper to remove ghost data during render." 
               << endl;
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
    // Note: have to say "Update" before saying GetOutput, since output
    // may change during execution.
    //
    filter->Update();
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


