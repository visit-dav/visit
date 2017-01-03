/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                           vtkSurfaceFromVolume.C                          //
// ************************************************************************* //

#include <vtkSurfaceFromVolume.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <vtkAccessors.h>

vtkSurfaceFromVolume::TriangleList::TriangleList()
{
    listSize = 4096;
    trianglesPerList = 1024;
 
    list = new vtkIdType*[listSize];
    list[0] = new vtkIdType[4*trianglesPerList];
    for (vtkIdType i = 1 ; i < listSize ; i++)
        list[i] = NULL;
 
    currentList = 0;
    currentTriangle = 0;
}
 

vtkSurfaceFromVolume::TriangleList::~TriangleList()
{
    for (int i = 0 ; i < listSize ; i++)
    {
        if (list[i] != NULL)
            delete [] list[i];
        else
            break;
    }
    delete [] list;
}
 
 
int
vtkSurfaceFromVolume::TriangleList::GetList(int listId, const vtkIdType *&outlist)
    const
{
    if (listId < 0 || listId > currentList)
    {
        outlist = NULL;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentTriangle : trianglesPerList);
}
 
 
vtkIdType
vtkSurfaceFromVolume::TriangleList::GetNumberOfLists(void) const
{
    return currentList+1;
}


vtkIdType
vtkSurfaceFromVolume::TriangleList::GetTotalNumberOfTriangles(void) const
{
    vtkIdType numFullLists = currentList;  // actually currentList-1+1
    vtkIdType numExtra = currentTriangle;  // again, currentTriangle-1+1
 
    return numFullLists*trianglesPerList + numExtra;
}
 
 
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Oct 21 15:32:07 PDT 2004
//    Fix bug when we get low on memory.
//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  Initialized new entries.
//
// ****************************************************************************

void
vtkSurfaceFromVolume::TriangleList::AddTriangle(vtkIdType cellId, 
    vtkIdType v1, vtkIdType v2, vtkIdType v3)
{
    if (currentTriangle >= trianglesPerList)
    {
        if ((currentList+1) >= listSize)
        {
            vtkIdType **tmpList = new vtkIdType*[2*listSize];
            for (vtkIdType i = 0 ; i < listSize ; i++)
                tmpList[i] = list[i];
            for (vtkIdType i = listSize ; i < listSize*2 ; i++)
                tmpList[i] = NULL;

            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new vtkIdType[4*trianglesPerList];
        currentTriangle = 0;
    }
 
    vtkIdType idx = 4*currentTriangle;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    currentTriangle++;
}

// ****************************************************************************
//  Modications:
//    Brad Whitlock, Mon Mar 12 16:15:37 PDT 2012
//    I separated this code out from ConstructPolyData into its own function so 
//    I could parameterize the point access code using a "point getter"
//    object. This code gets inlined in the ConstructPolyData methods so we
//    can support multiple coordinate data types.
//
// ****************************************************************************

template <typename PointGetter>
inline void
ConstructPolyDataHelper(vtkPointData *inPD, vtkCellData *inCD,
     vtkPolyData *output, const vtkSurfaceFromVolume::PointList &pt_list,
     const vtkSurfaceFromVolume::TriangleList &tris,
     int dataType, const PointGetter &pointGetter)
{
    vtkIdType   i, j;

    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    vtkIntArray *newOrigNodes = NULL;
    vtkIntArray *origNodes = vtkIntArray::SafeDownCast(
              inPD->GetArray("avtOriginalNodeNumbers"));

    //
    // Set up the output points and its point data.
    //
    vtkPoints *outPts = vtkPoints::New(dataType);
    vtkIdType nOutPts = pt_list.GetTotalNumberOfPoints();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD, nOutPts);
    if (origNodes != NULL)
    {
        newOrigNodes = vtkIntArray::New();
        newOrigNodes->SetNumberOfComponents(origNodes->GetNumberOfComponents());
        newOrigNodes->SetNumberOfTuples(nOutPts);
        newOrigNodes->SetName(origNodes->GetName());
    }
    vtkIdType nLists = pt_list.GetNumberOfLists();
    vtkIdType ptIdx = 0;
    for (i = 0 ; i < nLists ; i++)
    {
        const vtkDataSetFromVolume::PointEntry *pe_list = NULL;
        vtkIdType nPts = pt_list.GetList(i, pe_list);
        for (j = 0 ; j < nPts ; j++)
        {
            const vtkDataSetFromVolume::PointEntry &pe = pe_list[j];
            double pt[3], pt1[3], pt2[3];
            pointGetter.GetPoint(pe.ptIds[0], pt1);
            pointGetter.GetPoint(pe.ptIds[1], pt2);
            double p  = pe.percent;
            double bp = 1. - p;
            pt[0] = pt1[0]*p + pt2[0]*bp;
            pt[1] = pt1[1]*p + pt2[1]*bp;
            pt[2] = pt1[2]*p + pt2[2]*bp;
            outPts->SetPoint(ptIdx, pt);
            outPD->InterpolateEdge(inPD, ptIdx, pe.ptIds[0], pe.ptIds[1], bp);
            if (newOrigNodes)
            {
                vtkIdType id = (bp <= 0.5 ? pe.ptIds[0] : pe.ptIds[1]);
                newOrigNodes->SetTuple(ptIdx, origNodes->GetTuple(id));
            }
            ptIdx++;
        }
    }
    output->SetPoints(outPts);
    outPts->Delete();
    if (newOrigNodes)
    {
        // AddArray will overwrite an already existing array with 
        // the same name, exactly what we want here.
        outPD->AddArray(newOrigNodes);
        newOrigNodes->Delete();
    }

    //
    // Now set up the triangles and the cell data.
    //
    vtkIdType ntris = tris.GetTotalNumberOfTriangles();
    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(3*ntris + ntris);
    vtkIdType *nl = nlist->GetPointer(0);

    outCD->CopyAllocate(inCD, ntris);
    vtkIdType cellId = 0;
    vtkIdType nlists = tris.GetNumberOfLists();
    for (i = 0 ; i < nlists ; i++)
    {
        const vtkIdType *list;
        vtkIdType listSize = tris.GetList(i, list);
        for (j = 0 ; j < listSize ; j++)
        {
            outCD->CopyData(inCD, list[0], cellId);
            *nl++ = 3;
            *nl++ = list[1];
            *nl++ = list[2];
            *nl++ = list[3];
            list += 4;
            cellId++;
        }
    }
    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(ntris, nlist);
    nlist->Delete();

    output->SetPolys(cells);
    cells->Delete();
}

// ****************************************************************************
//  Modications:
//
//    Hank Childs, Fri Jan 30 08:50:44 PST 2004
//    Speed up construction by doing pointer arithmetic.
//
//    Kathleen Bonnell, Fri Nov 12 16:43:11 PST 2004 
//    Don't interoplate avtOriginalNodeNumbers, instead use the closest
//    point to the slice point.
//
//    Brad Whitlock, Thu Jul 23 16:01:46 PDT 2015
//    Support for non-standard memory layout.
//
// ****************************************************************************

void
vtkSurfaceFromVolume::ConstructPolyData(vtkPointData *inPD, vtkCellData *inCD,
                                        vtkPolyData *output, vtkPoints *pts)
{
    if(pts->GetData()->HasStandardMemoryLayout())
    {
        if(pts->GetDataType() == VTK_FLOAT)
            ConstructPolyDataHelper(inPD, inCD, output, this->pt_list, this->tris, pts->GetDataType(), vtkPointAccessor<float>(pts));
        else if(pts->GetDataType() == VTK_FLOAT)
            ConstructPolyDataHelper(inPD, inCD, output, this->pt_list, this->tris, pts->GetDataType(), vtkPointAccessor<double>(pts));
        else
            ConstructPolyDataHelper(inPD, inCD, output, this->pt_list, this->tris, pts->GetDataType(), vtkGeneralPointAccessor(pts));
    }
    else
    {
        ConstructPolyDataHelper(inPD, inCD, output, this->pt_list, this->tris, pts->GetDataType(), vtkGeneralPointAccessor(pts));
    }
}

// ****************************************************************************
//  Modications:
//
//    Hank Childs, Fri Jan 30 08:50:44 PST 2004
//    Speed up construction by doing pointer arithmetic.
//
//    Kathleen Bonnell, Tue Nov 23 14:07:10 PST 2004 
//    Don't interoplate avtOriginalNodeNumbers, instead use the closest
//    point to the slice point.
//
//    Brad Whitlock, Mon Mar 12 16:09:19 PDT 2012
//    I moved indexing code into "point accessor" classes and added support
//    for different coordinate types.
//
// ****************************************************************************

void
vtkSurfaceFromVolume::ConstructPolyData(vtkPointData *inPD, vtkCellData *inCD,
    vtkPolyData *output, int *dims, 
    vtkDataArray *X, vtkDataArray *Y, vtkDataArray *Z)
{
    int tx = X->GetDataType();
    int ty = Y->GetDataType();
    int tz = Z->GetDataType();
    bool same = tx == ty && ty == tz;
    if(same && tx == VTK_FLOAT)
        ConstructPolyDataHelper(inPD, inCD, output, this->pt_list, this->tris, tx, vtkRectPointAccessor<float>(dims, X, Y, Z));
    else if(same && tx == VTK_DOUBLE)
        ConstructPolyDataHelper(inPD, inCD, output, this->pt_list, this->tris, tx, vtkRectPointAccessor<double>(dims, X, Y, Z));
    else
        ConstructPolyDataHelper(inPD, inCD, output, this->pt_list, this->tris, tx, vtkGeneralRectPointAccessor(dims, X, Y, Z));
}
