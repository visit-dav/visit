// ************************************************************************* //
//                           vtkSurfaceFromVolume.h                          //
// ************************************************************************* //

#ifndef VTK_SURFACE_FROM_VOLUME_H
#define VTK_SURFACE_FROM_VOLUME_H


#include <vector>


class vtkCellData;
class vtkPointData;
class vtkPolyData;


// ****************************************************************************
//  Class: vtkSurfaceFromVolume
//
//  Purpose:
//      This class is a data object.  It stores out surfaces, making it similar
//      to vtkPolyData.  However, it assumes that the surfaces it is creating
//      stem from a volume.  In addition, it is assumed that each triangle from
//      the new surface is contained in a cell from the volume.  Finally, it
//      is assumed that each endpoint of the triangle is located on an edge of
//      a cell in the original volume.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2003
//
// ****************************************************************************

class vtkSurfaceFromVolume
{

class TriangleList
{
  public:
                   TriangleList();
    virtual       ~TriangleList();
 
    void           AddTriangle(int, int, int, int);
 
    int            GetTotalNumberOfTriangles(void) const;
    int            GetNumberOfLists(void) const;
    int            GetList(int, const int *&) const;
 
  protected:
    int          **list;
    int            currentList;
    int            currentTriangle;
    int            listSize;
    int            trianglesPerList;
};


struct PointEntry
{
   int     ptIds[2];
   float   percent;
};


class PointList
{
  public:
                   PointList();
    virtual       ~PointList();
 
    int            AddPoint(int, int, float);
 
    int            GetTotalNumberOfPoints(void) const;
    int            GetNumberOfLists(void) const;
    int            GetList(int, const PointEntry *&) const;
 
  protected:
    PointEntry   **list;
    int            currentList;
    int            currentPoint;
    int            listSize;
    int            pointsPerList;
};


class EdgeHashEntry
{
  public:
                    EdgeHashEntry();
    virtual        ~EdgeHashEntry() {;};
 
    void            SetInfo(int, int, int);
    bool            IsMatch(int i1, int i2)
                           { return (i1 == id1 && i2 == id2 ? true : false); };
    int             GetPointId(void) { return ptId; };
    EdgeHashEntry  *GetNext(void) { return next; };
    void            SetNext(EdgeHashEntry *n) { next = n; };
 
  protected:
    int             id1, id2;
    int             ptId;
    EdgeHashEntry  *next;
 
};


#define FREE_ENTRY_LIST_SIZE 16384
#define POOL_SIZE 256
class EdgeHashEntryMemoryManager
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


class EdgeHashTable
{
  public:
                      EdgeHashTable(int, PointList &);
    virtual          ~EdgeHashTable();
 
    int               AddPoint(int, int, float);
    PointList        &GetPointList(void);
 
  protected:
    int                             nHashes;
    EdgeHashEntry                 **hashes;
    EdgeHashEntryMemoryManager      emm;
    PointList                      &pointlist;
 
    int               GetKey(int, int);
};


  public:
                      vtkSurfaceFromVolume(int ptSizeGuess)
                           : pt_list(), tris(), edges(ptSizeGuess, pt_list)
                           { ; };
    virtual          ~vtkSurfaceFromVolume() { ; };

    void              ConstructPolyData(vtkPointData *, vtkCellData *,
                                        vtkPolyData *, float *);
    void              ConstructPolyData(vtkPointData *, vtkCellData *,
                                        vtkPolyData *, int *, float *, float *,
                                        float *);

    void              AddTriangle(int zone, int v0, int v1, int v2)
                            { tris.AddTriangle(zone, v0, v1, v2); };
    int               AddPoint(int p1, int p2, float percent)
                            { return edges.AddPoint(p1, p2, percent); };

  protected:
    PointList         pt_list;
    TriangleList      tris;
    EdgeHashTable     edges;
};


#endif


