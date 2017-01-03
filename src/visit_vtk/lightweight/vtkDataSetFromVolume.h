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
//                           vtkDataSetFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_DATASET_FROM_VOLUME_H
#define VTK_DATASET_FROM_VOLUME_H
#include <visit_vtk_light_exports.h>
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

    vtkIdType      AddPoint(vtkIdType, vtkIdType, float);
 
    vtkIdType      GetTotalNumberOfPoints(void) const;
    vtkIdType      GetNumberOfLists(void) const;
    int            GetList(vtkIdType, const PointEntry *&) const;
 
  protected:
    PointEntry   **list;
    vtkIdType      currentList;
    vtkIdType      currentPoint;
    vtkIdType      listSize;
    vtkIdType      pointsPerList;
};

protected:

class VISIT_VTK_LIGHT_API EdgeHashEntry
{
  public:
                    EdgeHashEntry();
    virtual        ~EdgeHashEntry() {;};
 
    void            SetInfo(vtkIdType, vtkIdType, vtkIdType);
    bool            IsMatch(vtkIdType i1, vtkIdType i2)
                           { return (i1 == id1 && i2 == id2 ? true : false); };
    vtkIdType       GetPointId(void) { return ptId; };
    EdgeHashEntry  *GetNext(void) { return next; };
    void            SetNext(EdgeHashEntry *n) { next = n; };
 
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
                      EdgeHashTable(int, PointList &);
    virtual          ~EdgeHashTable();
 
    void              Clear();

    vtkIdType         AddPoint(vtkIdType, vtkIdType, float);
    PointList        &GetPointList(void);
 
  protected:
    int                             nHashes;
    EdgeHashEntry                 **hashes;
    EdgeHashEntryMemoryManager      emm;
    PointList                      &pointlist;
 
    vtkIdType         GetKey(vtkIdType, vtkIdType);
};


  public:
                      vtkDataSetFromVolume(vtkIdType ptSizeGuess);
                      vtkDataSetFromVolume(vtkIdType nPts, vtkIdType ptSizeGuess);
    virtual          ~vtkDataSetFromVolume() { ; };

    vtkIdType         AddPoint(vtkIdType p1, vtkIdType p2, float percent)
                         { return numPrevPts + edges.AddPoint(p1,p2,percent); }

  protected:
    PointList         pt_list;
    EdgeHashTable     edges;
    vtkIdType         numPrevPts;
};


#endif


