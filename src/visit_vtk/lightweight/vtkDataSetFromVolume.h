// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           vtkDataSetFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_DATASET_FROM_VOLUME_H
#define VTK_DATASET_FROM_VOLUME_H
#include <visit_vtk_light_exports.h>
#include <cstddef>
#include <vtkType.h>
#include <vector>


class vtkCellData;
class vtkPointData;
class vtkPolyData;
class vtkPoints;

// ****************************************************************************
//  Class: vtkDataSetFromVolume
//
//  Purpose:
//      This class is a data object.  It stores out new datasets.  However, it
//      assumes that the new data set it is creating stems from a pre-existing
//      volumetric dataset. In addition, it is assumed that each cell from
//      the new data set is contained in a cell from the volume.  Finally, it
//      is assumed that each endpoint of the triangle is located on an edge
//      or original node of a cell in the original volume.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 15:58:10 PDT 2003
//    Refactored from vtkSurfaceFromVolume.  Allowed points based on
//    original cell points.
//
//    Brad Whitlock, Wed Mar 14 13:49:57 PDT 2012
//    Adapted the code to use vtkIdType.
//
//    Eric Brugger, Wed Jul 25 11:14:13 PDT 2012
//    Added a Clear method to the PointList and EdgeHashTable classes.
//
// ****************************************************************************

class VISIT_VTK_LIGHT_API vtkDataSetFromVolume
{
public:
struct PointEntry
{
   vtkIdType ptIds[2];
   float     percent;
};


class VISIT_VTK_LIGHT_API PointList
{
  public:
                   PointList();
    virtual       ~PointList();
 
    void           Clear();

    size_t         AddPoint(vtkIdType, vtkIdType, float);
 
    size_t         GetTotalNumberOfPoints(void) const;
    size_t         GetNumberOfLists(void) const;
    size_t         GetList(size_t, const PointEntry *&) const;
 
  protected:
    PointEntry   **list;
    size_t         currentList;
    size_t         currentPoint;
    size_t         listSize;
    size_t         pointsPerList;

    PointList(const PointList&) = delete;
    void operator=(const PointList&) = delete;
};

protected:

class VISIT_VTK_LIGHT_API EdgeHashEntry
{
  public:
                    EdgeHashEntry();
    virtual        ~EdgeHashEntry() {;}
 
    void            SetInfo(vtkIdType, vtkIdType, vtkIdType);
    bool            IsMatch(vtkIdType i1, vtkIdType i2)
                           { return (i1 == id1 && i2 == id2 ? true : false); }
    vtkIdType       GetPointId(void) { return ptId; }
    EdgeHashEntry  *GetNext(void) { return next; }
    void            SetNext(EdgeHashEntry *n) { next = n; }
 
  protected:
    vtkIdType       id1, id2;
    vtkIdType       ptId;
    EdgeHashEntry  *next;
 
};


#define FREE_ENTRY_LIST_SIZE 16384
#define POOL_SIZE 256
class VISIT_VTK_LIGHT_API EdgeHashEntryMemoryManager
{
  public:
                     EdgeHashEntryMemoryManager();
    virtual         ~EdgeHashEntryMemoryManager();
 
    inline EdgeHashEntry     *GetFreeEdgeHashEntry()
                         {
                             if (freeEntryindex <= 0)
                             {
                                 AllocateEdgeHashEntryPool();
                             }
                             freeEntryindex--;
                             return freeEntrylist[freeEntryindex];
                         }
 
    inline void      ReRegisterEdgeHashEntry(EdgeHashEntry *q)
                         {
                             if (freeEntryindex >= FREE_ENTRY_LIST_SIZE-1)
                             {
                                 // We've got plenty, so ignore this one.
                                 return;
                             }
                             freeEntrylist[freeEntryindex] = q;
                             freeEntryindex++;
                         }
 
  protected:
    EdgeHashEntry            *freeEntrylist[FREE_ENTRY_LIST_SIZE];
    int                       freeEntryindex;
 
    std::vector<EdgeHashEntry *> edgeHashEntrypool;
 
    void             AllocateEdgeHashEntryPool(void);
};


class VISIT_VTK_LIGHT_API EdgeHashTable
{
  public:
                      EdgeHashTable(size_t, PointList &);
    virtual          ~EdgeHashTable();
 
    void              Clear();

    vtkIdType         AddPoint(vtkIdType, vtkIdType, float);
    PointList        &GetPointList(void);
 
  protected:
    size_t                          nHashes;
    EdgeHashEntry                 **hashes;
    EdgeHashEntryMemoryManager      emm;
    PointList                      &pointlist;
 
    vtkIdType         GetKey(vtkIdType, vtkIdType);

    EdgeHashTable(const EdgeHashTable&)  = delete;
    void operator=(const EdgeHashTable&) = delete;
};


  public:
                      vtkDataSetFromVolume(size_t ptSizeGuess);
                      vtkDataSetFromVolume(vtkIdType nPts, size_t ptSizeGuess);
    virtual          ~vtkDataSetFromVolume() { ; }

    vtkIdType         AddPoint(vtkIdType p1, vtkIdType p2, float percent)
                         { return numPrevPts + edges.AddPoint(p1,p2,percent); }

  protected:
    PointList         pt_list;
    EdgeHashTable     edges;
    vtkIdType         numPrevPts;

    vtkDataSetFromVolume(const vtkDataSetFromVolume&) = delete;
    void operator=(const vtkDataSetFromVolume&) = delete;
};


#endif


