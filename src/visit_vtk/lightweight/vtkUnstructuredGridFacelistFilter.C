// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      vtkUnstructuredGridFacelistFilter.C                  //
// ************************************************************************* //

#include "vtkUnstructuredGridFacelistFilter.h"
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolygon.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

#include <vector>


//
// This is easier than doing static const members.
//

#define FREE_FACE_LIST_SIZE 16384
#define FACES_PER_HASH_ENTRY 8
#define POOL_SIZE 256

#if FACES_PER_HASH_ENTRY > 8
   #error Bits are packed into an unsigned char.  8 is maximum.
#endif

//
// Forward declare some of types used to hash faces.
//
class Quad;
class QuadMemoryManager;
class Tri;
class TriMemoryManager;
class HashEntry;
class HashEntryList;
class HashEntryMemoryManager;


// ****************************************************************************
//  Class: QuadMemoryManager
//
//  Purpose:
//      This provides a pool of quads that can be used for hashing.  When a
//      quad is released from the hash, it is sent back to this module so
//      that it can be used again.  This is cache-friendly.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

class QuadMemoryManager
{
  public:
                     QuadMemoryManager();
    virtual         ~QuadMemoryManager();

    inline Quad     *GetFreeQuad(HashEntryList *hel)
                         {
                             if (freequadindex <= 0)
                             {
                                 AllocateQuadPool(hel);
                             }
                             freequadindex--;
                             return freequadlist[freequadindex];
                         }

    inline void      ReRegisterQuad(Quad *q)
                         {
                             if (freequadindex >= FREE_FACE_LIST_SIZE-1)
                             {
                                 // We've got plenty, so ignore this one.
                                 return;
                             }
                             freequadlist[freequadindex] = q;
                             freequadindex++;
                         }

  protected:
    Quad            *freequadlist[FREE_FACE_LIST_SIZE];
    int              freequadindex;

    std::vector<Quad *> quadpool;

    void             AllocateQuadPool(HashEntryList *hel);
};


// ****************************************************************************
//  Class: TriMemoryManager
//
//  Purpose:
//      This provides a pool of triangles that can be used for hashing.  When a
//      triangle is released from the hash, it is sent back to this module so
//      that it can be used again.  This is cache-friendly.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

class TriMemoryManager
{
  public:
                     TriMemoryManager();
    virtual         ~TriMemoryManager();

    inline Tri      *GetFreeTri(HashEntryList *hel)
                         {
                             if (freetriindex <= 0)
                             {
                                 AllocateTriPool(hel);
                             }
                             freetriindex--;
                             return freetrilist[freetriindex];
                         }

    inline void      ReRegisterTri(Tri *q)
                         {
                             if (freetriindex >= FREE_FACE_LIST_SIZE-1)
                             {
                                 // We've got plenty.
                                 return;
                             }
                             freetrilist[freetriindex] = q;
                             freetriindex++;
                         }

  protected:
    Tri            *freetrilist[FREE_FACE_LIST_SIZE];
    int             freetriindex;

    std::vector<Tri *> tripool;

    void            AllocateTriPool(HashEntryList *hel);
};


// ****************************************************************************
//  Class: HashEntry
//
//  Purpose:
//      This is one entry in a larger hash list.  It knows how to add quads
//      and tris to its lists and free them whenever we have a match.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

typedef union
{
    Quad *quad;
    Tri  *tri;
}  Face;

static int face_mask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

class HashEntry
{
  public:
                   HashEntry();
    virtual       ~HashEntry() {;};

    void           AddQuad(Quad *);
    void           AddTri(Tri *);

    inline void    SetPointIndex(int pi) { point_index = pi; };
    inline void    RegisterHashEntryList(HashEntryList *hel)
                          { hashEntryList = hel; };

    void           CreateOutputCells(vtkPolyData*, vtkCellData*, vtkCellData*);

  protected:
    Face           faces[FACES_PER_HASH_ENTRY];
    int            point_index;
    unsigned char  last_good_entry;
    unsigned char  face_type;
    HashEntry     *extension;
   
    HashEntryList *hashEntryList;

    bool           LocateAndRemoveTri(Tri *);
    bool           LocateAndRemoveQuad(Quad *);
    void           ActuallyAddTri(Tri *);
    void           ActuallyAddQuad(Quad *);
    void           RemoveEntry(int);
};


// ****************************************************************************
//  Class: HashEntryMemoryManager
//
//  Purpose:
//      This provides a pool of HashEntry objects.  They are allocated in bulk,
//      and then freed in bulk.
//
//  Programmer: Hank Childs
//  Creation:   October 22, 2002
//
// ****************************************************************************

class HashEntryMemoryManager
{
  public:
                         HashEntryMemoryManager();
    virtual             ~HashEntryMemoryManager();

    inline HashEntry    *GetHashEntry(HashEntryList *hel)
                         {
                             if (currentHash+1 >= POOL_SIZE)
                             {
                                 AllocateHashEntryPool(hel);
                             }
                             HashEntry *rv = currentHashPool + currentHash;
                             currentHash++;
                             return rv;
                         }

    void                 AllocateHashEntryPool(HashEntryList *hel);

  protected:
    HashEntry           *currentHashPool;
    int                  currentHash;

    std::vector<HashEntry *> hashpool;
};


// ****************************************************************************
//  Class: HashEntryList
//
//  Purpose:
//      This effectively works as the hash.  It hashes each faces by its lowest
//      numbered index. 
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

class HashEntryList
{
  public:
                HashEntryList(int npts);
    virtual    ~HashEntryList();

    void        AddTri(const vtkIdType *, vtkIdType orig_zone);
    void        AddQuad(const vtkIdType *, vtkIdType orig_zone);

    inline void RemoveFace(void) { nfaces--; };
    inline int  GetNumberOfFaces(void) { return nfaces; };

    void        CreateOutputCells(vtkPolyData *, vtkCellData *, vtkCellData *);

  protected:
    HashEntry            **list;
    int                    nhashes;
    int                    npts;
    int                    nfaces;
    QuadMemoryManager      qmm;
    TriMemoryManager       tmm;
    HashEntryMemoryManager hemm;

    // The friend is so each of these classes can access the memory managers.
    friend class HashEntry;
    friend class Quad;
    friend class Tri;
};

// ****************************************************************************
//  Class: Quad
//
//  Purpose:
//      A representation of a quadrilateral.  For efficient caching, the
//      indices are sorted in numerical order.  The lowest indexed node is
//      dropped, since that serves as the key into the hash.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

class Quad
{
    friend class   Tri;

  public:
                   Quad() { ordering_case = 255; };

    vtkIdType      AssignNodes(const vtkIdType *);
    bool           Equals(Quad *);
    bool           Equals(Tri *);
    void           AddInRemainingTriangle(Tri *, int);
    inline void    ReRegisterMemory(void)
                         {
                             hashEntryList->qmm.ReRegisterQuad(this);
                         }

    inline void    SetOriginalZone(const int &oz) { orig_zone = oz; };
    inline int     GetOriginalZone(void) { return orig_zone; };

    void           OutputCell(int,vtkPolyData *, vtkCellData *, vtkCellData *);

    inline void    RegisterHashEntryList(HashEntryList *hel)
                          { hashEntryList = hel; };
    inline void    SetNumberOfPoints(int np) { npts = np; };

  protected:
    unsigned char  ordering_case;
    vtkIdType      nodes[3];
    vtkIdType      orig_zone;

    HashEntryList *hashEntryList;
    int            npts;

    void           AddInRemainingTriangle(int, int);
};

//
// We will be re-ordering the nodes into numerical order.  This enumerated
// type will allow the ordering to be preserved.
//
typedef enum
{
    Q0123, Q0132, Q0213, Q0231, Q0312, Q0321,
    Q1023, Q1032, Q1203, Q1230, Q1302, Q1320,
    Q2013, Q2031, Q2103, Q2130, Q2301, Q2310,
    Q3012, Q3021, Q3102, Q3120, Q3201, Q3210
}  QUAD_ORDERING_CASES;

static vtkIdType quad_reorder_list[24][4] = 
    { { -1, 0, 1, 2 }, { -1, 0, 2, 1 }, { -1, 1, 0, 2 }, { -1, 2, 0, 1 },
      { -1, 1, 2, 0 }, { -1, 2, 1, 0 },
      { 0, -1, 1, 2 }, { 0, -1, 2, 1 }, { 1, -1, 0, 2 }, { 2, -1, 0, 1 },
      { 1, -1, 2, 0 }, { 2, -1, 1, 0 },
      { 0, 1, -1, 2 }, { 0, 2, -1, 1 }, { 1, 0, -1, 2 }, { 2, 0, -1, 1 },
      { 1, 2, -1, 0 }, { 2, 1, -1, 0 },
      { 0, 1, 2, -1 }, { 0, 2, 1, -1 }, { 1, 0, 2, -1 }, { 2, 0, 1, -1 },
      { 1, 2, 0, -1 }, { 2, 1, 0, -1 } 
    };

static vtkIdType quad_map_back_list[24][3] =
    {
         { 1, 2, 3 }, { 1, 3, 2 }, { 2, 1, 3 },
         { 2, 3, 1 }, { 3, 1, 2 }, { 3, 2, 1 },
         { 0, 2, 3 }, { 0, 3, 2 }, { 2, 0, 3 },
         { 2, 3, 0 }, { 3, 0, 2 }, { 3, 2, 0 },
         { 0, 1, 3 }, { 0, 3, 1 }, { 1, 0, 3 },
         { 1, 3, 0 }, { 3, 0, 1 }, { 3, 1, 0 },
         { 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 },
         { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 }
    };


// ****************************************************************************
//  Class: Tri
//
//  Purpose:
//      A representation of a triangle.  For efficient caching, the
//      indices are sorted in numerical order.  The lowest indexed node is
//      dropped, since that serves as the key into the hash.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

class Tri
{
    friend class   Quad;

  public:
                   Tri() { ordering_case = 255; };

    vtkIdType      AssignNodes(const vtkIdType *);
    inline bool    Equals(Tri *&t)
                   {
                      if (t->nodes[0] == nodes[0] && t->nodes[1] == nodes[1])
                      {
                          return true;
                      }
                      return false;
                   }

    bool           Equals(Quad *);
    void           AddInRemainingTriangle(Quad *, int);
    inline void    ReRegisterMemory(void)
                       {
                           hashEntryList->tmm.ReRegisterTri(this);
                       }

    inline void    SetOriginalZone(const int &oz) { orig_zone = oz; };
    inline int     GetOriginalZone(void) { return orig_zone; };

    void           OutputCell(int,vtkPolyData *, vtkCellData *, vtkCellData *);

    inline void    RegisterHashEntryList(HashEntryList *hel)
                          { hashEntryList = hel; };
    inline void    SetNumberOfPoints(int np) { npts = np; };


  protected:
    unsigned char  ordering_case;
    vtkIdType      nodes[2];
    vtkIdType      orig_zone;

    int            npts;
    HashEntryList *hashEntryList;
};

//
// We will be re-ordering the nodes into numerical order.  This enumerated
// type will allow the ordering to be preserved.
//
typedef enum
{
    T012, T021,
    T102, T120,
    T201, T210
}  TRI_ORDERING_CASES;

static int tri_reorder_list[6][3] = 
    { 
        { -1, 0, 1 }, { -1, 1, 0 }, 
        { 0, -1, 1 }, { 0, 1, -1 },
        { 1, -1, 0 }, { 1, 0, -1 }
    };


//
// Function prototypes
//

static void AddTetrahedron(const vtkIdType *, int, HashEntryList &);
static void AddWedge(const vtkIdType *, int, HashEntryList &);
static void AddPyramid(const vtkIdType *, int, HashEntryList &);
static void AddHexahedron(const vtkIdType *, int, HashEntryList &);
static void AddVoxel(const vtkIdType *, int, HashEntryList &);

static void AddQuadraticTriangle(const vtkIdType *, int, HashEntryList &);
static void AddQuadraticQuad(const vtkIdType *, int, HashEntryList &);
static void AddQuadraticTetrahedron(const vtkIdType *, int, HashEntryList &);
static void AddQuadraticHexahedron(const vtkIdType *, int, HashEntryList &);
static void AddQuadraticPyramid(const vtkIdType *, int, HashEntryList &);
static void AddQuadraticWedge(const vtkIdType *, int, HashEntryList &);

static void AddQuadraticLinearQuad(const vtkIdType *, int, HashEntryList &);
static void AddQuadraticLinearWedge(const vtkIdType *, int, HashEntryList &);
static void AddBiQuadraticTriangle(const vtkIdType *, int, HashEntryList &);
static void AddBiQuadraticQuad(const vtkIdType *, int, HashEntryList &);
static void AddBiQuadraticQuadraticWedge(const vtkIdType *, int, HashEntryList &);
static void AddBiQuadraticQuadraticHexahedron(const vtkIdType *, int, HashEntryList &);
static void AddTriQuadraticHexahedron(const vtkIdType *, int, HashEntryList &);

static void AddUnknownCell(vtkCell *, int, HashEntryList &);

static void LoopOverAllCells(vtkUnstructuredGrid *, HashEntryList &,
                             int &, int &, int &, int &);
static void LoopOverVertexCells(vtkUnstructuredGrid *, vtkPolyData *,
                                vtkCellData *, vtkCellData *);
static void LoopOverLineCells(vtkUnstructuredGrid *, vtkPolyData *,
                              vtkCellData *, vtkCellData *);
static void LoopOverPolygonCells(vtkUnstructuredGrid *, vtkPolyData *,
                                 vtkCellData *, vtkCellData *);
static void LoopOverStripCells(vtkUnstructuredGrid *, vtkPolyData *,
                               vtkCellData *, vtkCellData *);


// ****************************************************************************
//  Method: Quad::AssignNodes
//
//  Purpose:
//      Tells the Quad what its nodes will be.  Also sorts the nodes in
//      numerical order and stores off the original ordering.  The return value
//      is the lowest-indexed node, which is not stored with the quad, but is
//      instead used as the hash key.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Nov 16 14:54:09 PST 2004
//    Make more robust for degenerate input data.  Specifically, quads with 
//    all four corners at the same node.  See '5659.
//
// ****************************************************************************

vtkIdType
Quad::AssignNodes(const vtkIdType *n)
{
    vtkIdType smallest = 0;
    if (n[1] < n[smallest])
       smallest = 1;
    if (n[2] < n[smallest])
       smallest = 2;
    if (n[3] < n[smallest])
       smallest = 3;

    int biggest = 0;
    if (n[1] > n[biggest])
       biggest = 1;
    if (n[2] > n[biggest])
       biggest = 2;
    if (n[3] > n[biggest])
       biggest = 3;

    // Handle degenerate cases; it doesn't matter which one we pick
    if (biggest == smallest)
    {
        ordering_case = Q0123;
        nodes[0] = n[1];
        nodes[1] = n[2];
        nodes[2] = n[3];
    }
    else if (biggest == 3)
    { 
        if (smallest == 0)
        {
            if (n[1] < n[2])
            {
                ordering_case = Q0123;
                nodes[0] = n[1];
                nodes[1] = n[2];
                nodes[2] = n[3];
            }
            else
            {
                ordering_case = Q0213;
                nodes[0] = n[2];
                nodes[1] = n[1];
                nodes[2] = n[3];
            }
        }
        else if (smallest == 1)
        {
            if (n[0] < n[2])
            {
                ordering_case = Q1023;
                nodes[0] = n[0];
                nodes[1] = n[2];
                nodes[2] = n[3];
            }
            else
            {
                ordering_case = Q1203;
                nodes[0] = n[2];
                nodes[1] = n[0];
                nodes[2] = n[3];
            }
        }
        else if (smallest == 2)
        {
            if (n[0] < n[1])
            {
                ordering_case = Q2013;
                nodes[0] = n[0];
                nodes[1] = n[1];
                nodes[2] = n[3];
            }
            else
            {
                ordering_case = Q2103;
                nodes[0] = n[1];
                nodes[1] = n[0];
                nodes[2] = n[3];
            }
        }
    }
    else if (biggest == 2)
    {
        if (smallest == 0)
        {
            if (n[1] < n[3])
            {
                ordering_case = Q0132;
                nodes[0] = n[1];
                nodes[1] = n[3];
                nodes[2] = n[2];
            }
            else
            {
                ordering_case = Q0312;
                nodes[0] = n[3];
                nodes[1] = n[1];
                nodes[2] = n[2];
            }
        }
        else if (smallest == 1)
        {
            if (n[0] < n[3])
            {
                ordering_case = Q1032;
                nodes[0] = n[0];
                nodes[1] = n[3];
                nodes[2] = n[2];
            }
            else
            {
                ordering_case = Q1302;
                nodes[0] = n[3];
                nodes[1] = n[0];
                nodes[2] = n[2];
            }
        }
        else if (smallest == 3)
        {
            if (n[0] < n[1])
            {
                ordering_case = Q3012;
                nodes[0] = n[0];
                nodes[1] = n[1];
                nodes[2] = n[2];
            }
            else
            {
                ordering_case = Q3102;
                nodes[0] = n[1];
                nodes[1] = n[0];
                nodes[2] = n[2];
            }
        }
    }
    else if (biggest == 1)
    {
        if (smallest == 0)
        {
            if (n[2] < n[3])
            {
                ordering_case = Q0231;
                nodes[0] = n[2];
                nodes[1] = n[3];
                nodes[2] = n[1];
            }
            else
            {
                ordering_case = Q0321;
                nodes[0] = n[3];
                nodes[1] = n[2];
                nodes[2] = n[1];
            }
        }
        else if (smallest == 2)
        {
            if (n[0] < n[3])
            {
                ordering_case = Q2031;
                nodes[0] = n[0];
                nodes[1] = n[3];
                nodes[2] = n[1];
            }
            else
            {
                ordering_case = Q2301;
                nodes[0] = n[3];
                nodes[1] = n[0];
                nodes[2] = n[1];
            }
        }
        else if (smallest == 3)
        {
            if (n[0] < n[2])
            {
                ordering_case = Q3021;
                nodes[0] = n[0];
                nodes[1] = n[2];
                nodes[2] = n[1];
            }
            else
            {
                ordering_case = Q3201;
                nodes[0] = n[2];
                nodes[1] = n[0];
                nodes[2] = n[1];
            }
        }
    }
    else if (biggest == 0)
    {
        if (smallest == 1)
        {
            if (n[2] < n[3])
            {
                ordering_case = Q1230;
                nodes[0] = n[2];
                nodes[1] = n[3];
                nodes[2] = n[0];
            }
            else
            {
                ordering_case = Q1320;
                nodes[0] = n[3];
                nodes[1] = n[2];
                nodes[2] = n[0];
            }
        }
        else if (smallest == 2)
        {
            if (n[1] < n[3])
            {
                ordering_case = Q2130;
                nodes[0] = n[1];
                nodes[1] = n[3];
                nodes[2] = n[0];
            }
            else
            {
                ordering_case = Q2310;
                nodes[0] = n[3];
                nodes[1] = n[1];
                nodes[2] = n[0];
            }
        }
        else if (smallest == 3)
        {
            if (n[1] < n[2])
            {
                ordering_case = Q3120;
                nodes[0] = n[1];
                nodes[1] = n[2];
                nodes[2] = n[0];
            }
            else
            {
                ordering_case = Q3210;
                nodes[0] = n[2];
                nodes[1] = n[1];
                nodes[2] = n[0];
            }
        }
    }
    
/*** There was an effort to play with additional hashing functions.  It was
 *** determined that the functions to calculate the key was just too expensive.
 *** The only function that could be used for a single pass hash was one that
 *** focused on the smallest node and its smallest neighboring node.
 *** The below code is being left behind in case anyone wants to play with this
 *** again.
    static int smallest_neighbor_to_minimum_node[24] =
       { 1, 1, 1, 3, 3, 3, 0, 0, 2, 2, 0, 2,
         1, 3, 1, 1, 3, 3, 0, 0, 0, 2, 2, 2 };
    int n1 = nodes[smallest_neighbor_to_minimum_node[ordering_case]];
    int nmod4 = n1%4;
    int val = nmod4*npts + n[smallest];
    return val;
 */
    return n[smallest];
}


// ****************************************************************************
//  Method: Quad::OutputCell
//
//  Purpose:
//      Outputs a vtkQuad into the poly data object.
//
//  Programmer: Hank Childs
//  Creation:   October 23, 2002
//
// ****************************************************************************

void
Quad::OutputCell(int node0, vtkPolyData *pd, vtkCellData *in_cd,
                 vtkCellData *out_cd)
{
    vtkIdType n[4];
    vtkIdType *list = quad_reorder_list[ordering_case];
    n[0] = (list[0] == -1 ? node0 : nodes[list[0]]);
    n[1] = (list[1] == -1 ? node0 : nodes[list[1]]);
    n[2] = (list[2] == -1 ? node0 : nodes[list[2]]);
    n[3] = (list[3] == -1 ? node0 : nodes[list[3]]);
    int newId = pd->InsertNextCell(VTK_QUAD, 4, n);
    out_cd->CopyData(in_cd, orig_zone, newId);
}


// ****************************************************************************
//  Method: Quad::Equals
//
//  Purpose:
//      Returns true if this quad equals the argument.  Assumes that both quads
//      shares the same lowest indexed node.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

bool
Quad::Equals(Quad *q)
{
    if (q->nodes[0] == nodes[0] && q->nodes[1] == nodes[1] &&
        q->nodes[2] == nodes[2])
    {
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: Quad::Equals
//
//  Purpose:
//      Returns true if this quad equals the argument.  Assumes that both faces
//      shares the same lowest indexed node.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

bool
Quad::Equals(Tri *t)
{
    if (t->nodes[0] == nodes[0])
    {
        if (t->nodes[1] == nodes[1])
        {
            return true;
        }
        else if (t->nodes[1] == nodes[2])
        {
            return true;
        }
    }
    else if (t->nodes[0] == nodes[1] && t->nodes[1] == nodes[2])
    {
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: Quad::AddInRemainingTriangle
//
//  Purpose:
//      A publicly accessible function to add in the remaining triangle.  This
//      is needed because a triangle has matched half a quad and the other
//      half needs to be added back into the hash.
//
//  Programmer: Hank Childs
//  Creation:   October 28, 2002
//
// ****************************************************************************

void
Quad::AddInRemainingTriangle(Tri *t, int node_0)
{
    if (t->nodes[0] == nodes[0])
    {
        if (t->nodes[1] == nodes[1])
        {
            AddInRemainingTriangle(2, node_0);
        }
        else if (t->nodes[1] == nodes[2])
        {
            AddInRemainingTriangle(1, node_0);
        }
    }
    else if (t->nodes[0] == nodes[1] && t->nodes[1] == nodes[2])
    {
        AddInRemainingTriangle(0, node_0);
    }
}


// ****************************************************************************
//  Method: Quad::AddInRemainingTriangle
//
//  Purpose:
//      If a quad A is matched by triangle B, this will determine the triangle
//      that is the result if you take B away from A.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Jan  2 17:34:56 PST 2003
//    (-1 % 4) != 3  (sadly)
//
// ****************************************************************************

void
Quad::AddInRemainingTriangle(int n, int node_0)
{
    vtkIdType orig_quad_index = quad_map_back_list[ordering_case][n];
    vtkIdType *neighbors = quad_reorder_list[ordering_case];

    vtkIdType n_list[3];
    n_list[0] = neighbors[(orig_quad_index+3)%4];
    n_list[1] = neighbors[orig_quad_index];
    n_list[2] = neighbors[(orig_quad_index+1)%4];
    vtkIdType tmp_nodes[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        tmp_nodes[i] = (n_list[i] == -1 ? node_0 : nodes[n_list[i]]);
    }
    hashEntryList->AddTri(tmp_nodes, orig_zone);
}


// ****************************************************************************
//  Method: Tri::AssignNodes
//
//  Purpose:
//      Tells the Tri what its nodes will be.  Also sorts the nodes in
//      numerical order and stores off the original ordering.  The return value
//      is the lowest-indexed node, which is not stored with the tri, but is
//      instead used as the hash key.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

vtkIdType
Tri::AssignNodes(const vtkIdType *n)
{
    int smallest = 0;
    if (n[0] < n[1])
    {
        if (n[1] < n[2])
        {
            ordering_case = T012;
            smallest = n[0];
            nodes[0] = n[1];
            nodes[1] = n[2];
        }
        else if (n[0] < n[2])
        {
            ordering_case = T021;
            smallest = n[0];
            nodes[0] = n[2];
            nodes[1] = n[1];
        }
        else
        {
            ordering_case = T201;
            smallest = n[2];
            nodes[0] = n[0];
            nodes[1] = n[1];
        }
    }
    else
    {
        if (n[2] < n[1])
        {
            ordering_case = T210;
            smallest = n[2];
            nodes[0] = n[1];
            nodes[1] = n[0];
        }
        else if (n[0] < n[2])
        {
            ordering_case = T102;
            smallest = n[1];
            nodes[0] = n[0];
            nodes[1] = n[2];
        }
        else
        {
            ordering_case = T120;
            smallest = n[1];
            nodes[0] = n[2];
            nodes[1] = n[0];
        }
    }

/* *** See comment in Quad::AssignNodes to explain this previous hashing
   *** scheme.
    int nmod4 = nodes[0] % 4;
    int val = nmod4*npts + smallest;
    return val;
 */
    return smallest;
}


// ****************************************************************************
//  Method: Tri::OutputCell
//
//  Purpose:
//      Outputs a vtkTri into the poly data object.
//
//  Programmer: Hank Childs
//  Creation:   October 23, 2002
//
// ****************************************************************************

void
Tri::OutputCell(int node0, vtkPolyData *pd, vtkCellData *in_cd,
                 vtkCellData *out_cd)
{
    vtkIdType n[3];
    int *list = tri_reorder_list[ordering_case];
    n[0] = (list[0] == -1 ? node0 : nodes[list[0]]);
    n[1] = (list[1] == -1 ? node0 : nodes[list[1]]);
    n[2] = (list[2] == -1 ? node0 : nodes[list[2]]);
    int newId = pd->InsertNextCell(VTK_TRIANGLE, 3, n);
    out_cd->CopyData(in_cd, orig_zone, newId);
}


// ****************************************************************************
//  Method: Tri::Equals
//
//  Purpose:
//      Returns true if this tri equals the argument.  Assumes that both faces
//      shares the same lowest indexed node.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

bool
Tri::Equals(Quad *q)
{
    //
    // The quad already knows how to do this, so let it do the dirty work.
    //
    return q->Equals(this);
}


// ****************************************************************************
//  Method: Tri::AddInRemainingTriangle
//
//  Purpose:
//      A publicly accessible function to add in the remaining triangle.  This
//      is needed because a triangle has matched half a quad and the other
//      half of the quad needs to be added back into the hash.
//
//  Programmer: Hank Childs
//  Creation:   October 28, 2002
//
// ****************************************************************************

void
Tri::AddInRemainingTriangle(Quad *q, int node_0)
{
    q->AddInRemainingTriangle(this, node_0);
}


// ****************************************************************************
//  Method: HashEntry constructor
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Mar  9 07:14:49 PST 2005
//    Initialize face_type.
//
// ****************************************************************************

HashEntry::HashEntry()
{
    point_index = -1;
    last_good_entry = 0;
    extension = NULL;
    face_type = 0;
}


// ****************************************************************************
//  Method: HashEntry::AddTri
//
//  Purpose:
//      Adds a triangle to the hash entry.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
HashEntry::AddTri(Tri *f)
{
    bool foundFace = LocateAndRemoveTri(f);

    if (!foundFace)
    {
        ActuallyAddTri(f);
    }
}
        

// ****************************************************************************
//  Method: HashEntry::AddQuad
//
//  Purpose:
//      Adds a quad to the hash entry.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
HashEntry::AddQuad(Quad *f)
{
    bool foundFace = LocateAndRemoveQuad(f);

    if (!foundFace)
    {
        ActuallyAddQuad(f);
    }
}
        

// ****************************************************************************
//  Method: HashEntry::RemoveEntry
//
//  Purpose:
//      This removes an entry from the hash list.  It swaps the last entry
//      with the argument ('ind'), and updates the bit logic saying whether
//      each face is a triangle or a quadrilateral.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
HashEntry::RemoveEntry(int ind)
{
    last_good_entry--;
    faces[ind] = faces[last_good_entry];
    face_type &= ~(face_mask[ind]);
    if (face_type & face_mask[last_good_entry])
    {
        face_type |= face_mask[ind];
    }
}


// ****************************************************************************
//  Method: HashEntry::LocateAndRemoveQuad
// 
//  Purpose:
//      Locates a quad in the hash entry and removes it if it exists.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

bool
HashEntry::LocateAndRemoveQuad(Quad *f)
{
    for (int i = 0 ; i < last_good_entry ; i++)
    {
        if (face_type & face_mask[i])
        {
            Quad *q = faces[i].quad;
            if (q->Equals(f))
            {
                hashEntryList->RemoveFace();
                hashEntryList->RemoveFace();
                RemoveEntry(i);
                q->ReRegisterMemory();
                f->ReRegisterMemory();
                return true;
            }
        }
        else
        {
            Tri *t = faces[i].tri;
            if (t->Equals(f))
            {
                hashEntryList->RemoveFace();
                hashEntryList->RemoveFace();
                RemoveEntry(i);
                f->AddInRemainingTriangle(t, point_index);
                t->ReRegisterMemory();
                f->ReRegisterMemory();
                return true;
            }
        }
    }
    if (extension != NULL)
    {
        return extension->LocateAndRemoveQuad(f);
    }

    return false;
}


// ****************************************************************************
//  Method: HashEntry::LocateAndRemoveQuad
// 
//  Purpose:
//      Locates a triangle in the hash entry and removes it if it exists.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

bool
HashEntry::LocateAndRemoveTri(Tri *f)
{
    for (int i = 0 ; i < last_good_entry ; i++)
    {
        if (face_type & face_mask[i])
        {
            Quad *q = faces[i].quad;
            if (q->Equals(f))
            {
                hashEntryList->RemoveFace();
                hashEntryList->RemoveFace();
                RemoveEntry(i);
                q->AddInRemainingTriangle(f, point_index);
                q->ReRegisterMemory();
                f->ReRegisterMemory();
                return true;
            }
        }
        else
        {
            Tri *t = faces[i].tri;
            if (t->Equals(f))
            {
                hashEntryList->RemoveFace();
                hashEntryList->RemoveFace();
                RemoveEntry(i);
                t->ReRegisterMemory();
                f->ReRegisterMemory();
                return true;
            }
        }
    }
    if (extension != NULL)
    {
        return extension->LocateAndRemoveTri(f);
    }

    return false;
}


// ****************************************************************************
//  Method: HashEntry::ActuallyAddQuad
//
//  Purpose:
//      After determining that this quad is unique, this actually adds it to
//      the hash entry.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
HashEntry::ActuallyAddQuad(Quad *f)
{
    if (last_good_entry < FACES_PER_HASH_ENTRY)
    {
         faces[last_good_entry].quad = f;
         face_type |= face_mask[last_good_entry];
         last_good_entry++;
         return;
    }

    if (extension == NULL)
    {
        extension = hashEntryList->hemm.GetHashEntry(hashEntryList);
        extension->SetPointIndex(point_index);
    }
    extension->ActuallyAddQuad(f);
}


// ****************************************************************************
//  Method: HashEntry::ActuallyAddTri
//
//  Purpose:
//      After determining that this triangle is unique, this actually adds it 
//      to the hash entry.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
HashEntry::ActuallyAddTri(Tri *f)
{
    if (last_good_entry < FACES_PER_HASH_ENTRY)
    {
         faces[last_good_entry].tri = f;
         face_type &= ~(face_mask[last_good_entry]);
         last_good_entry++;
         return;
    }

    if (extension == NULL)
    {
        extension = hashEntryList->hemm.GetHashEntry(hashEntryList);
        extension->SetPointIndex(point_index);
    }
    extension->ActuallyAddTri(f);
}


// ****************************************************************************
//  Method: HashEntryMemoryManager constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

HashEntryMemoryManager::HashEntryMemoryManager()
{
}


// ****************************************************************************
//  Method: HashEntryMemoryManager destructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

HashEntryMemoryManager::~HashEntryMemoryManager()
{
    size_t size = hashpool.size();
    for (size_t i = 0 ; i < size ; i++)
    {
        delete [] hashpool[i];
    }
}


// ****************************************************************************
//  Method: HashEntryMemoryManager::AllocateHashEntryPool
//
//  Purpose:
//      Creates a pool of hash entries in a large chunk.  This is to reduce
//      the memory costs.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
HashEntryMemoryManager::AllocateHashEntryPool(HashEntryList *hel)
{
    currentHashPool = new HashEntry[POOL_SIZE];
    currentHash = 0;
    hashpool.push_back(currentHashPool);
    for (int i = 0 ; i < POOL_SIZE ; i++)
    {
        currentHashPool[i].RegisterHashEntryList(hel);
    }
}


// ****************************************************************************
//  Method: QuadMemoryManager constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

QuadMemoryManager::QuadMemoryManager()
{
    freequadindex = 0;
}


// ****************************************************************************
//  Method: QuadMemoryManager destructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

QuadMemoryManager::~QuadMemoryManager()
{
    size_t npools = quadpool.size();
    for (size_t i = 0 ; i < npools ; i++)
    {
        Quad *pool = quadpool[i];
        delete [] pool;
    }
}


// ****************************************************************************
//  Method: QuadMemoryManager::AllocateQuadPool
//
//  Purpose:
//      Creates a pool of quads in a large chunk.  This is to reduce
//      the memory costs.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
QuadMemoryManager::AllocateQuadPool(HashEntryList *hel)
{
    if (freequadindex == 0)
    {
        Quad *newlist = new Quad[POOL_SIZE];
        quadpool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
            newlist[i].RegisterHashEntryList(hel);
            freequadlist[i] = &(newlist[i]);
        }
        freequadindex = POOL_SIZE;
    }
}


// ****************************************************************************
//  Method: TriMemoryManager constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

TriMemoryManager::TriMemoryManager()
{
    freetriindex = 0;
}


// ****************************************************************************
//  Method: TriMemoryManager destructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

TriMemoryManager::~TriMemoryManager()
{
    size_t npools = tripool.size();
    for (size_t i = 0 ; i < npools ; i++)
    {
        Tri *pool = tripool[i];
        delete [] pool;
    }
}


// ****************************************************************************
//  Method: TriMemoryManager::AllocateTriPool
//
//  Purpose:
//      Creates a pool of triangles in a large chunk.  This is to reduce
//      the memory costs.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
TriMemoryManager::AllocateTriPool(HashEntryList *hel)
{
    if (freetriindex <= 0)
    {
        Tri *newlist = new Tri[POOL_SIZE];
        tripool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
            newlist[i].RegisterHashEntryList(hel);
            freetrilist[i] = &(newlist[i]);
        }
        freetriindex = POOL_SIZE;
    }
}


// ****************************************************************************
//  Method: HashEntryList constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

HashEntryList::HashEntryList(int np)
{
    npts = np;
    nhashes = npts;
    nfaces = 0;
    list = new HashEntry*[nhashes];
    for (int i = 0 ; i < nhashes ; i++)
    {
        list[i] = NULL;
    }
    hemm.AllocateHashEntryPool(this);
}


// ****************************************************************************
//  Method: HashEntryList constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

HashEntryList::~HashEntryList()
{
    if (list != NULL)
    {
        delete [] list;
        list = NULL;
    }
}


// ****************************************************************************
//  Method: HashEntryList::AddTri
//
//  Purpose:
//      Constructs a 'Tri' object out of the arguments and attempts to add it
//      to whatever HashEntry object the 'Tri' hashes to.  The HashEntry will
//      either add it, or identify it as a duplicate and remove it and the
//      duplicate.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
HashEntryList::AddTri(const vtkIdType *node_list, vtkIdType orig_zone)
{
    nfaces++;
    Tri *tri = tmm.GetFreeTri(this);
    vtkIdType hash_index = tri->AssignNodes(node_list);
    tri->SetOriginalZone(orig_zone);
    if (list[hash_index] == NULL)
    {
        list[hash_index] = hemm.GetHashEntry(this);
        list[hash_index]->SetPointIndex(hash_index);
    }
    list[hash_index]->AddTri(tri);
}


// ****************************************************************************
//  Method: HashEntryList::AddQuad
//
//  Purpose:
//      Constructs a 'Quad' object out of the arguments and attempts to add it
//      to whatever HashEntry object the 'Quad' hashes to.  The HashEntry will
//      either add it, or identify it as a duplicate and remove it and the
//      duplicate.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
HashEntryList::AddQuad(const vtkIdType *node_list, vtkIdType orig_zone)
{
    nfaces++;
    Quad *quad = qmm.GetFreeQuad(this);
    vtkIdType hash_index = quad->AssignNodes(node_list);
    quad->SetOriginalZone(orig_zone);
    if (list[hash_index] == NULL)
    {
        list[hash_index] = hemm.GetHashEntry(this);
        list[hash_index]->SetPointIndex(hash_index%npts);
    }
    list[hash_index]->AddQuad(quad);
}


// ****************************************************************************
//  Method: HashEntryList::CreateOutputCells
//
//  Purpose:
//      Goes through each of the hash entries and has it output its faces as
//      VTK objects.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
HashEntryList::CreateOutputCells(vtkPolyData *output, vtkCellData *in_cd,
                                 vtkCellData *out_cd)
{
    for (int i = 0 ; i < nhashes ; i++)
    {
        if (list[i] != NULL)
        {
            list[i]->CreateOutputCells(output, in_cd, out_cd);
        }
    }
}


// ****************************************************************************
//  Method: HashEntry::CreateOutputCells
//
//  Purpose:
//      Goes through each of the faces and has them output themselves as VTK 
//      objects.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
HashEntry::CreateOutputCells(vtkPolyData *output, vtkCellData *in_cd,
                             vtkCellData *out_cd)
{
    for (int i = 0 ; i < last_good_entry ; i++)
    {
        bool isQuad = (face_type & face_mask[i]) > 0;
        if (isQuad)
        {
            Quad *q = faces[i].quad;
            q->OutputCell(point_index, output, in_cd, out_cd);
        }
        else
        {
            Tri *t = faces[i].tri;
            t->OutputCell(point_index, output, in_cd, out_cd);
        }
    }
    if (extension != NULL)
    {
        extension->CreateOutputCells(output, in_cd, out_cd);
    }
}


vtkStandardNewMacro(vtkUnstructuredGridFacelistFilter); 


// ****************************************************************************
//  Method: vtkUnstructuredGridFacelistFilter::RequestData
//
//  Purpose:
//      Finds the faces that are external to the unstructured grid input.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jun 12 09:10:14 PDT 2003
//    Changed the estimate of how big a connectivity array to allocate.
//
//    Brad Whitlock, Fri Oct 1 17:11:47 PST 2004
//    Passed the field data through.
//
//    Eric Brugger, Fri Jan 27 14:31:40 PST 2012
//    I changed the routine to add cells to the polydata so that cells were
//    added in the order - vertex based, line based, polygon based, and strip
//    based. This was so that the cell data would be ordered in that fashion,
//    since that was necessary for the polydata to be rendered correctly.
//
//    Eric Brugger, Mon Jan 30 09:02:23 PST 2012
//    I changed the order in which polygon cells and polygons from solid
//    cells are output so that polygon cells are rendered on top of solid
//    cells. While technically not more correct, this gives the result users
//    would expect if a polygon cell was on the face of a solid cell.
//
// ****************************************************************************

int
vtkUnstructuredGridFacelistFilter::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
    vtkDebugMacro(<<"Executing geometry filter for unstructured grid input");

    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    //
    // Initialize some frequently used values.
    //
    vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkCellData *cd = input->GetCellData();
    vtkCellData *outputCD = output->GetCellData();
 
    //
    // We won't be doing anything to the points, so they can be passed right
    // through.
    //
    output->SetPoints(input->GetPoints());
    vtkPointData *pd = input->GetPointData();
    vtkPointData *outputPD = output->GetPointData();
    outputPD->PassData(pd);

    // Pass the field data through
    output->GetFieldData()->ShallowCopy(GetInput()->GetFieldData());

    int ntotalpts = input->GetNumberOfPoints();
    HashEntryList list(ntotalpts);

    //
    // This does the work of looping over all the cells and storing them in
    // our hash table.
    //
    int numVertexCells = 0;
    int numLineCells = 0;
    int numPolygonCells = 0;
    int numStripCells = 0;
    LoopOverAllCells(input, list, numVertexCells, numLineCells,
                     numPolygonCells, numStripCells);

    //
    // Count up how many output cells we will have.
    //
    int hashedFaces = list.GetNumberOfFaces();
    int numOutCells = numVertexCells + numLineCells + numPolygonCells +
                      numStripCells + hashedFaces;

    //
    // Now create our output cells.
    //
    output->Allocate(numOutCells, numOutCells*(4+1));
    outputCD->CopyAllocate(cd, numOutCells);
    if (numVertexCells > 0)
    {
        LoopOverVertexCells(input, output, cd, outputCD);
    }
    if (numLineCells > 0)
    {
        LoopOverLineCells(input, output, cd, outputCD);
    }
    list.CreateOutputCells(output, cd, outputCD);
    if (numPolygonCells > 0)
    {
        LoopOverPolygonCells(input, output, cd, outputCD);
    }
    if (numStripCells > 0)
    {
        LoopOverStripCells(input, output, cd, outputCD);
    }

    return 1;
}


// ****************************************************************************
//  Method: vtkUnstructuredGridFacelistFilter::FillInputPortInformation
//
// ****************************************************************************

int
vtkUnstructuredGridFacelistFilter::FillInputPortInformation(int,
    vtkInformation *info)
{
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
    return 1;
}


// ****************************************************************************
//  Method: vtkUnstructuredGridFacelistFilter::PrintSelf
//
// ****************************************************************************

void
vtkUnstructuredGridFacelistFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}


// ****************************************************************************
//  Function: LoopOverVertexCells
//
//  Purpose:
//      If there were cells in the unstructured grid that were vertex based,
//      then this routine is called to output them.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Eric Brugger, Fri Jan 27 14:31:40 PST 2012
//    I split the routine LoopOverPolygonalCells into LoopOverVertexCells,
//    LoopOverLineCells, LoopOverPolygonCells and LoopOverStripCells.
//
// ****************************************************************************

void
LoopOverVertexCells(vtkUnstructuredGrid *input, vtkPolyData *output,
                    vtkCellData *in_cd, vtkCellData *out_cd)
{
    vtkCellArray *Connectivity = input->GetCells();
    if (Connectivity == NULL)
    {
        return;
    }

    vtkIdType   cellId;
    vtkIdType   newCellId;
    vtkIdType   npts;
    const vtkIdType   *pts;
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
    {
        int cellType = input->GetCellType(cellId);
        switch (cellType)
        {
          case VTK_VERTEX:
          case VTK_POLY_VERTEX:
            newCellId = output->InsertNextCell(cellType, npts, pts);
            out_cd->CopyData(in_cd, cellId, newCellId);
            break;
        }
    }
}


// ****************************************************************************
//  Function: LoopOverLineCells
//
//  Purpose:
//      If there were cells in the unstructured grid that were line based,
//      then this routine is called to output them.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Brad Whitlock, Mon May 8 14:29:48 PST 2006
//    Added support for turning VTK_QUADRATIC_EDGE into VTK_POLY_LINE.
//
//    Eric Brugger, Fri Jan 27 14:31:40 PST 2012
//    I split the routine LoopOverPolygonalCells into LoopOverVertexCells,
//    LoopOverLineCells, LoopOverPolygonCells and LoopOverStripCells.
//
// ****************************************************************************

void
LoopOverLineCells(vtkUnstructuredGrid *input, vtkPolyData *output,
                  vtkCellData *in_cd, vtkCellData *out_cd)
{
    vtkCellArray *Connectivity = input->GetCells();
    if (Connectivity == NULL)
    {
        return;
    }

    vtkIdType   ids[3];
    vtkIdType   cellId;
    vtkIdType   newCellId;
    vtkIdType   npts;
    const vtkIdType   *pts;
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
    {
        int cellType = input->GetCellType(cellId);
        switch (cellType)
        {
          case VTK_LINE:
          case VTK_POLY_LINE:
            newCellId = output->InsertNextCell(cellType, npts, pts);
            out_cd->CopyData(in_cd, cellId, newCellId);
            break;

          case VTK_QUADRATIC_EDGE:
            ids[0] = pts[0];
            ids[1] = pts[2];
            ids[2] = pts[1];
            newCellId = output->InsertNextCell(VTK_POLY_LINE, 3, ids);
            out_cd->CopyData(in_cd, cellId, newCellId);
            break;
        }
    }
}


// ****************************************************************************
//  Function: LoopOverPolygonCells
//
//  Purpose:
//      If there were cells in the unstructured grid that were polygon based,
//      then this routine is called to output them.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Hank Childs, Wed Aug 25 16:11:24 PDT 2004
//    Since we are translating a pixel into a quad, make sure to tell the
//    output that it is of "quad" type.
//
//    Eric Brugger, Fri Jan 27 14:31:40 PST 2012
//    I split the routine LoopOverPolygonalCells into LoopOverVertexCells,
//    LoopOverLineCells, LoopOverPolygonCells and LoopOverStripCells.
//
// ****************************************************************************

void
LoopOverPolygonCells(vtkUnstructuredGrid *input, vtkPolyData *output,
                     vtkCellData *in_cd, vtkCellData *out_cd)
{
    vtkCellArray *Connectivity = input->GetCells();
    if (Connectivity == NULL)
    {
        return;
    }

    vtkIdType   ids[4];
    vtkIdType   cellId;
    vtkIdType   newCellId;
    vtkIdType   npts;
    const vtkIdType   *pts;
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
    {
        int cellType = input->GetCellType(cellId);
        switch (cellType)
        {
          case VTK_TRIANGLE:
          case VTK_QUAD:
          case VTK_POLYGON:
            newCellId = output->InsertNextCell(cellType, npts, pts);
            out_cd->CopyData(in_cd, cellId, newCellId);
            break;

          case VTK_PIXEL:
            ids[0] = pts[0];
            ids[1] = pts[1];
            ids[2] = pts[3];
            ids[3] = pts[2];
            newCellId = output->InsertNextCell(VTK_QUAD, npts, ids);
            out_cd->CopyData(in_cd, cellId, newCellId);
            break;
        }
    }
}


// ****************************************************************************
//  Function: LoopOverStripCells
//
//  Purpose:
//      If there were cells in the unstructured grid that were poly strip
//      based, then this routine is called to output them.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Eric Brugger, Fri Jan 27 14:31:40 PST 2012
//    I split the routine LoopOverPolygonalCells into LoopOverVertexCells,
//    LoopOverLineCells, LoopOverPolygonCells and LoopOverStripCells.
//
// ****************************************************************************

void
LoopOverStripCells(vtkUnstructuredGrid *input, vtkPolyData *output,
                   vtkCellData *in_cd, vtkCellData *out_cd)
{
    vtkCellArray *Connectivity = input->GetCells();
    if (Connectivity == NULL)
    {
        return;
    }

    vtkIdType   cellId;
    vtkIdType   newCellId;
    vtkIdType   npts;
    const vtkIdType   *pts;
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
    {
        int cellType = input->GetCellType(cellId);
        switch (cellType)
        {
          case VTK_TRIANGLE_STRIP:
            newCellId = output->InsertNextCell(cellType, npts, pts);
            out_cd->CopyData(in_cd, cellId, newCellId);
            break;
        }
    }
}


// ****************************************************************************
//  Function: LoopOverAllCells
//
//  Purpose:
//      Loops over all of the cells in the unstructured grid input and adds
//      them to the hash entry list.
//
//  Returns:    The number of polygonal cells contained in 'input'.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Brad Whitlock, Mon May 8 14:54:58 PST 2006
//    Added cases to add the faces of quadratic cells to the hash entry list
//    as sets of linear triangles.
//
//    Hank Childs, Fri Sep  8 14:38:54 PDT 2006
//    Add support for unexpected cell types.
//
//    Brad Whitlock, Thu Apr 29 14:10:51 PST 2010
//    I added quadratic pyramid and wedge.
//
//    Eric Brugger, Fri Jan 27 14:31:40 PST 2012
//    I modified the routine to return the number of vertex based cells, line
//    based cells, polygon based cells and strip based cells, instead of just
//    the sum of those numbers. 
//
// ****************************************************************************

void
LoopOverAllCells(vtkUnstructuredGrid *input, HashEntryList &list,
                 int &numVertexCells, int &numLineCells,
                 int &numPolygonCells, int &numStripCells)
{
    vtkCellArray *Connectivity = input->GetCells();
    if (Connectivity == NULL)
    {
        return;
    }

    numVertexCells = 0;
    numLineCells = 0;
    numPolygonCells = 0;
    numStripCells = 0;
    vtkIdType   cellId;
    vtkIdType   npts;
    const vtkIdType   *pts;
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
    {
        int cellType = input->GetCellType(cellId);
 
        switch (cellType)
        {
          case VTK_VERTEX:
          case VTK_POLY_VERTEX:
            numVertexCells++;
            break;
 
          case VTK_LINE:
          case VTK_POLY_LINE:
          case VTK_QUADRATIC_EDGE:
            numLineCells++;
            break;
 
          case VTK_TRIANGLE:
          case VTK_QUAD:
          case VTK_POLYGON:
          case VTK_PIXEL:
            numPolygonCells++;
            break;
 
          case VTK_TRIANGLE_STRIP:
            numStripCells++;
            break;
 
          case VTK_TETRA:
            AddTetrahedron(pts, cellId, list);
            break;

          case VTK_VOXEL:
            AddVoxel(pts, cellId, list);
            break;

          case VTK_HEXAHEDRON:
            AddHexahedron(pts, cellId, list);
            break;

          case VTK_WEDGE:
            AddWedge(pts, cellId, list);
            break;

          case VTK_PYRAMID:
            AddPyramid(pts, cellId, list);
            break;

          case VTK_QUADRATIC_TRIANGLE:
            AddQuadraticTriangle(pts, cellId, list);
            break;

          case VTK_QUADRATIC_QUAD:
            AddQuadraticQuad(pts, cellId, list);
            break;

          case VTK_QUADRATIC_TETRA:
            AddQuadraticTetrahedron(pts, cellId, list);
            break;

          case VTK_QUADRATIC_HEXAHEDRON:
            AddQuadraticHexahedron(pts, cellId, list);
            break;

          case VTK_QUADRATIC_PYRAMID:
            AddQuadraticPyramid(pts, cellId, list);
            break;

          case VTK_QUADRATIC_WEDGE:
            AddQuadraticWedge(pts, cellId, list);
            break;

          case VTK_QUADRATIC_LINEAR_QUAD:
            AddQuadraticLinearQuad(pts, cellId, list);
            break;

          case VTK_QUADRATIC_LINEAR_WEDGE:
            AddQuadraticLinearWedge(pts, cellId, list);
            break;

          case VTK_BIQUADRATIC_TRIANGLE:
            AddBiQuadraticTriangle(pts, cellId, list);
            break;         

          case VTK_BIQUADRATIC_QUAD:
            AddBiQuadraticQuad(pts, cellId, list);
            break;

          case VTK_BIQUADRATIC_QUADRATIC_WEDGE:
            AddBiQuadraticQuadraticWedge(pts, cellId, list);
            break;

          case VTK_BIQUADRATIC_QUADRATIC_HEXAHEDRON:
            AddBiQuadraticQuadraticHexahedron(pts, cellId, list);
            break;

          case VTK_TRIQUADRATIC_HEXAHEDRON:
            AddTriQuadraticHexahedron(pts, cellId, list);
            break;      

          default:
            AddUnknownCell(input->GetCell(cellId), cellId, list);
        }
    }
}

// ****************************************************************************
//  Function: AddTetrahedron
//
//  Purpose:
//      Adds all of the faces from a tetrahedron to our hash entry list.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
AddTetrahedron(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[2];
    nodes[1] = pts[1];
    nodes[2] = pts[0];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[2];
    nodes[2] = pts[0];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[1];
    nodes[2] = pts[2];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[0];
    nodes[2] = pts[1];
    list.AddTri(nodes, cellId);
}


// ****************************************************************************
//  Function: AddVoxel
//
//  Purpose:
//      Adds all of the faces from a voxel to our hash entry list.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
AddVoxel(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[0];
    nodes[1] = pts[4];
    nodes[2] = pts[6];
    nodes[3] = pts[2];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[1];
    nodes[1] = pts[3];
    nodes[2] = pts[7];
    nodes[3] = pts[5];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[0];
    nodes[1] = pts[1];
    nodes[2] = pts[5];
    nodes[3] = pts[4];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[2];
    nodes[1] = pts[6];
    nodes[2] = pts[7];
    nodes[3] = pts[3];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[1];
    nodes[1] = pts[0];
    nodes[2] = pts[2];
    nodes[3] = pts[3];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[5];
    nodes[2] = pts[7];
    nodes[3] = pts[6];
    list.AddQuad(nodes, cellId);
}


// ****************************************************************************
//  Function: AddHexahedron
//
//  Purpose:
//      Adds all of the faces from a hexahedron to our hash entry list.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
AddHexahedron(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[0];
    nodes[1] = pts[4];
    nodes[2] = pts[7];
    nodes[3] = pts[3];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[1];
    nodes[1] = pts[2];
    nodes[2] = pts[6];
    nodes[3] = pts[5];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[0];
    nodes[1] = pts[1];
    nodes[2] = pts[5];
    nodes[3] = pts[4];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[7];
    nodes[2] = pts[6];
    nodes[3] = pts[2];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[0];
    nodes[1] = pts[3];
    nodes[2] = pts[2];
    nodes[3] = pts[1];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[5];
    nodes[2] = pts[6];
    nodes[3] = pts[7];
    list.AddQuad(nodes, cellId);
}


// ****************************************************************************
//  Function: AddWedge
//
//  Purpose:
//      Adds all of the faces from a wedge to our hash entry list.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
AddWedge(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[0];
    nodes[1] = pts[1];
    nodes[2] = pts[2];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[5];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[0];
    nodes[1] = pts[3];
    nodes[2] = pts[4];
    nodes[3] = pts[1];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[1];
    nodes[1] = pts[4];
    nodes[2] = pts[5];
    nodes[3] = pts[2];
    list.AddQuad(nodes, cellId);
    nodes[0] = pts[2];
    nodes[1] = pts[5];
    nodes[2] = pts[3];
    nodes[3] = pts[0];
    list.AddQuad(nodes, cellId);
}


// ****************************************************************************
//  Function: AddPyramid
//
//  Purpose:
//      Adds all of the faces from a pyramid to our hash entry list.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

void
AddPyramid(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[0];
    nodes[1] = pts[1];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[1];
    nodes[1] = pts[2];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[2];
    nodes[1] = pts[3];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[0];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[0];
    nodes[1] = pts[3];
    nodes[2] = pts[2];
    nodes[3] = pts[1];
    list.AddQuad(nodes, cellId);
}

// ****************************************************************************
// Function: AddQuadraticTriangle
//
// Purpose: 
//   Breaks up the quadratic triangle into linear triangles and adds them
//   to the hash entry list.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 8 14:52:30 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
AddQuadraticTriangle(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[3];
    nodes[0] = pts[0];
    nodes[1] = pts[3];
    nodes[2] = pts[5];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[1];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[5];
    nodes[1] = pts[4];
    nodes[2] = pts[2];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[4];
    nodes[2] = pts[5];
    list.AddTri(nodes, cellId);
}

// ****************************************************************************
// Function: AddQuadraticQuad
//
// Purpose: 
//   Breaks up the quadratic quad into linear triangles and adds them
//   to the hash entry list.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 8 14:52:30 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
AddQuadraticQuad(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[3];
    nodes[0] = pts[0];
    nodes[1] = pts[4];
    nodes[2] = pts[7];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[1];
    nodes[2] = pts[5];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[5];
    nodes[1] = pts[2];
    nodes[2] = pts[6];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[6];
    nodes[1] = pts[3];
    nodes[2] = pts[7];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[6];
    nodes[2] = pts[7];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[5];
    nodes[2] = pts[6];
    list.AddTri(nodes, cellId);
}

// ****************************************************************************
// Function: AddQuadraticTetrahedron
//
// Purpose: 
//   Breaks up the faces of the quadratic tet into linear triangles and adds
//   them to the hash entry list.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 8 14:52:30 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
AddQuadraticTetrahedron(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    // Break up the surface of the quadratic tet into triangles.
    const int triangles[][3] = {
        {0,4,7},{4,1,8},{4,8,7},{8,3,7},
        {2,6,9},{6,0,7},{6,7,9},{7,3,9},
        {1,5,8},{5,2,9},{5,9,8},{9,3,8},
        {1,4,5},{4,0,6},{4,6,5},{6,2,5}
    };
    vtkIdType nodes[3];
    for(int i = 0; i < 16; ++i)
    {
        nodes[0] = pts[triangles[i][0]];
        nodes[1] = pts[triangles[i][1]];
        nodes[2] = pts[triangles[i][2]];
        list.AddTri(nodes, cellId);
    }
}

// ****************************************************************************
// Function: AddQuadraticHexahedron
//
// Purpose: 
//   Breaks up the faces of the quadratic hex into linear triangles and adds
//   them to the hash entry list.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 8 14:52:30 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
AddQuadraticHexahedron(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    // Break up the surface of the quadratic hex into triangles.
    const int triangles[][3] = {
        {0,8,16},{8,1,17},{17,5,12},{12,4,16},{8,17,12},{8,12,16},
        {1,9,17},{9,2,18},{18,6,13},{13,5,17},{9,18,13},{9,13,17},
        {2,10,18},{10,3,19},{19,7,14},{14,18,6},{10,19,14},{10,14,18},
        {3,11,19},{11,0,16},{16,4,15},{15,7,19},{11,16,15},{11,15,19},
        {4,12,15},{12,5,13},{13,6,14},{14,7,15},{12,13,14},{12,14,15},
        {3,10,11},{10,2,9},{9,1,8},{8,0,11},{10,9,8},{10,8,11}
    };

    vtkIdType nodes[3];
    for(int i = 0; i < 36; ++i)
    {
        nodes[0] = pts[triangles[i][0]];
        nodes[1] = pts[triangles[i][1]];
        nodes[2] = pts[triangles[i][2]];
        list.AddTri(nodes, cellId);
    }
}

// ****************************************************************************
// Function: AddQuadraticPyramid
//
// Purpose: 
//   Breaks up the faces of the quadratic pyramid into linear triangles and
//   adds them to the list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 29 14:32:38 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
AddQuadraticPyramid(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    const int triangles[][3] = {
       {0,5,9},{5,10,9},{5,1,10},{9,10,4},
       {1,6,10},{6,11,10},{6,2,11},{10,11,4},
       {2,7,11},{7,12,11},{7,3,12},{11,12,4},
       {3,8,12},{8,9,12},{8,0,9},{12,9,4},
       {3,7,8},{8,5,0},{5,1,6},{7,2,6}
    };
    const int quads[][4] = {
       {8,7,6,5}
    };

    vtkIdType nodes[4];
    for(int i = 0; i < 20; ++i)
    {
        nodes[0] = pts[triangles[i][0]];
        nodes[1] = pts[triangles[i][1]];
        nodes[2] = pts[triangles[i][2]];
        list.AddTri(nodes, cellId);
    }

    nodes[0] = pts[quads[0][0]];
    nodes[1] = pts[quads[0][1]];
    nodes[2] = pts[quads[0][2]];
    nodes[3] = pts[quads[0][3]];
    list.AddQuad(nodes, cellId);
}

// ****************************************************************************
// Function: AddQuadraticWedge
//
// Purpose: 
//   Breaks up the faces of the quadratic wedge into linear triangles and
//   adds them to the list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 29 14:32:38 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
AddQuadraticWedge(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    const int triangles[][3] = {
        {0,6,8},{6,7,8},{6,1,7},{8,7,2},
        {4,9,10},{9,10,11},{9,3,11},{10,11,5},
        {3,12,11},{11,14,5},{12,0,8},{14,8,2},
        {5,14,10},{10,13,4},{14,2,7},{13,7,1},
        {4,13,9},{9,12,3},{13,1,6},{12,6,0}
    };
    const int quads[][4] = {
        {11,12,8,14},
        {10,14,7,13},
        {9,13,6,12}
    };
    vtkIdType nodes[4];
    for(int i = 0; i < 20; ++i)
    {
        nodes[0] = pts[triangles[i][0]];
        nodes[1] = pts[triangles[i][1]];
        nodes[2] = pts[triangles[i][2]];
        list.AddTri(nodes, cellId);
    }
    for(int i = 0; i < 3; ++i)
    {
        nodes[0] = pts[quads[i][0]];
        nodes[1] = pts[quads[i][1]];
        nodes[2] = pts[quads[i][2]];
        nodes[3] = pts[quads[i][3]];
        list.AddQuad(nodes, cellId);
    }
}

// ****************************************************************************
// Function: AddQuadraticLinearQuad
//
// Purpose:
//   Breaks up the faces of the quadratic linear quad into linear triangles and
//   adds them to the list.
//
// Programmer: Kenneth Leiter
// Creation:   Sun Feb 20 11:09:27 PST 2011
//
// Modifications:
//
// ****************************************************************************

void
AddQuadraticLinearQuad(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[3];
    nodes[0] = pts[0];
    nodes[1] = pts[5];
    nodes[2] = pts[3];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[0];
    nodes[1] = pts[4];
    nodes[2] = pts[5];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[1];
    nodes[2] = pts[5];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[1];
    nodes[1] = pts[2];
    nodes[2] = pts[5];
    list.AddTri(nodes, cellId);
}

// ****************************************************************************
// Function: AddQuadraticLinearWedge
//
// Purpose:
//   Breaks up the faces of the quadratic linear wedge into linear triangles and
//   adds them to the list.
//
// Programmer: Kenneth Leiter
// Creation:   Sun Feb 20 12:56:55 PST 2011
//
// Modifications:
//
// ****************************************************************************

void
AddQuadraticLinearWedge(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    const int triangles[][3] = {
        {0,6,8},{6,7,8},{6,1,7},{8,7,2},
        {4,9,10},{9,11,10},{9,3,11},{10,11,5},
    };
    const int quads[][4] = {
        {0,2,5,3},
        {0,3,4,1},
        {1,4,5,2}
    };
    vtkIdType nodes[4];
    for(int i = 0; i < 8; ++i)
    {
        nodes[0] = pts[triangles[i][0]];
        nodes[1] = pts[triangles[i][1]];
        nodes[2] = pts[triangles[i][2]];
        list.AddTri(nodes, cellId);
    }
    for(int i = 0; i < 3; ++i)
    {
        nodes[0] = pts[quads[i][0]];
        nodes[1] = pts[quads[i][1]];
        nodes[2] = pts[quads[i][2]];
        nodes[3] = pts[quads[i][3]];
        list.AddQuad(nodes, cellId);
    }
}

// ****************************************************************************
// Function: AddBiQuadraticTriangle
//
// Purpose:
//   Breaks up the bi quadratic triangle into linear triangles and adds them
//   to the hash entry list.
//
// Programmer: Kenneth Leiter
// Creation:   Mon Feb 21 09:11:40 PST 2006
//
// Modifications:
//
// ****************************************************************************

void
AddBiQuadraticTriangle(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[3];
    nodes[0] = pts[0];
    nodes[1] = pts[3];
    nodes[2] = pts[6];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[1];
    nodes[2] = pts[6];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[1];
    nodes[1] = pts[4];
    nodes[2] = pts[6];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[2];
    nodes[2] = pts[5];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[5];
    nodes[2] = pts[6];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[5];
    nodes[1] = pts[0];
    nodes[2] = pts[6];
    list.AddTri(nodes, cellId);
}

// ****************************************************************************
// Function: AddBiQuadraticQuad
//
// Purpose:
//   Breaks up the bi quadratic quad into linear triangles and adds them
//   to the hash entry list.
//
// Programmer: Kenneth Leiter
// Creation:   Mon Feb 21 10:01:41 PST 2006
//
// Modifications:
//
// ****************************************************************************

void
AddBiQuadraticQuad(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    vtkIdType nodes[3];
    nodes[0] = pts[0];
    nodes[1] = pts[4];
    nodes[2] = pts[8];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[4];
    nodes[1] = pts[1];
    nodes[2] = pts[8];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[1];
    nodes[1] = pts[5];
    nodes[2] = pts[8];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[5];
    nodes[1] = pts[2];
    nodes[2] = pts[8];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[2];
    nodes[1] = pts[6];
    nodes[2] = pts[8];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[6];
    nodes[1] = pts[3];
    nodes[2] = pts[8];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[3];
    nodes[1] = pts[7];
    nodes[2] = pts[8];
    list.AddTri(nodes, cellId);
    nodes[0] = pts[7];
    nodes[1] = pts[0];
    nodes[2] = pts[8];
    list.AddTri(nodes, cellId);
}

// ****************************************************************************
// Function: AddBiQuadraticQuadraticWedge
//
// Purpose:
//   Breaks up the faces of the bi quadratic wedge into linear triangles and
//   adds them to the list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 29 14:32:38 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
AddBiQuadraticQuadraticWedge(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    const int triangles[][3] = {
        {0,6,8},{6,7,8},{6,1,7},{8,7,2},
        {4,9,10},{9,10,11},{9,3,11},{10,11,5},
        {3,12,11},{11,14,5},{12,0,8},{14,8,2},
        {5,14,10},{10,13,4},{14,2,7},{13,7,1},
        {4,13,9},{9,12,3},{13,1,6},{12,6,0},
        {6,15,13},{13,15,9},{9,15,12},{12,15,6},
        {14,17,8},{11,17,14},{12,17,11},{8,17,12},
        {14,7,16},{10,14,16},{13,10,16},{7,13,16}
    };
    vtkIdType nodes[3];
    for(int i = 0; i < 32; ++i)
    {
        nodes[0] = pts[triangles[i][0]];
        nodes[1] = pts[triangles[i][1]];
        nodes[2] = pts[triangles[i][2]];
        list.AddTri(nodes, cellId);
    }
}

// ****************************************************************************
// Function: AddBiQuadraticQuadraticHexahedron
//
// Purpose:
//   Breaks up the faces of the bi quadratic hexahedron into linear triangles 
//   and adds them to the list.
//
// Programmer: Kenneth Leiter
// Creation:   Mon Feb 21 14:55:30 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
AddBiQuadraticQuadraticHexahedron(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    const int triangles[][3] = {
        {0,8,16},{8,1,17},{17,5,12},{12,4,16},
        {16,8,22},{8,17,22},{17,12,22},{12,16,22},
        {1,9,17},{9,2,18},{18,6,13},{13,5,17},
        {17,9,21},{9,18,21},{18,13,21},{13,17,21},
        {2,10,18},{10,3,19},{19,7,14},{14,6,18},
        {18,10,23},{10,19,23},{19,14,23},{14,18,23},
        {3,11,19},{11,0,16},{16,4,15},{15,7,19},
        {19,11,20},{11,16,20},{16,15,20},{15,19,20},
        {4,12,15},{12,5,13},{13,6,14},{14,7,15},
        {12,13,14},{12,14,15},
        {3,10,11},{10,2,9},{9,1,8},{8,0,11},
        {10,9,8},{10,8,11}        
    };
    vtkIdType nodes[3];
    for(int i = 0; i < 44; ++i)
    {
        nodes[0] = pts[triangles[i][0]];
        nodes[1] = pts[triangles[i][1]];
        nodes[2] = pts[triangles[i][2]];
        list.AddTri(nodes, cellId);
    }
}

// ****************************************************************************
// Function: AddTriQuadraticHexahedron
//
// Purpose:
//   Breaks up the faces of the tri quadratic hexahedron into linear triangles 
//   and adds them to the list.
//
// Programmer: Kenneth Leiter
// Creation:   Mon Feb 21 14:55:30 PST 2010
//
// Modifications:
//
// ****************************************************************************

void
AddTriQuadraticHexahedron(const vtkIdType *pts, int cellId, HashEntryList &list)
{
    const int triangles[][3] = {
        {0,8,16},{8,1,17},{17,5,12},{12,4,16},
        {16,8,22},{8,17,22},{17,12,22},{12,16,22},
        {1,9,17},{9,2,18},{18,6,13},{13,5,17},
        {17,9,21},{9,18,21},{18,13,21},{13,17,21},
        {2,10,18},{10,3,19},{19,7,14},{14,6,18},
        {18,10,23},{10,19,23},{19,14,23},{14,18,23},
        {3,11,19},{11,0,16},{16,4,15},{15,7,19},
        {19,11,20},{11,16,20},{16,15,20},{15,19,20},
        {4,12,15},{12,5,13},{13,6,14},{14,7,15},
        {15,12,25},{12,13,25},{13,14,25},{14,15,25},
        {3,10,11},{10,2,9},{9,1,8},{8,0,11},
        {9,8,24},{8,11,24},{11,10,24},{10,9,24}        
    };
    vtkIdType nodes[3];
    for(int i = 0; i < 48; ++i)
    {
        nodes[0] = pts[triangles[i][0]];
        nodes[1] = pts[triangles[i][1]];
        nodes[2] = pts[triangles[i][2]];
        list.AddTri(nodes, cellId);
    }
}

// ****************************************************************************
// Function: AddUnknownCell
//
// Purpose: 
//     Adds a cell of unknown type by using VTK general interface methods.
//
// Programmer: Hank Childs
// Creation:   September 7, 2006
//
// Modifications:
//   
//   Hank Childs, Thu Jul  9 08:09:26 PDT 2009
//   Add support for polygons.
//
// ****************************************************************************

void
AddUnknownCell(vtkCell *cell, int cellId, HashEntryList &list)
{
    int nFaces = cell->GetNumberOfFaces();
    vtkIdType nodes[4];
    for (int i = 0 ; i < nFaces ; i++)
    {
        vtkCell *face = cell->GetFace(i);
        if (face->GetCellType() == VTK_TRIANGLE)
        {
            nodes[0] = face->GetPointId(0);
            nodes[1] = face->GetPointId(1);
            nodes[2] = face->GetPointId(2);
            list.AddTri(nodes, cellId);
        }
        else if (face->GetCellType() == VTK_QUAD)
        {
            nodes[0] = face->GetPointId(0);
            nodes[1] = face->GetPointId(1);
            nodes[2] = face->GetPointId(2);
            nodes[3] = face->GetPointId(3);
            list.AddQuad(nodes, cellId);
        }
        else if (face->GetCellType() == VTK_POLYGON)
        {
            vtkIdList *tris = vtkIdList::New();
            vtkPolygon *polygon = (vtkPolygon *) face;
            polygon->Triangulate(tris);
            int numTris = tris->GetNumberOfIds() / 3;
            for (int i = 0 ; i < numTris ; i++)
            {
                nodes[0] = polygon->GetPointId(tris->GetId(3*i+0));
                nodes[1] = polygon->GetPointId(tris->GetId(3*i+1));
                nodes[2] = polygon->GetPointId(tris->GetId(3*i+2));
                list.AddTri(nodes, cellId);
            }
            tris->Delete();
        }
    }
}

