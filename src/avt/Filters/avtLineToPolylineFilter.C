/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                          avtLineToPolylineFilter.C                        //
// ************************************************************************* //

#include <avtLineToPolylineFilter.h>
#include <map>
#include <set>
#include <deque>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>
#include <TimingsManager.h>

// ****************************************************************************
//  Method: avtLineToPolylineFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
//
// ****************************************************************************

avtLineToPolylineFilter::avtLineToPolylineFilter() : avtDataTreeIterator()
{
}

// ****************************************************************************
//  Method: avtLineToPolylineFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
//
// ****************************************************************************

avtLineToPolylineFilter::~avtLineToPolylineFilter()
{
}

// ****************************************************************************
//  Method: avtLineToPolylineFilter::ExecuteData
//
//  Purpose:
//      Groups connected line cells into polyline cells.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output polydata.
//
//  Note: The cell data copying is untested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
//
//  Modifications:
//
// ****************************************************************************

struct edge
{
    edge() : first(0), second(0), cellid(0) { }
    edge(vtkIdType a, vtkIdType b, vtkIdType c) : first(a), second(b), cellid(c) { }

    bool operator < (const edge &obj) const
    {
        bool retval = false;
        if(first < obj.first)
            retval = true;
        else if(first == obj.first)
        {
            if(second < obj.second)
                retval = true;
            else if(second == obj.second)
            {
                if(cellid < obj.cellid)
                    retval = true;
            }
        }

        return retval;
    }

    bool operator == (const edge &obj) const
    {
        return first == obj.first &&
               second == obj.second &&
               cellid == obj.cellid;
    }

    vtkIdType first, second, cellid;
};

vtkDataSet *
avtLineToPolylineFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        // We only work on line data
        EXCEPTION1(VisItException, "avtLineToPolylineFilter::ExecuteDataTree "
                                   "-- Did not get polydata");
    }

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1)
    {
        return inDS;
    }

    int total = visitTimer->StartTimer();
    vtkPolyData  *input = (vtkPolyData *)inDS;
    vtkPolyData  *output = vtkPolyData::New();
    output->DeepCopy(input);
    vtkCellArray *lines = vtkCellArray::New();
    lines->EstimateSize(input->GetLines()->GetSize(), 1);
    output->SetLines(lines);
    lines->Delete();

    vtkCellData *inCD = input->GetCellData();
    vtkCellData *outCD  = output->GetCellData();

    // Copy the vert cell data
    for(vtkIdType i = 0; i < input->GetVerts()->GetNumberOfCells(); ++i)
        outCD->CopyData(inCD, i, i);

    //
    // Iterate over the lines and pick out all the single line segments
    // and add them to the free edges list. We just add the polylines
    // straight away.
    //
    input->GetLines()->InitTraversal();
    vtkIdType n, *pts = 0;
    std::set<edge> freeEdges;
    vtkIdType toCellId = input->GetVerts()->GetNumberOfCells();
    for(vtkIdType cellid = 0; input->GetLines()->GetNextCell(n, pts); ++cellid)
    {
        vtkIdType fromCellId = cellid + input->GetVerts()->GetNumberOfCells();
        if(n == 2)
        {
            edge e01(pts[0], pts[1], fromCellId);
            freeEdges.insert(e01);
        }
        else
        {
            // Copy in the polyline
            outCD->CopyData(inCD, fromCellId, toCellId++);
            output->InsertNextCell(VTK_POLY_LINE, n, pts);
        }
    }

    int grouping = visitTimer->StartTimer();
    int lineOffset = input->GetVerts()->GetNumberOfCells();
    int ptsBufSize = 200;
    pts = new vtkIdType[ptsBufSize];
    while(freeEdges.size() > 0)
    {
        std::deque<vtkIdType> shape;

        // Get seed edge and remove it from the pool
        edge currentEdge;
        if(freeEdges.begin() != freeEdges.end())
        {
            currentEdge = *freeEdges.begin();
            freeEdges.erase(freeEdges.begin());
        }

        vtkIdType firstCellInLine = currentEdge.cellid;

        // Add the current edge to the poly line.
        shape.push_back(currentEdge.first);
        shape.push_back(currentEdge.second);

        bool found = false;
        do
        {
            found = false; 
            for(std::set<edge>::iterator pos = freeEdges.begin(); 
                pos != freeEdges.end() && !found; ++pos)
            {
                if(currentEdge.first == pos->first)
                {
                    currentEdge.first = pos->second;
                    shape.push_front(pos->second);
                    freeEdges.erase(pos);
                    found = true;
                }
                else if(currentEdge.first == pos->second)
                {
                    currentEdge.first = pos->first;
                    shape.push_front(pos->first);
                    freeEdges.erase(pos);
                    found = true;
                }
                else if(currentEdge.second == pos->first)
                {
                    currentEdge.second = pos->second;
                    shape.push_back(pos->second);
                    freeEdges.erase(pos);
                    found = true;
                }
                else if(currentEdge.second == pos->second)
                {
                    currentEdge.second = pos->first;
                    shape.push_back(pos->first);
                    freeEdges.erase(pos);
                    found = true;
                }
            }
        } while(found);

        if(shape.size() > 2)
        {
            // We have a connected shape, add it to the new cells
            if(shape.size() > ptsBufSize)
            {
                delete [] pts;
                ptsBufSize = shape.size();
                pts = new vtkIdType[ptsBufSize];
            }
            for(size_t i = 0; i < shape.size(); ++i)
                pts[i] = shape[i];
            outCD->CopyData(inCD, firstCellInLine, toCellId++);
            output->InsertNextCell(VTK_POLY_LINE, shape.size(), pts);
            shape.clear();
        }
        else if(shape.size() == 2)
        {
            // We have a connected shape, add it to the new cells
            vtkIdType pts2[2];
            pts2[0] = shape[0];
            pts2[1] = shape[1];
            outCD->CopyData(inCD, firstCellInLine, toCellId++);
            output->InsertNextCell(VTK_LINE, shape.size(), pts2);
            shape.clear();
        }
    }
    delete [] pts;
    visitTimer->StopTimer(grouping, "Grouping into polylines");

    // Copy the polys cell data
    int fromPolyOffset = input->GetVerts()->GetNumberOfCells() + 
                         input->GetLines()->GetNumberOfCells();
    for(int i = 0; i < input->GetPolys()->GetNumberOfCells(); ++i)
        outCD->CopyData(inCD, fromPolyOffset + i, toCellId++);

    // Copy strip cell data.
    int fromTSOffset = fromPolyOffset + 
                       input->GetStrips()->GetNumberOfCells();
    for(int i = 0; i < input->GetStrips()->GetNumberOfCells(); ++i)
        outCD->CopyData(inCD, fromTSOffset + i, toCellId++);

    ManageMemory(output);
    output->Delete();

    visitTimer->StopTimer(total, "avtLineToPolylineFilter::ExecuteData");

    return output;
}


// ****************************************************************************
//  Method: avtLineToPolylineFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep  3 11:45:19 PDT 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtLineToPolylineFilter::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
        GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


