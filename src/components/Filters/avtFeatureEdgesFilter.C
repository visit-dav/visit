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
//                          avtFeatureEdgesFilter.C                          //
// ************************************************************************* //

#include <avtFeatureEdgesFilter.h>

#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkVisItFeatureEdges.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>


using     std::string;


// ****************************************************************************
//  Method: avtFeatureEdgesFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtFeatureEdgesFilter::avtFeatureEdgesFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtFeatureEdgesFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtFeatureEdgesFilter::~avtFeatureEdgesFilter()
{
    ;
}


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
//    Hank Childs, Wed Oct 15 21:19:00 PDT 2003
//    Improved for the single cell case -- VTK feature edges was handling
//    poorly.
//
//    Hank Childs, Tue Aug 31 08:40:44 PDT 2004
//    The VTK feature edges filter removes edges that have "vtkGhostLevels".
//    But we now use "avtGhostZones" to indicate this.  So we need to remove
//    the ghost zones ourselves to do what used to be done from within the
//    feature edges filter.
//
//    Hank Childs, Thu Mar 31 11:16:23 PST 2005
//    Fix problem with single cell case ['6105].
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

    vtkDataSet *outDS = NULL;

    if (inDS->GetNumberOfCells() == 1)
    {
        //
        // VTK doesn't do a good job with 1-cell feature edges, so just do it
        // ourselves.
        //
        int  i, j;

        vtkCell *cell = inDS->GetCell(0);
        vtkPolyData  *output = vtkPolyData::New();
        vtkCellData  *inCD   = inDS->GetCellData();
        vtkPointData *inPD   = inDS->GetPointData();
        vtkCellData  *outCD  = output->GetCellData();
        vtkPointData *outPD  = output->GetPointData();
        
        vtkIdList *ids = cell->GetPointIds();
        int npts = ids->GetNumberOfIds();
        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(npts);
        outPD->CopyAllocate(inPD, npts);
        std::vector<int> origId(npts, -1);
        for (i = 0 ; i < npts ; i++)
        {
             origId[i] = ids->GetId(i);
             outPD->CopyData(inPD, ids->GetId(i), i);
             double pt[3];
             inDS->GetPoint(ids->GetId(i), pt);
             pts->SetPoint(i, pt);
        }
        int ncells = cell->GetNumberOfEdges();
        outCD->CopyAllocate(inCD, ncells);
        vtkCellArray *lines = vtkCellArray::New();
        lines->Allocate(ncells*(2+1));
        for (i = 0 ; i < ncells ; i++)
        {
             outCD->CopyData(inCD, 0, i);
             vtkCell *edge = cell->GetEdge(i);
             vtkIdList *edge_ids = edge->GetPointIds();
             int line[2];
             int origId0 = edge_ids->GetId(0);
             int origId1 = edge_ids->GetId(1);
             int newId0 = 0, newId1 = 0;
             for (j = 0 ; j < npts ; j++)
             {
                 if (origId0 == origId[j])
                     newId0 = j;
                 if (origId1 == origId[j])
                     newId1 = j;
             }
             line[0] = newId0;
             line[1] = newId1;
             lines->InsertNextCell(2, line);
        }

        output->SetPoints(pts);
        pts->Delete();
        output->SetLines(lines);
        lines->Delete();

        ManageMemory(output);
        output->Delete();
        outDS = output;
    }
    else
    {
        //
        // Set up and apply the filter
        //
        vtkVisItFeatureEdges *featureEdgesFilter = vtkVisItFeatureEdges::New();
        featureEdgesFilter->SetInput((vtkPolyData*)inDS);
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

        vtkDataSet *output = featureEdgesFilter->GetOutput();
        output->Update();

        if (output->GetNumberOfCells() > 0)
        {
            outDS = output;
        }
        ManageMemory(outDS);
        featureEdgesFilter->Delete();
    }

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


