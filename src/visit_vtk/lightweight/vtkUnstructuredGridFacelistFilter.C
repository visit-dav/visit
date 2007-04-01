// ************************************************************************* //
//                      vtkUnstructuredGridFacelistFilter.C                  //
// ************************************************************************* //

#include "vtkUnstructuredGridFacelistFilter.h"
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

#include <vector>
using std::vector;


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

    inline Quad     *GetFreeQuad()
                         {
                             if (freequadindex <= 0)
                             {
                                 AllocateQuadPool();
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

    void             AllocateQuadPool(void);
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

    inline Tri      *GetFreeTri()
                         {
                             if (freetriindex <= 0)
                             {
                                 AllocateTriPool();
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

    void            AllocateTriPool(void);
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

    int            AssignNodes(const int *);
    bool           Equals(Quad *);
    bool           Equals(Tri *);
    void           AddInRemainingTriangle(Tri *, int);
    inline void    ReRegisterMemory(void)
                         {
                             MemoryManager->ReRegisterQuad(this);
                         }

    inline void    SetOriginalZone(const int &oz) { orig_zone = oz; };
    inline int     GetOriginalZone(void) { return orig_zone; };

    void           OutputCell(int,vtkPolyData *, vtkCellData *, vtkCellData *);

    static void    RegisterHashEntryList(HashEntryList *);
    static void    RegisterMemoryManager(QuadMemoryManager *);
    static void    SetNumberOfPoints(int np) { npts = np; };

  protected:
    unsigned char ordering_case;
    int           nodes[3];
    int           orig_zone;

    static HashEntryList     *list;
    static QuadMemoryManager *MemoryManager;
    static int                npts;

    void          AddInRemainingTriangle(int, int);
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

static int quad_reorder_list[24][4] = 
    { { -1, 0, 1, 2 }, { -1, 0, 2, 1 }, { -1, 1, 0, 2 }, { -1, 2, 0, 1 },
      { -1, 1, 2, 0 }, { -1, 2, 1, 0 },
      { 0, -1, 1, 2 }, { 0, -1, 2, 1 }, { 1, -1, 0, 2 }, { 2, -1, 0, 1 },
      { 1, -1, 2, 0 }, { 2, -1, 1, 0 },
      { 0, 1, -1, 2 }, { 0, 2, -1, 1 }, { 1, 0, -1, 2 }, { 2, 0, -1, 1 },
      { 1, 2, -1, 0 }, { 2, 1, -1, 0 },
      { 0, 1, 2, -1 }, { 0, 2, 1, -1 }, { 1, 0, 2, -1 }, { 2, 0, 1, -1 },
      { 1, 2, 0, -1 }, { 2, 1, 0, -1 } 
    };

static int quad_map_back_list[24][3] =
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

    int            AssignNodes(const int *);
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
                           MemoryManager->ReRegisterTri(this);
                       }

    inline void    SetOriginalZone(const int &oz) { orig_zone = oz; };
    inline int     GetOriginalZone(void) { return orig_zone; };

    void           OutputCell(int,vtkPolyData *, vtkCellData *, vtkCellData *);

    static void    RegisterMemoryManager(TriMemoryManager *);
    static void    SetNumberOfPoints(int np) { npts = np; };


  protected:
    unsigned char ordering_case;
    int           nodes[2];
    int           orig_zone;

    static TriMemoryManager *MemoryManager;
    static int               npts;
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
    static void    RegisterMemoryManager(HashEntryMemoryManager *mm)
                          { MemoryManager = mm; };
    static void    RegisterHashEntryList(HashEntryList *hel)
                          { list = hel; };

    void           CreateOutputCells(vtkPolyData*, vtkCellData*, vtkCellData*);

  protected:
    Face           faces[FACES_PER_HASH_ENTRY];
    int            point_index;
    unsigned char  last_good_entry;
    unsigned char  face_type;
    HashEntry     *extension;
   
    static HashEntryMemoryManager *MemoryManager;
    static HashEntryList          *list;

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

    inline HashEntry    *GetHashEntry()
                         {
                             if (currentHash+1 >= POOL_SIZE)
                             {
                                 AllocateHashEntryPool();
                             }
                             HashEntry *rv = currentHashPool + currentHash;
                             currentHash++;
                             return rv;
                         }

  protected:
    HashEntry      *currentHashPool;
    int             currentHash;

    std::vector<HashEntry *> hashpool;

    void            AllocateHashEntryPool(void);
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

    void        AddTri(const int *, int orig_zone);
    void        AddQuad(const int *, int orig_zone);

    inline void RemoveFace(void) { nfaces--; };
    int         GetNumberOfFaces(void) { return nfaces; };

    void        CreateOutputCells(vtkPolyData *, vtkCellData *, vtkCellData *);

  protected:
    HashEntry            **list;
    int                    nhashes;
    int                    npts;
    int                    nfaces;
    QuadMemoryManager      qmm;
    TriMemoryManager       tmm;
    HashEntryMemoryManager hemm;
};


//
// Declare these static members.
//
HashEntryList          *Quad::list = NULL;
QuadMemoryManager      *Quad::MemoryManager = NULL;
int                     Quad::npts = 0;
int                     Tri::npts = 0;
TriMemoryManager       *Tri::MemoryManager  = NULL;
HashEntryList          *HashEntry::list = NULL;
HashEntryMemoryManager *HashEntry::MemoryManager = NULL;


//
// Function prototypes
//

static void AddTetrahedron(vtkIdType *, int, HashEntryList &);
static void AddWedge(vtkIdType *, int, HashEntryList &);
static void AddPyramid(vtkIdType *, int, HashEntryList &);
static void AddHexahedron(vtkIdType *, int, HashEntryList &);
static void AddVoxel(vtkIdType *, int, HashEntryList &);
static int  LoopOverAllCells(vtkUnstructuredGrid *, HashEntryList &);
static void LoopOverPolygonalCells(vtkUnstructuredGrid *, vtkPolyData *,
                                   vtkCellData *, vtkCellData *);


// ****************************************************************************
//  Method: Quad::RegisterHashEntryList
//
//  Purpose:
//      Registers the hash entry list with the Quad class.  This is necessary
//      because a triangle may collide with a quadrilateral and the result
//      (another triangle) needs to be added back into the hash as its own
//      entry.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
Quad::RegisterHashEntryList(HashEntryList *l)
{
    list = l;
}


// ****************************************************************************
//  Method: Quad::RegisterMemoryManager
//
//  Purpose:
//      Registers the memory manager with the Quad class.  Whenever a quad is
//      freed from the hash, the newly freed quad is put back on a queue with
//      the memory manager via this reference.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
Quad::RegisterMemoryManager(QuadMemoryManager *mm)
{
    MemoryManager = mm;
}


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
// ****************************************************************************

int
Quad::AssignNodes(const int *n)
{
    int smallest = 0;
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

    if (biggest == 3)
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
    if (biggest == 2)
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
    if (biggest == 1)
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
    if (biggest == 0)
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
    int *list = quad_reorder_list[ordering_case];
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
    int orig_quad_index = quad_map_back_list[ordering_case][n];
    int *neighbors = quad_reorder_list[ordering_case];

    int n_list[3];
    n_list[0] = neighbors[(orig_quad_index+3)%4];
    n_list[1] = neighbors[orig_quad_index];
    n_list[2] = neighbors[(orig_quad_index+1)%4];
    int tmp_nodes[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        tmp_nodes[i] = (n_list[i] == -1 ? node_0 : nodes[n_list[i]]);
    }
    list->AddTri(tmp_nodes, orig_zone);
}


// ****************************************************************************
//  Method: Tri::RegisterMemoryManager
//
//  Purpose:
//      Registers the memory manager with the Tri class.  Whenever a tri is
//      freed from the hash, the newly freed tri is put back on a queue with
//      the memory manager via this reference.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
Tri::RegisterMemoryManager(TriMemoryManager *mm)
{
    MemoryManager = mm;
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

int
Tri::AssignNodes(const int *n)
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
// ****************************************************************************

HashEntry::HashEntry()
{
    point_index = -1;
    last_good_entry = 0;
    extension = NULL;
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
                list->RemoveFace();
                list->RemoveFace();
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
                list->RemoveFace();
                list->RemoveFace();
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
                list->RemoveFace();
                list->RemoveFace();
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
                list->RemoveFace();
                list->RemoveFace();
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
        extension = MemoryManager->GetHashEntry();
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
        extension = MemoryManager->GetHashEntry();
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
    AllocateHashEntryPool();
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
    int size = hashpool.size();
    for (int i = 0 ; i < size ; i++)
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
HashEntryMemoryManager::AllocateHashEntryPool(void)
{
    currentHashPool = new HashEntry[POOL_SIZE];
    currentHash = 0;
    hashpool.push_back(currentHashPool);
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
    int npools = quadpool.size();
    for (int i = 0 ; i < npools ; i++)
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
QuadMemoryManager::AllocateQuadPool(void)
{
    if (freequadindex == 0)
    {
        Quad *newlist = new Quad[POOL_SIZE];
        quadpool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
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
    int npools = tripool.size();
    for (int i = 0 ; i < npools ; i++)
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
TriMemoryManager::AllocateTriPool(void)
{
    if (freetriindex <= 0)
    {
        Tri *newlist = new Tri[POOL_SIZE];
        tripool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
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
    HashEntry::RegisterMemoryManager(&hemm);
    HashEntry::RegisterHashEntryList(this);
    Quad::RegisterMemoryManager(&qmm);
    Tri::RegisterMemoryManager(&tmm);
    Quad::RegisterHashEntryList(this);
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
HashEntryList::AddTri(const int *node_list, int orig_zone)
{
    nfaces++;
    Tri *tri = tmm.GetFreeTri();
    int hash_index = tri->AssignNodes(node_list);
    tri->SetOriginalZone(orig_zone);
    if (list[hash_index] == NULL)
    {
        list[hash_index] = hemm.GetHashEntry();
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
HashEntryList::AddQuad(const int *node_list, int orig_zone)
{
    nfaces++;
    Quad *quad = qmm.GetFreeQuad();
    int hash_index = quad->AssignNodes(node_list);
    quad->SetOriginalZone(orig_zone);
    if (list[hash_index] == NULL)
    {
        list[hash_index] = hemm.GetHashEntry();
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
        bool isQuad = face_type & face_mask[i];
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

void
vtkUnstructuredGridFacelistFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}

// ****************************************************************************
//  Method: vtkUnstructuredGridFacelistFilter::Execute
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
// ****************************************************************************

void
vtkUnstructuredGridFacelistFilter::Execute()
{
    vtkDebugMacro(<<"Executing geometry filter for unstructured grid input");

    vtkUnstructuredGrid *input= (vtkUnstructuredGrid *)this->GetInput();
    vtkCellData *cd = input->GetCellData();
    vtkPolyData *output = this->GetOutput();
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
    int numPolygonalCells = LoopOverAllCells(input, list);

    //
    // Count up how many output cells we will have.
    //
    int hashedFaces = list.GetNumberOfFaces();
    int numOutCells = numPolygonalCells + hashedFaces;

    //
    // Now create our output cells.
    //
    output->Allocate(numOutCells, numOutCells*(4+1));
    outputCD->CopyAllocate(cd, numOutCells);
    list.CreateOutputCells(output, cd, outputCD);
    if (numPolygonalCells > 0)
    {
        LoopOverPolygonalCells(input, output, cd, outputCD);
    }
}


// ****************************************************************************
//  Function: LoopOverPolygonalCells
//
//  Purpose:
//      If there were cells in the unstructured grid that were polygonal (ie
//      beams, tris, etc, as opposed to tets, hexes, etc), then this called to
//      output them.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Aug 25 16:11:24 PDT 2004
//    Since we are translating a pixel into a quad, make sure to tell the
//    output that it is of "quad" type.
//
// ****************************************************************************

void
LoopOverPolygonalCells(vtkUnstructuredGrid *input, vtkPolyData *output,
                       vtkCellData *in_cd, vtkCellData *out_cd)
{
    vtkCellArray *Connectivity = input->GetCells();
    if (Connectivity == NULL)
    {
        return;
    }

    vtkIdType   pixel_ids[4];
    vtkIdType   cellId;
    vtkIdType   newCellId;
    vtkIdType   npts;
    vtkIdType   *pts;
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
    {
        int cellType = input->GetCellType(cellId);
        switch (cellType)
        {
          case VTK_VERTEX:
          case VTK_POLY_VERTEX:
          case VTK_LINE:
          case VTK_POLY_LINE:
          case VTK_TRIANGLE:
          case VTK_TRIANGLE_STRIP:
          case VTK_QUAD:
          case VTK_POLYGON:
            newCellId = output->InsertNextCell(cellType, npts, pts);
            out_cd->CopyData(in_cd, cellId, newCellId);
            break;

          case VTK_PIXEL:
            pixel_ids[0] = pts[0];
            pixel_ids[1] = pts[1];
            pixel_ids[2] = pts[3];
            pixel_ids[3] = pts[2];
            newCellId = output->InsertNextCell(VTK_QUAD, npts, pixel_ids);
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
// ****************************************************************************

int
LoopOverAllCells(vtkUnstructuredGrid *input, HashEntryList &list)
{
    vtkCellArray *Connectivity = input->GetCells();
    if (Connectivity == NULL)
    {
        return 0;
    }

    int         numPolygonalCells = 0;
    vtkIdType   cellId;
    vtkIdType   npts;
    vtkIdType   *pts;
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
    {
        int cellType = input->GetCellType(cellId);
 
        switch (cellType)
        {
          case VTK_VERTEX:
          case VTK_POLY_VERTEX:
          case VTK_LINE:
          case VTK_POLY_LINE:
          case VTK_TRIANGLE:
          case VTK_TRIANGLE_STRIP:
          case VTK_QUAD:
          case VTK_POLYGON:
          case VTK_PIXEL:
            numPolygonalCells++;
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
        }
    }

    return numPolygonalCells;
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
AddTetrahedron(vtkIdType *pts, int cellId, HashEntryList &list)
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
AddVoxel(vtkIdType *pts, int cellId, HashEntryList &list)
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
AddHexahedron(vtkIdType *pts, int cellId, HashEntryList &list)
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
AddWedge(vtkIdType *pts, int cellId, HashEntryList &list)
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
AddPyramid(vtkIdType *pts, int cellId, HashEntryList &list)
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


