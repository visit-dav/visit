// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtLineToPolylineFilter.C                        //
// ************************************************************************* //

#include <avtLineToPolylineFilter.h>

#include <visit-config.h> // For LIB_VERSION_LE

#include <vtkCellArray.h>
#include <vtkCellData.h>
#if LIB_VERSION_GE(VTK, 9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkDataSet.h>
#include <vtkPolyData.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>
#include <TimingsManager.h>

#include <set>
#include <deque>

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
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
//
//  Note: The cell data copying is untested.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Aug 27 11:53:59 PDT 2009
//
//  Modifications:
//
//    Tom Fogal, Mon Apr 26 17:27:44 MDT 2010
//    Break out of a loop to prevent incrementing a singular iterator.
//    Use `empty' instead of 'size'.
//
//    Eric Brugger, Mon Jul 21 13:51:51 PDT 2014
//    Modified the class to work with avtDataRepresentation.
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

avtDataRepresentation *
avtLineToPolylineFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        // We only work on line data
        EXCEPTION1(VisItException, "avtLineToPolylineFilter::ExecuteDataTree "
                                   "-- Did not get polydata");
    }

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 1)
    {
        return inDR;
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
    vtkCellData *outCD = output->GetCellData();

    // Copy the vert cell data
    for(vtkIdType i = 0; i < input->GetVerts()->GetNumberOfCells(); ++i)
        outCD->CopyData(inCD, i, i);

    //
    // Iterate over the lines and pick out all the single line segments
    // and add them to the free edges list. We just add the polylines
    // straight away.
    //
    vtkCellArray *inlines = input->GetLines();
    vtkIdType n = 0;
    std::set<edge> freeEdges;
    const vtkIdType numVerts = input->GetVerts()->GetNumberOfCells();
    vtkIdType toCellId = numVerts;
#if LIB_VERSION_LE(VTK,8,1,0)
    inlines->InitTraversal();
    vtkIdType *cellPts = nullptr;
    for(vtkIdType cellid = 0; inlines->GetNextCell(n, cellPts); ++cellid)
    {
#else
    const vtkIdType *cellPts = nullptr;
    vtkIdType cellid = 0;
    auto iter = vtk::TakeSmartPointer(inlines->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell(), cellid++)
    {
        iter->GetCurrentCell(n, cellPts);
#endif
        vtkIdType fromCellId = cellid + numVerts;
        if(n == 2)
        {
            edge e01(cellPts[0], cellPts[1], fromCellId);
            freeEdges.insert(e01);
        }
        else
        {
            // Copy in the polyline
            outCD->CopyData(inCD, fromCellId, toCellId++);
            output->InsertNextCell(VTK_POLY_LINE, n, cellPts);
        }
    }

    int grouping = visitTimer->StartTimer();
    int ptsBufSize = 200;
    vtkIdType *pts = new vtkIdType[ptsBufSize];
    while(!freeEdges.empty())
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
                    break;
                }
                else if(currentEdge.first == pos->second)
                {
                    currentEdge.first = pos->first;
                    shape.push_front(pos->first);
                    freeEdges.erase(pos);
                    found = true;
                    break;
                }
                else if(currentEdge.second == pos->first)
                {
                    currentEdge.second = pos->second;
                    shape.push_back(pos->second);
                    freeEdges.erase(pos);
                    found = true;
                    break;
                }
                else if(currentEdge.second == pos->second)
                {
                    currentEdge.second = pos->first;
                    shape.push_back(pos->first);
                    freeEdges.erase(pos);
                    found = true;
                    break;
                }
            }
        } while(found);

        if(shape.size() > 2)
        {
            // We have a connected shape, add it to the new cells
            if(shape.size() > (size_t)ptsBufSize)
            {
                delete [] pts;
                ptsBufSize = (int)shape.size();
                pts = new vtkIdType[ptsBufSize];
            }
            for(size_t i = 0; i < shape.size(); ++i)
                pts[i] = shape[i];
            outCD->CopyData(inCD, firstCellInLine, toCellId++);
            output->InsertNextCell(VTK_POLY_LINE, (int)shape.size(), pts);
            shape.clear();
        }
        else if(shape.size() == 2)
        {
            // We have a connected shape, add it to the new cells
            vtkIdType pts2[2];
            pts2[0] = shape[0];
            pts2[1] = shape[1];
            outCD->CopyData(inCD, firstCellInLine, toCellId++);
            output->InsertNextCell(VTK_LINE, (int)shape.size(), pts2);
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

    visitTimer->StopTimer(total, "avtLineToPolylineFilter::ExecuteData");

    avtDataRepresentation *outDR= new avtDataRepresentation(output,
        inDR->GetDomain(), inDR->GetLabel());

    output->Delete();

    return outDR;
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


