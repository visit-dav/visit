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
//                           vtkVolumeFromVolume.C                          //
// ************************************************************************* //

#include <vtkVolumeFromVolume.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <vtkAccessors.h>

// ****************************************************************************
//  Method: vtkVolumeFromVolume constructor
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2004
//
//  Modifications:
//
//    Hank Childs, Sun Oct 24 21:05:03 PDT 2004
//    Change ordering of shapes back to the way it was before all of the 
//    shapes were put in a vector.  This way we won't have to update baseline
//    images.
//
//    Jeremy Meredith, Tue Aug 29 15:47:28 EDT 2006
//    Added lines and vertices.
//
// ****************************************************************************

vtkVolumeFromVolume::vtkVolumeFromVolume(vtkIdType nPts, vtkIdType ptSizeGuess)
    : vtkDataSetFromVolume(nPts, ptSizeGuess), nshapes(8)
{
    shapes[0] = &tets;
    shapes[1] = &pyramids;
    shapes[2] = &wedges;
    shapes[3] = &hexes;
    shapes[4] = &quads;
    shapes[5] = &tris;
    shapes[6] = &lines;
    shapes[7] = &vertices;
}

vtkVolumeFromVolume::CentroidPointList::CentroidPointList()
{
    listSize = 4096;
    pointsPerList = 1024;
 
    list = new CentroidPointEntry*[listSize];
    list[0] = new CentroidPointEntry[pointsPerList];
    for (vtkIdType i = 1 ; i < listSize ; i++)
        list[i] = NULL;
 
    currentList = 0;
    currentPoint = 0;
}
 
 
vtkVolumeFromVolume::CentroidPointList::~CentroidPointList()
{
    for (vtkIdType i = 0 ; i < listSize ; i++)
    {
        if (list[i] != NULL)
            delete [] list[i];
        else
            break;
    }
    delete [] list;
}


void
vtkVolumeFromVolume::CentroidPointList::Clear()
{
    for (vtkIdType i = 0 ; i < listSize ; i++)
    {
        if (list[i] != NULL)
        {
            delete [] list[i];
            list[i] = NULL;
        }
        else
            break;
    }
    list[0] = new CentroidPointEntry[pointsPerList];

    currentList = 0;
    currentPoint = 0;
}


vtkIdType
vtkVolumeFromVolume::CentroidPointList::GetList(vtkIdType listId,
                                    const CentroidPointEntry *&outlist) const
{
    if (listId < 0 || listId > currentList)
    {
        outlist = NULL;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentPoint : pointsPerList);
}
 
 
vtkIdType
vtkVolumeFromVolume::CentroidPointList::GetNumberOfLists(void) const
{
    return currentList+1;
}
 
 
vtkIdType
vtkVolumeFromVolume::CentroidPointList::GetTotalNumberOfPoints(void) const
{
    vtkIdType numFullLists = currentList;  // actually currentList-1+1
    vtkIdType numExtra = currentPoint;  // again, currentPoint-1+1
 
    return numFullLists*pointsPerList + numExtra;
}


//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//
//    Sean Ahern, Mon Mar  5 15:35:00 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
vtkIdType
vtkVolumeFromVolume::CentroidPointList::AddPoint(vtkIdType npts, const vtkIdType *pts)
{
    if (currentPoint >= pointsPerList)
    {
        if ((currentList+1) >= listSize)
        {
            CentroidPointEntry **tmpList = new CentroidPointEntry*[2*listSize];
            for (vtkIdType i = 0 ; i < listSize ; i++)
                tmpList[i] = list[i];
            for (vtkIdType i = listSize ; i < listSize*2 ; i++)
                tmpList[i] = NULL;
            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new CentroidPointEntry[pointsPerList];
        currentPoint = 0;
    }
 
    list[currentList][currentPoint].nPts = npts;
    for (vtkIdType i = 0 ; i < npts ; i++)
    {
        list[currentList][currentPoint].ptIds[i] = pts[i];
    }
    currentPoint++;
 
    return (GetTotalNumberOfPoints()-1);
}


vtkVolumeFromVolume::ShapeList::ShapeList(vtkIdType size)
{
    shapeSize = size;
    listSize = 4096;
    shapesPerList = 1024;
 
    list = new vtkIdType*[listSize];
    list[0] = new vtkIdType[(shapeSize+1)*shapesPerList];
    for (vtkIdType i = 1 ; i < listSize ; i++)
        list[i] = NULL;
 
    currentList = 0;
    currentShape = 0;
}
 
 
vtkVolumeFromVolume::ShapeList::~ShapeList()
{
    for (vtkIdType i = 0 ; i < listSize ; i++)
    {
        if (list[i] != NULL)
            delete [] list[i];
        else
            break;
    }
    delete [] list;
}
 
 
vtkIdType
vtkVolumeFromVolume::ShapeList::GetList(vtkIdType listId, const vtkIdType *&outlist)
    const
{
    if (listId < 0 || listId > currentList)
    {
        outlist = NULL;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentShape : shapesPerList);
}
 
 
vtkIdType
vtkVolumeFromVolume::ShapeList::GetList(vtkIdType listId, vtkIdType *&outlist)
    const
{
    if (listId < 0 || listId > currentList)
    {
        outlist = NULL;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentShape : shapesPerList);
}
 
 
vtkIdType
vtkVolumeFromVolume::ShapeList::GetNumberOfLists(void) const
{
    return currentList+1;
}


vtkIdType
vtkVolumeFromVolume::ShapeList::GetTotalNumberOfShapes(void) const
{
    vtkIdType numFullLists = currentList;  // actually currentList-1+1
    vtkIdType numExtra = currentShape;  // again, currentShape-1+1
 
    return numFullLists*shapesPerList + numExtra;
}

vtkVolumeFromVolume::HexList::HexList()
    : vtkVolumeFromVolume::ShapeList(8)
{
}
 

vtkVolumeFromVolume::HexList::~HexList()
{
}
 
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Oct 21 15:32:07 PDT 2004
//    Fix bug where we run out of memory.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//    
//    Sean Ahern, Mon Mar  5 15:43:19 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
// ****************************************************************************

void
vtkVolumeFromVolume::HexList::AddHex(vtkIdType cellId,
    vtkIdType v1, vtkIdType v2, vtkIdType v3, vtkIdType v4,
    vtkIdType v5, vtkIdType v6, vtkIdType v7, vtkIdType v8)
{
    if (currentShape >= shapesPerList)
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
        list[currentList] = new vtkIdType[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    vtkIdType idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    list[currentList][idx+5] = v5;
    list[currentList][idx+6] = v6;
    list[currentList][idx+7] = v7;
    list[currentList][idx+8] = v8;
    currentShape++;
}

vtkVolumeFromVolume::WedgeList::WedgeList()
    : vtkVolumeFromVolume::ShapeList(6)
{
}
 

vtkVolumeFromVolume::WedgeList::~WedgeList()
{
}
 
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Oct 21 15:32:07 PDT 2004
//    Fix bug where we run out of memory.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
// ****************************************************************************

void
vtkVolumeFromVolume::WedgeList::AddWedge(vtkIdType cellId,
    vtkIdType v1, vtkIdType v2, vtkIdType v3,
    vtkIdType v4, vtkIdType v5, vtkIdType v6)
{
    if (currentShape >= shapesPerList)
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
        list[currentList] = new vtkIdType[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    vtkIdType idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    list[currentList][idx+5] = v5;
    list[currentList][idx+6] = v6;
    currentShape++;
}

vtkVolumeFromVolume::PyramidList::PyramidList()
    : vtkVolumeFromVolume::ShapeList(5)
{
}
 

vtkVolumeFromVolume::PyramidList::~PyramidList()
{
}
 
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Oct 21 15:32:07 PDT 2004
//    Fix bug where we run out of memory.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
// ****************************************************************************

void
vtkVolumeFromVolume::PyramidList::AddPyramid(vtkIdType cellId,
    vtkIdType v1, vtkIdType v2, vtkIdType v3, vtkIdType v4, vtkIdType v5)
{
    if (currentShape >= shapesPerList)
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
        list[currentList] = new vtkIdType[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    vtkIdType idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    list[currentList][idx+5] = v5;
    currentShape++;
}

vtkVolumeFromVolume::TetList::TetList()
    : vtkVolumeFromVolume::ShapeList(4)
{
}
 

vtkVolumeFromVolume::TetList::~TetList()
{
}
 
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Oct 21 15:32:07 PDT 2004
//    Fix bug where we run out of memory.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
// ****************************************************************************

void
vtkVolumeFromVolume::TetList::AddTet(vtkIdType cellId,
    vtkIdType v1,vtkIdType v2,vtkIdType v3,vtkIdType v4)
{
    if (currentShape >= shapesPerList)
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
        list[currentList] = new vtkIdType[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    vtkIdType idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    currentShape++;
}

vtkVolumeFromVolume::QuadList::QuadList()
    : vtkVolumeFromVolume::ShapeList(4)
{
}
 

vtkVolumeFromVolume::QuadList::~QuadList()
{
}
 
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Oct 21 15:32:07 PDT 2004
//    Fix bug where we run out of memory.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
// ****************************************************************************

void
vtkVolumeFromVolume::QuadList::AddQuad(vtkIdType cellId,
    vtkIdType v1,vtkIdType v2,vtkIdType v3,vtkIdType v4)
{
    if (currentShape >= shapesPerList)
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
        list[currentList] = new vtkIdType[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    vtkIdType idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    list[currentList][idx+4] = v4;
    currentShape++;
}

vtkVolumeFromVolume::TriList::TriList()
    : vtkVolumeFromVolume::ShapeList(3)
{
}
 

vtkVolumeFromVolume::TriList::~TriList()
{
}
 
// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu Oct 21 15:32:07 PDT 2004
//    Fix bug where we run out of memory.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
// ****************************************************************************

void
vtkVolumeFromVolume::TriList::AddTri(vtkIdType cellId,
    vtkIdType v1,vtkIdType v2,vtkIdType v3)
{
    if (currentShape >= shapesPerList)
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
        list[currentList] = new vtkIdType[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    vtkIdType idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    list[currentList][idx+3] = v3;
    currentShape++;
}


vtkVolumeFromVolume::LineList::LineList()
    : vtkVolumeFromVolume::ShapeList(2)
{
}
 

vtkVolumeFromVolume::LineList::~LineList()
{
}
 
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
void
vtkVolumeFromVolume::LineList::AddLine(vtkIdType cellId,
    vtkIdType v1,vtkIdType v2)
{
    if (currentShape >= shapesPerList)
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
        list[currentList] = new vtkIdType[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    vtkIdType idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    list[currentList][idx+2] = v2;
    currentShape++;
}


vtkVolumeFromVolume::VertexList::VertexList()
    : vtkVolumeFromVolume::ShapeList(1)
{
}
 

vtkVolumeFromVolume::VertexList::~VertexList()
{
}
 
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed off-by-one error in test to resize list
//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  You really do need the +1.
//    Initialized new entries.
//
void
vtkVolumeFromVolume::VertexList::AddVertex(vtkIdType cellId, vtkIdType v1)
{
    if (currentShape >= shapesPerList)
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
        list[currentList] = new vtkIdType[(shapeSize+1)*shapesPerList];
        currentShape = 0;
    }
 
    vtkIdType idx = (shapeSize+1)*currentShape;
    list[currentList][idx+0] = cellId;
    list[currentList][idx+1] = v1;
    currentShape++;
}

// ****************************************************************************
//  Method: ConstructDataSetHelper
//
//  Purpose:
//      Constructs the output dataset.
//
//  Notes:  The code for this function use to be located in the two functions
//          written by Jeremy Meredith.  I unified them here and made some
//          additional enhancements.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Oct 21 07:23:55 PDT 2004
//    Only create points in the output that will actually be referenced.  Also
//    generalized the shape list to reduce code.
//
//    Kathleen Bonnell, Mon May  1 08:50:46 PDT 2006 
//    Don't interpolate avtOriginalNodeNumbers, use value from closest node
//    instead. 
//
//    Mark C. Miller, Tue Nov  7 20:35:07 PST 2006
//    Through emperical analysis running the code and printing values, I
//    discovered that for centroid points, the point indices that were
//    getting assigned were too large by 1. Apparently, centroid points
//    are initially identified using negative point ids (see AddCentroidPoint)
//    starting from -1. This -1 offset was not being taken into account when
//    doing the index arithmetic for the cases involving centroid points.
//    There were two lines of code that were modified tagged with '-1 offset'
//    in-line comments. This corrected a slew of cases I discovered. However,
//    even after making this change, I have encountered some cases where I
//    still see the too large by 1 problem but with very low probablility of
//    occurance.
//    
//    Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//    Undid above change 
//
//    Brad Whitlock, Thu Mar 22 14:11:31 PDT 2012
//    I turned the method into a template helper function.
//
// ****************************************************************************

template <typename PointGetter>
void
ConstructDataSetHelper(vtkPointData *inPD, vtkCellData *inCD, vtkUnstructuredGrid *output,
    int dataType, vtkIdType numPrevPts, 
    vtkVolumeFromVolume::ShapeList *shapes[8], int nshapes,
    vtkVolumeFromVolume::PointList &pt_list,
    vtkVolumeFromVolume::CentroidPointList &centroid_list,
    const PointGetter &pointGetter)
{
    vtkPointData *outPD = output->GetPointData();
    vtkCellData  *outCD = output->GetCellData();

    vtkIntArray *newOrigNodes = NULL;
    vtkIntArray *origNodes = vtkIntArray::SafeDownCast(
              inPD->GetArray("avtOriginalNodeNumbers"));
    //
    // If the isovolume only affects a small part of the dataset, we can save
    // on memory by only bringing over the points from the original dataset
    // that are used with the output.  Determine which points those are here.
    //
    int *ptLookup = new int[numPrevPts];
    for (vtkIdType i = 0 ; i < numPrevPts ; i++)
        ptLookup[i] = -1;
    int numUsed = 0;
    for (int i = 0 ; i < nshapes ; i++)
    {
        vtkIdType nlists = shapes[i]->GetNumberOfLists();
        int npts_per_shape = shapes[i]->GetShapeSize();
        for (vtkIdType j = 0 ; j < nlists ; j++)
        {
            const vtkIdType *list;
            vtkIdType listSize = shapes[i]->GetList(j, list);
            for (vtkIdType k = 0 ; k < listSize ; k++)
            {
                list++; // skip the cell id entry
                for (vtkIdType l = 0 ; l < npts_per_shape ; l++)
                {
                    int pt = *list;
                    list++;
                    if (pt >= 0 && pt < numPrevPts)
                        if (ptLookup[pt] == -1)
                            ptLookup[pt] = numUsed++;
                }
            }
        }
    }

    //
    // Set up the output points and its point data.
    //
    vtkPoints *outPts = vtkPoints::New(dataType);
    vtkIdType centroidStart = numUsed + pt_list.GetTotalNumberOfPoints();
    vtkIdType nOutPts = centroidStart + centroid_list.GetTotalNumberOfPoints();
    outPts->SetNumberOfPoints(nOutPts);
    outPD->CopyAllocate(inPD, nOutPts);
    if (origNodes != NULL)
    {
        newOrigNodes = vtkIntArray::New();
        newOrigNodes->SetNumberOfComponents(origNodes->GetNumberOfComponents());
        newOrigNodes->SetNumberOfTuples(nOutPts);
        newOrigNodes->SetName(origNodes->GetName());
    }

    //
    // Copy over all the points from the input that are actually used in the
    // output.
    //
    for (vtkIdType i = 0 ; i < numPrevPts ; i++)
    {
        if (ptLookup[i] == -1)
            continue;

        double pt[3];
        pointGetter.GetPoint(i, pt);
        outPts->SetPoint(ptLookup[i], pt);

        outPD->CopyData(inPD, i, ptLookup[i]);
        if (newOrigNodes)
            newOrigNodes->SetTuple(ptLookup[i], origNodes->GetTuple(i));
    }
    vtkIdType ptIdx = numUsed;

    //
    // Now construct all the points that are along edges and new and add 
    // them to the points list.
    //
    vtkIdType nLists = pt_list.GetNumberOfLists();
    for (vtkIdType i = 0 ; i < nLists ; i++)
    {
        const vtkVolumeFromVolume::PointEntry *pe_list = NULL;
        vtkIdType nPts = pt_list.GetList(i, pe_list);
        for (vtkIdType j = 0 ; j < nPts ; j++)
        {
            const vtkVolumeFromVolume::PointEntry &pe = pe_list[j];
            double pt[3], pt1[3], pt2[3];

            pointGetter.GetPoint(pe.ptIds[0], pt1);
            pointGetter.GetPoint(pe.ptIds[1], pt2);

            // Now that we have the original points, calculate the new one.
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

    // 
    // Now construct the new "centroid" points and add them to the points list.
    //
    nLists = centroid_list.GetNumberOfLists();
    vtkIdList *idList = vtkIdList::New();
    for (vtkIdType i = 0 ; i < nLists ; i++)
    {
        const vtkVolumeFromVolume::CentroidPointEntry *ce_list = NULL;
        vtkIdType nPts = centroid_list.GetList(i, ce_list);
        for (vtkIdType j = 0 ; j < nPts ; j++)
        {
            const vtkVolumeFromVolume::CentroidPointEntry &ce = ce_list[j];
            idList->SetNumberOfIds(ce.nPts);
            double pts[8][3];
            double weights[8];
            double pt[3] = {0., 0., 0.};
            double weight_factor = 1. / ce.nPts;
            for (int k = 0 ; k < ce.nPts ; k++)
            {
                weights[k] = 1.0 * weight_factor;
                vtkIdType id = 0;
                if (ce.ptIds[k] < 0)
                    id = centroidStart-1 - ce.ptIds[k];
                else if (ce.ptIds[k] >= numPrevPts)
                    id = numUsed + (ce.ptIds[k] - numPrevPts);
                else
                    id = ptLookup[ce.ptIds[k]];
                idList->SetId(k, id);
                outPts->GetPoint(id, pts[k]);
                pt[0] += pts[k][0];
                pt[1] += pts[k][1];
                pt[2] += pts[k][2];
            }
            pt[0] *= weight_factor;
            pt[1] *= weight_factor;
            pt[2] *= weight_factor;

            outPts->SetPoint(ptIdx, pt);
            outPD->InterpolatePoint(outPD, ptIdx, idList, weights);
            if (newOrigNodes)
            {
                // these 'created' nodes have no original designation
                for (int z = 0; z < newOrigNodes->GetNumberOfComponents(); z++)
                    newOrigNodes->SetComponent(ptIdx, z, -1);
            }
            ptIdx++;
        }
    }
    idList->Delete();

    //
    // We are finally done constructing the points list.  Set it with our
    // output and clean up memory.
    //
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
    // Now set up the shapes and the cell data.
    //
    vtkIdType cellId = 0;
    vtkIdType nlists;

    vtkIdType ncells = 0;
    vtkIdType conn_size = 0;
    for (int i = 0 ; i < nshapes ; i++)
    {
        vtkIdType ns = shapes[i]->GetTotalNumberOfShapes();
        ncells += ns;
        conn_size += (shapes[i]->GetShapeSize()+1)*ns;
    }

    outCD->CopyAllocate(inCD, ncells);

    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(conn_size);
    vtkIdType *nl = nlist->GetPointer(0);

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(ncells);
    unsigned char *ct = cellTypes->GetPointer(0);

    vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
    cellLocations->SetNumberOfValues(ncells);
    vtkIdType *cl = cellLocations->GetPointer(0);

    vtkIdType ids[1024]; // 8 (for hex) should be max, but...
    vtkIdType current_index = 0;
    for (int i = 0 ; i < nshapes ; i++)
    {
        const vtkIdType *list;
        nlists = shapes[i]->GetNumberOfLists();
        int shapesize = shapes[i]->GetShapeSize();
        int vtk_type = shapes[i]->GetVTKType();
        for (vtkIdType j = 0 ; j < nlists ; j++)
        {
            int listSize = shapes[i]->GetList(j, list);
            for (vtkIdType k = 0 ; k < listSize ; k++)
            {
                outCD->CopyData(inCD, list[0], cellId);
                for (int l = 0 ; l < shapesize ; l++)
                {
                    if (list[l+1] < 0)
                        ids[l] = centroidStart-1 - list[l+1];
                    else if (list[l+1] >= numPrevPts)
                        ids[l] = numUsed + (list[l+1] - numPrevPts);
                    else
                        ids[l] = ptLookup[list[l+1]];
                }
                list += shapesize+1;
                *nl++ = shapesize;
                *cl++ = current_index;
                *ct++ = vtk_type;
                for (int l = 0 ; l < shapesize ; l++)
                    *nl++ = ids[l];
                current_index += shapesize+1;
                //output->InsertNextCell(vtk_type, shapesize, ids);
                cellId++;
            }
        }
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(ncells, nlist);
    nlist->Delete();

    output->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();

    delete [] ptLookup;
}

// ****************************************************************************
//  Modifications:
//    Jeremy Meredith, Mon Feb 16 19:11:34 PST 2004
//    Added polygonal cell support.
//
//    Hank Childs, Thu Oct 21 07:52:31 PDT 2004
//    Instead of duplicating code, just call a common function.
//    (Moved whole routine to new ConstructDataSet method with 
//     CommonPointsStructure in signature).
//
//    Brad Whitlock, Thu Mar 22 12:36:10 PDT 2012
//    Call ConstructDataSetHelper.
//
// ****************************************************************************
void
vtkVolumeFromVolume::ConstructDataSet(vtkPointData *inPD, vtkCellData *inCD,
                                      vtkUnstructuredGrid *output,
                                      vtkPoints *pts)
{
    if(pts->GetDataType() == VTK_FLOAT)
        ConstructDataSetHelper(inPD, inCD, output, VTK_FLOAT, this->numPrevPts,
                               this->shapes, this->nshapes, 
                               this->pt_list, this->centroid_list,
                               vtkPointAccessor<float>(pts));
    else if(pts->GetDataType() == VTK_DOUBLE)
        ConstructDataSetHelper(inPD, inCD, output, VTK_DOUBLE, this->numPrevPts,
                               this->shapes, this->nshapes, 
                               this->pt_list, this->centroid_list, 
                               vtkPointAccessor<double>(pts));
    else
        ConstructDataSetHelper(inPD, inCD, output, VTK_FLOAT, this->numPrevPts,
                               this->shapes, this->nshapes,
                               this->pt_list, this->centroid_list, 
                               vtkGeneralPointAccessor(pts));
}


// ****************************************************************************
//  Modifications:
//    Jeremy Meredith, Mon Feb 16 19:11:34 PST 2004
//    Added polygonal cell support.
//
//    Hank Childs, Thu Oct 21 07:52:31 PDT 2004
//    Instead of duplicating code, just call a common function.
//    (Moved whole routine to new ConstructDataSet method with 
//     CommonPointsStructure in signature).
//
//    Brad Whitlock, Thu Mar 22 12:36:10 PDT 2012
//    Call ConstructDataSetHelper.
//
// ****************************************************************************
void
vtkVolumeFromVolume::ConstructDataSet(vtkPointData *inPD, vtkCellData *inCD,
                                      vtkUnstructuredGrid *output,
                                      const int *dims, vtkDataArray *X, vtkDataArray *Y, vtkDataArray *Z)
{
    int tx = X->GetDataType();
    int ty = Y->GetDataType();
    int tz = Z->GetDataType();
    bool same = tx == ty && ty == tz;
    if(same && tx == VTK_FLOAT)
        ConstructDataSetHelper(inPD, inCD, output, VTK_FLOAT, this->numPrevPts,
                               this->shapes, this->nshapes, 
                               this->pt_list, this->centroid_list, 
                               vtkRectPointAccessor<float>(dims, X, Y, Z));
    else if(same && tx == VTK_DOUBLE)
        ConstructDataSetHelper(inPD, inCD, output, VTK_DOUBLE, this->numPrevPts, 
                               this->shapes, this->nshapes, 
                               this->pt_list, this->centroid_list, 
                               vtkRectPointAccessor<double>(dims, X, Y, Z));
    else
        ConstructDataSetHelper(inPD, inCD, output, VTK_FLOAT, this->numPrevPts,
                               this->shapes, this->nshapes, 
                               this->pt_list, this->centroid_list, 
                               vtkGeneralRectPointAccessor(dims, X, Y, Z));
}
