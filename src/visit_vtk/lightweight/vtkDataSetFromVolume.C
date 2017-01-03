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
//                           vtkDataSetFromVolume.C                          //
// ************************************************************************* //

#include <vtkDataSetFromVolume.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

using std::vector;




vtkDataSetFromVolume::PointList::PointList()
{
    listSize = 4096;
    pointsPerList = 1024;
 
    list = new PointEntry*[listSize];
    list[0] = new PointEntry[pointsPerList];
    for (int i = 1 ; i < listSize ; i++)
        list[i] = NULL;
 
    currentList = 0;
    currentPoint = 0;
}
 
 
vtkDataSetFromVolume::PointList::~PointList()
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


void
vtkDataSetFromVolume::PointList::Clear()
{
    for (int i = 0 ; i < listSize ; i++)
    {
        if (list[i] != NULL)
        {
            delete [] list[i];
            list[i] = NULL;
        }
        else
            break;
    }
    list[0] = new PointEntry[pointsPerList];

    currentList = 0;
    currentPoint = 0;
}


int
vtkDataSetFromVolume::PointList::GetList(vtkIdType listId,
                                         const PointEntry *&outlist) const
{
    if (listId > currentList)
    {
        outlist = NULL;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentPoint : pointsPerList);
}
 
 
vtkIdType
vtkDataSetFromVolume::PointList::GetNumberOfLists(void) const
{
    return currentList+1;
}
 
 
vtkIdType
vtkDataSetFromVolume::PointList::GetTotalNumberOfPoints(void) const
{
    vtkIdType numFullLists = currentList;  // actually currentList-1+1
    vtkIdType numExtra = currentPoint;  // again, currentPoint-1+1
 
    return numFullLists*pointsPerList + numExtra;
}


//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  Initialized new entries.
//
vtkIdType
vtkDataSetFromVolume::PointList::AddPoint(vtkIdType pt0, vtkIdType pt1, float percent)
{
    if (currentPoint >= pointsPerList)
    {
        if ((currentList+1) >= listSize)
        {
            PointEntry **tmpList = new PointEntry*[2*listSize];
            for (vtkIdType i = 0 ; i < listSize ; i++)
                tmpList[i] = list[i];
            for (vtkIdType i = listSize ; i < listSize*2 ; i++)
                tmpList[i] = NULL;

            listSize *= 2;
            delete [] list;
            list = tmpList;
        }
 
        currentList++;
        list[currentList] = new PointEntry[pointsPerList];
        currentPoint = 0;
    }
 
    list[currentList][currentPoint].ptIds[0] = pt0;
    list[currentList][currentPoint].ptIds[1] = pt1;
    list[currentList][currentPoint].percent = percent;
    currentPoint++;
 
    return (GetTotalNumberOfPoints()-1);
}


vtkDataSetFromVolume::EdgeHashEntry::EdgeHashEntry()
{
    id1  = 0;
    id2  = 0;
    ptId = 0;
    next = NULL;
}
 
 
void
vtkDataSetFromVolume::EdgeHashEntry::SetInfo(vtkIdType i1, vtkIdType i2, vtkIdType pId)
{
    id1  = i1;
    id2  = i2;
    ptId = pId;
    next = NULL;
}


vtkDataSetFromVolume::EdgeHashEntryMemoryManager::EdgeHashEntryMemoryManager()
{
    freeEntryindex = 0;
}
 
 
vtkDataSetFromVolume::EdgeHashEntryMemoryManager::~EdgeHashEntryMemoryManager()
{
    size_t npools = edgeHashEntrypool.size();
    for (size_t i = 0 ; i < npools ; i++)
    {
        EdgeHashEntry *pool = edgeHashEntrypool[i];
        delete [] pool;
    }
}
 
 
void
vtkDataSetFromVolume::EdgeHashEntryMemoryManager
::AllocateEdgeHashEntryPool(void)
{
    if (freeEntryindex == 0)
    {
        EdgeHashEntry *newlist = new EdgeHashEntry[POOL_SIZE];
        edgeHashEntrypool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
            freeEntrylist[i] = &(newlist[i]);
        }
        freeEntryindex = POOL_SIZE;
    }
}


vtkDataSetFromVolume::EdgeHashTable::EdgeHashTable(int nh, PointList &p)
    : pointlist(p)
{
    nHashes = nh;
    hashes = new EdgeHashEntry*[nHashes];
    for (int i = 0 ; i < nHashes ; i++)
        hashes[i] = NULL;
}
 
 
vtkDataSetFromVolume::EdgeHashTable::~EdgeHashTable()
{
    delete [] hashes;
}
 
 
void
vtkDataSetFromVolume::EdgeHashTable::Clear()
{
    delete [] hashes;
    hashes = new EdgeHashEntry*[nHashes];
    for (int i = 0 ; i < nHashes ; i++)
        hashes[i] = NULL;
}


vtkIdType
vtkDataSetFromVolume::EdgeHashTable::GetKey(vtkIdType p1, vtkIdType p2)
{
    vtkIdType rv = ((p1*18457 + p2*234749) % nHashes);
 
    // In case of overflows and modulo with negative numbers.
    if (rv < 0)
       rv += nHashes;
 
    return rv;
}


vtkIdType
vtkDataSetFromVolume::EdgeHashTable::AddPoint(vtkIdType ap1, vtkIdType ap2, float apercent)
{
    vtkIdType p1, p2;
    float percent;
    if (ap2 < ap1)
    {
        p1 = ap2;
        p2 = ap1;
        percent = 1. - apercent;
    }
    else
    {
        p1 = ap1;
        p2 = ap2;
        percent = apercent;
    }

    vtkIdType key = GetKey(p1, p2);
 
    //
    // See if we have any matches in the current hashes.
    //
    EdgeHashEntry *cur = hashes[key];
    while (cur != NULL)
    {
        if (cur->IsMatch(p1, p2))
        {
            //
            // We found a match.
            //
            return cur->GetPointId();
        }
        cur = cur->GetNext();
    }
 
    //
    // There was no match.  We will have to add a new entry.
    //
    EdgeHashEntry *new_one = emm.GetFreeEdgeHashEntry();
 
    vtkIdType newPt = pointlist.AddPoint(p1, p2, percent);
    new_one->SetInfo(p1, p2, newPt);
    new_one->SetNext(hashes[key]);
    hashes[key] = new_one;
 
    return newPt;
}


vtkDataSetFromVolume::vtkDataSetFromVolume(vtkIdType ptSizeGuess)
   : pt_list(), edges(ptSizeGuess, pt_list), numPrevPts(0)
{
}
      
vtkDataSetFromVolume::vtkDataSetFromVolume(vtkIdType nPts, vtkIdType ptSizeGuess)
   : pt_list(), edges(ptSizeGuess, pt_list), numPrevPts(nPts)
{
}
