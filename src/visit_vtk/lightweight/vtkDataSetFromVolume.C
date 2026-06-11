// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           vtkDataSetFromVolume.C                          //
// ************************************************************************* //

#include "vtkDataSetFromVolume.h"

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
    for (size_t i = 1 ; i < listSize ; i++)
        list[i] = nullptr;
 
    currentList = 0;
    currentPoint = 0;
}
 
 
vtkDataSetFromVolume::PointList::~PointList()
{
    for (size_t i = 0 ; i < listSize ; i++)
    {
        if (list[i] != nullptr)
            delete [] list[i];
        else
            break;
    }
    delete [] list;
}


void
vtkDataSetFromVolume::PointList::Clear()
{
    for (size_t i = 0 ; i < listSize ; i++)
    {
        if (list[i] != nullptr)
        {
            delete [] list[i];
            list[i] = nullptr;
        }
        else
            break;
    }
    list[0] = new PointEntry[pointsPerList];

    currentList = 0;
    currentPoint = 0;
}


size_t
vtkDataSetFromVolume::PointList::GetList(size_t listId,
                                         const PointEntry *&outlist) const
{
    if (listId > currentList)
    {
        outlist = nullptr;
        return 0;
    }
 
    outlist = list[listId];
    return (listId == currentList ? currentPoint : pointsPerList);
}
 
 
size_t
vtkDataSetFromVolume::PointList::GetNumberOfLists(void) const
{
    return currentList+1;
}
 
 
size_t
vtkDataSetFromVolume::PointList::GetTotalNumberOfPoints(void) const
{
    size_t numFullLists = currentList;  // actually currentList-1+1
    size_t numExtra = currentPoint;  // again, currentPoint-1+1
 
    return numFullLists*pointsPerList + numExtra;
}


//
//    Sean Ahern, Mon Mar  5 15:44:05 EST 2007
//    Fixed test for resizing list.  Initialized new entries.
//
size_t
vtkDataSetFromVolume::PointList::AddPoint(vtkIdType pt0, vtkIdType pt1, float percent)
{
    if (currentPoint >= pointsPerList)
    {
        if ((currentList+1) >= listSize)
        {
            PointEntry **tmpList = new PointEntry*[2*listSize];
            for (size_t i = 0 ; i < listSize ; i++)
                tmpList[i] = list[i];
            for (size_t i = listSize ; i < listSize*2 ; i++)
                tmpList[i] = nullptr;

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
    next = nullptr;
}
 
 
void
vtkDataSetFromVolume::EdgeHashEntry::SetInfo(vtkIdType i1, vtkIdType i2, vtkIdType pId)
{
    id1  = i1;
    id2  = i2;
    ptId = pId;
    next = nullptr;
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


vtkDataSetFromVolume::EdgeHashTable::EdgeHashTable(size_t nh, PointList &p)
    : pointlist(p)
{
    nHashes = nh;
    hashes = new EdgeHashEntry*[nHashes];
    for (size_t i = 0 ; i < nHashes ; i++)
        hashes[i] = nullptr;
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
    for (size_t i = 0 ; i < nHashes ; i++)
        hashes[i] = nullptr;
}


vtkIdType
vtkDataSetFromVolume::EdgeHashTable::GetKey(vtkIdType p1, vtkIdType p2)
{
    vtkIdType rv = ((p1*18457 + p2*234749) % static_cast<vtkIdType>(nHashes));
 
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
        percent = 1.f - apercent;
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
    while (cur != nullptr)
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
 
    vtkIdType newPt = static_cast<vtkIdType>(pointlist.AddPoint(p1, p2, percent));
    new_one->SetInfo(p1, p2, newPt);
    new_one->SetNext(hashes[key]);
    hashes[key] = new_one;
 
    return newPt;
}


vtkDataSetFromVolume::vtkDataSetFromVolume(size_t ptSizeGuess)
   : pt_list(), edges(ptSizeGuess, pt_list), numPrevPts(0)
{
}
      
vtkDataSetFromVolume::vtkDataSetFromVolume(vtkIdType nPts, size_t ptSizeGuess)
   : pt_list(), edges(ptSizeGuess, pt_list), numPrevPts(nPts)
{
}
