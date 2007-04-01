// ************************************************************************* //
//                      vtkUnstructuredGridBoundaryFilter.C                  //
// ************************************************************************* //

#include "vtkUnstructuredGridBoundaryFilter.h"

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
#define FREE_LINE_LIST_SIZE 16384
#define FACES_PER_HASH_ENTRY 8
#define LINES_PER_HASH_ENTRY 8
#define POOL_SIZE 256

#if FACES_PER_HASH_ENTRY > 8
   #error Bits are packed into an unsigned char.  8 is maximum.
#endif

//
// Forward declare some of types used to hash faces.
//
class BQuad;
class BQuadMemoryManager;
class BTri;
class BTriMemoryManager;
class BHashEntry;
class BHashEntryList;
class BHashEntryMemoryManager;

class BLine;
class BLineMemoryManager;
class BHashEntry2D;
class BHashEntryList2D;
class BHashEntryMemoryManager2D;


// ****************************************************************************
//  Class: BQuadMemoryManager
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

class BQuadMemoryManager
{
  public:
                     BQuadMemoryManager();
    virtual         ~BQuadMemoryManager();

    inline BQuad    *GetFreeQuad()
                         {
                             if (freequadindex <= 0)
                             {
                                 AllocateQuadPool();
                             }
                             freequadindex--;
                             return freequadlist[freequadindex];
                         }

    inline void      ReRegisterQuad(BQuad *q)
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
    BQuad           *freequadlist[FREE_FACE_LIST_SIZE];
    int              freequadindex;

    std::vector<BQuad *> quadpool;

    void             AllocateQuadPool(void);
};


// ****************************************************************************
//  Class: BTriMemoryManager
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

class BTriMemoryManager
{
  public:
                     BTriMemoryManager();
    virtual         ~BTriMemoryManager();

    inline BTri      *GetFreeTri()
                         {
                             if (freetriindex <= 0)
                             {
                                 AllocateTriPool();
                             }
                             freetriindex--;
                             return freetrilist[freetriindex];
                         }

    inline void      ReRegisterTri(BTri *q)
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
    BTri            *freetrilist[FREE_FACE_LIST_SIZE];
    int             freetriindex;

    std::vector<BTri *> tripool;

    void            AllocateTriPool(void);
};


// ****************************************************************************
//  Class: BLineMemoryManager
//
//  Purpose:
//      This provides a pool of lines that can be used for hashing.  When a
//      line is released from the hash, it is sent back to this module so
//      that it can be used again.  This is cache-friendly.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

class BLineMemoryManager
{
  public:
                     BLineMemoryManager();
    virtual         ~BLineMemoryManager();

    inline BLine    *GetFreeLine()
                         {
                             if (freelineindex <= 0)
                             {
                                 AllocateLinePool();
                             }
                             freelineindex--;
                             return freelinelist[freelineindex];
                         }

    inline void      ReRegisterLine(BLine *q)
                         {
                             if (freelineindex >= FREE_LINE_LIST_SIZE-1)
                             {
                                 // We've got plenty.
                                 return;
                             }
                             freelinelist[freelineindex] = q;
                             freelineindex++;
                         }

  protected:
    BLine            *freelinelist[FREE_LINE_LIST_SIZE];
    int               freelineindex;

    std::vector<BLine*> linepool;

    void              AllocateLinePool(void);
};


// ****************************************************************************
//  Class: BQuad
//
//  Purpose:
//      A representation of a quadrilateral.  For efficient caching, the
//      indices are sorted in numerical order.  The lowest indexed node is
//      dropped, since that serves as the key into the hash.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Allowed cell value and matched flag.
//
// ****************************************************************************

class BQuad
{
    friend class   BTri;

  public:
                   BQuad() { ordering_case = 255; matched = false; };

    int            AssignNodes(const int *);
    bool           Equals(BQuad *);
    bool           Equals(BTri *);
    void           AddInRemainingTriangle(BTri *, int);
    inline void    ReRegisterMemory(void)
                         {
                             MemoryManager->ReRegisterQuad(this);
                         }

    inline void    SetOriginalZone(const int &oz) { orig_zone = oz; };
    inline int     GetOriginalZone(void) { return orig_zone; };
    inline void    SetCellValue(const int &cv) { cell_value = cv; };
    inline int     GetCellValue(void) { return cell_value; };
    inline void    SetMatched(bool m) { matched = m; };
    inline bool    GetMatched() { return matched; };

    void           OutputCell(int,vtkPolyData *, vtkCellData *, vtkCellData *);

    static void    RegisterBHashEntryList(BHashEntryList *);
    static void    RegisterMemoryManager(BQuadMemoryManager *);
    static void    SetNumberOfPoints(int np) { npts = np; };

  protected:
    unsigned char ordering_case;
    int           nodes[3];
    int           orig_zone;
    int           cell_value;
    bool          matched;

    static BHashEntryList     *list;
    static BQuadMemoryManager *MemoryManager;
    static int                 npts;

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
//  Class: BTri
//
//  Purpose:
//      A representation of a triangle.  For efficient caching, the
//      indices are sorted in numerical order.  The lowest indexed node is
//      dropped, since that serves as the key into the hash.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Allowed cell value and matched flag.
//
// ****************************************************************************

class BTri
{
    friend class   BQuad;

  public:
                   BTri() { ordering_case = 255; matched = false; };

    int            AssignNodes(const int *);
    inline bool    Equals(BTri *&t)
                   {
                      if (t->nodes[0] == nodes[0] && t->nodes[1] == nodes[1])
                      {
                          return true;
                      }
                      return false;
                   }

    bool           Equals(BQuad *);
    void           AddInRemainingTriangle(BQuad *, int);
    inline void    ReRegisterMemory(void)
                       {
                           MemoryManager->ReRegisterTri(this);
                       }

    inline void    SetOriginalZone(const int &oz) { orig_zone = oz; };
    inline int     GetOriginalZone(void) { return orig_zone; };
    inline void    SetCellValue(const int &cv) { cell_value = cv; };
    inline int     GetCellValue(void) { return cell_value; };
    inline void    SetMatched(bool m) { matched = m; };
    inline bool    GetMatched() { return matched; };

    void           OutputCell(int,vtkPolyData *, vtkCellData *, vtkCellData *);

    static void    RegisterMemoryManager(BTriMemoryManager *);
    static void    SetNumberOfPoints(int np) { npts = np; };


  protected:
    unsigned char ordering_case;
    int           nodes[2];
    int           orig_zone;
    int           cell_value;
    bool          matched;

    static BTriMemoryManager *MemoryManager;
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
//  Class: BLine
//
//  Purpose:
//      A representation of a line.  For efficient caching, the
//      indices are sorted in numerical order.  The lowest indexed node is
//      dropped, since that serves as the key into the hash.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
//  Modifications:
//
// ****************************************************************************

class BLine
{
  public:
                   BLine() { ordering_case = 255; matched = false; };

    int            AssignNodes(const int *);
    inline bool    Equals(BLine *l)
                   {
                       return l->nodes[0] == nodes[0];
                   }

    inline void    ReRegisterMemory(void)
                       {
                           MemoryManager->ReRegisterLine(this);
                       }

    inline void    SetOriginalZone(const int &oz) { orig_zone = oz; };
    inline int     GetOriginalZone(void) { return orig_zone; };
    inline void    SetCellValue(const int &cv) { cell_value = cv; };
    inline int     GetCellValue(void) { return cell_value; };
    inline void    SetMatched(bool m) { matched = m; };
    inline bool    GetMatched() { return matched; };

    void           OutputCell(int,vtkPolyData *, vtkCellData *, vtkCellData *);

    static void    RegisterMemoryManager(BLineMemoryManager *);
    static void    SetNumberOfPoints(int np) { npts = np; };


  protected:
    unsigned char ordering_case;
    int           nodes[1];
    int           orig_zone;
    int           cell_value;
    bool          matched;

    static BLineMemoryManager *MemoryManager;
    static int                 npts;
};

//
// We will be re-ordering the nodes into numerical order.  This enumerated
// type will allow the ordering to be preserved.
//
typedef enum
{
    T01, T10
}  LINE_ORDERING_CASES;

static int line_reorder_list[2][2] = 
    { 
        { -1, 0 }, { 0, -1 }
    };


// ****************************************************************************
//  Class: BHashEntry
//
//  Purpose:
//      This is one entry in a larger hash list.  It knows how to add quads
//      and tris to its lists and free them whenever we have a match with the
//      same cell value, and mark them when we have a match with a different
//      cell value.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

typedef union
{
    BQuad *quad;
    BTri  *tri;
}  Face;

static int face_mask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

class BHashEntry
{
  public:
                   BHashEntry();
    virtual       ~BHashEntry() {;};

    void           AddQuad(BQuad *);
    void           AddTri(BTri *);

    inline void    SetPointIndex(int pi) { point_index = pi; };
    static void    RegisterMemoryManager(BHashEntryMemoryManager *mm)
                          { MemoryManager = mm; };
    static void    RegisterBHashEntryList(BHashEntryList *hel)
                          { list = hel; };

    void           CreateOutputCells(vtkPolyData*, vtkCellData*, vtkCellData*,
                                     bool);

  protected:
    Face           faces[FACES_PER_HASH_ENTRY];
    int            point_index;
    unsigned char  last_good_entry;
    unsigned char  face_type;
    BHashEntry     *extension;
   
    static BHashEntryMemoryManager *MemoryManager;
    static BHashEntryList          *list;

    bool           LocateAndRemoveTri(BTri *);
    bool           LocateAndRemoveQuad(BQuad *);
    void           ActuallyAddTri(BTri *);
    void           ActuallyAddQuad(BQuad *);
    void           RemoveEntry(int);
};

// ****************************************************************************
//  Class: BHashEntry2D
//
//  Purpose:
//      This is one entry in a larger hash list.  It knows how to add lines
//      to its lists and free them whenever we have a match with the same
//      cell value, and mark them when we have a match with a different
//      cell value.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
// ****************************************************************************

typedef BLine *pBLine;

class BHashEntry2D
{
  public:
                   BHashEntry2D();
    virtual       ~BHashEntry2D() {;};

    void           AddLine(BLine *);

    inline void    SetPointIndex(int pi) { point_index = pi; };
    static void    RegisterMemoryManager(BHashEntryMemoryManager2D *mm)
                          { MemoryManager = mm; };
    static void    RegisterBHashEntryList(BHashEntryList2D *hel)
                          { list = hel; };

    void           CreateOutputCells(vtkPolyData*, vtkCellData*, vtkCellData*,
                                     bool);

  protected:
    pBLine         lines[LINES_PER_HASH_ENTRY];
    int            point_index;
    unsigned char  last_good_entry;
    BHashEntry2D   *extension;
   
    static BHashEntryMemoryManager2D *MemoryManager;
    static BHashEntryList2D          *list;

    bool           LocateAndRemoveLine(BLine *);
    void           ActuallyAddLine(BLine *);
    void           RemoveEntry(int);
};



// ****************************************************************************
//  Class: BHashEntryMemoryManager
//
//  Purpose:
//      This provides a pool of BHashEntry objects.  They are allocated in bulk,
//      and then freed in bulk.
//
//  Programmer: Hank Childs
//  Creation:   October 22, 2002
//
// ****************************************************************************

class BHashEntryMemoryManager
{
  public:
                         BHashEntryMemoryManager();
    virtual             ~BHashEntryMemoryManager();

    inline BHashEntry    *GetBHashEntry()
                         {
                             if (currentHash+1 >= POOL_SIZE)
                             {
                                 AllocateBHashEntryPool();
                             }
                             BHashEntry *rv = currentHashPool + currentHash;
                             currentHash++;
                             return rv;
                         }

  protected:
    BHashEntry      *currentHashPool;
    int             currentHash;

    std::vector<BHashEntry *> hashpool;

    void            AllocateBHashEntryPool(void);
};


// ****************************************************************************
//  Class: BHashEntryMemoryManager2D
//
//  Purpose:
//      This provides a pool of BHashEntry2D objects.  They are allocated in,
//      bulk and then freed in bulk.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
// ****************************************************************************

class BHashEntryMemoryManager2D
{
  public:
                         BHashEntryMemoryManager2D();
    virtual             ~BHashEntryMemoryManager2D();

    inline BHashEntry2D  *GetBHashEntry()
                         {
                             if (currentHash+1 >= POOL_SIZE)
                             {
                                 AllocateBHashEntryPool();
                             }
                             BHashEntry2D *rv = currentHashPool + currentHash;
                             currentHash++;
                             return rv;
                         }

  protected:
    BHashEntry2D      *currentHashPool;
    int                currentHash;

    std::vector<BHashEntry2D *> hashpool;

    void            AllocateBHashEntryPool(void);
};


// ****************************************************************************
//  Class: BHashEntryList
//
//  Purpose:
//      This effectively works as the hash.  It hashes each faces by its lowest
//      numbered index. 
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Jun 12 09:14:12 PDT 2003
//    Added ability to return matched faces only.
//
// ****************************************************************************

class BHashEntryList
{
  public:
                BHashEntryList(int npts);
    virtual    ~BHashEntryList();

    void        AddTri(const int *, int orig_zone, int cell_value);
    void        AddQuad(const int *, int orig_zone, int cell_value);

    inline void IncrementMatchedCount() { matchedCount++; }

    inline void RemoveFace(void) { nfaces--; };
    int         GetNumberOfMatchedFaces(void) { return matchedCount; };

    void        CreateOutputCells(vtkPolyData *, vtkCellData *, vtkCellData *,
                                  bool);

  protected:
    BHashEntry             **list;
    int                     nhashes;
    int                     npts;
    int                     nfaces;
    int                     matchedCount;
    BQuadMemoryManager      qmm;
    BTriMemoryManager       tmm;
    BHashEntryMemoryManager hemm;
};


// ****************************************************************************
//  Class: BHashEntryList2D
//
//  Purpose:
//      This effectively works as the hash.  It hashes each line by its lowest
//      numbered index. 
//
//  Programmer: Jeremy Meredith
//  Creation:   June 12, 2003
//
//  Modifications:
//
// ****************************************************************************

class BHashEntryList2D
{
  public:
                BHashEntryList2D(int npts);
    virtual    ~BHashEntryList2D();

    inline void IncrementMatchedCount() { matchedCount++; }

    void        AddLine(const int *, int orig_zone, int cell_value);

    inline void RemoveLine(void) { nlines--; };
    int         GetNumberOfMatchedLines(void) { return matchedCount; };

    void        CreateOutputCells(vtkPolyData *, vtkCellData *, vtkCellData *,
                                  bool);

  protected:
    BHashEntry2D             **list;
    int                       nhashes;
    int                       npts;
    int                       nlines;
    int                       matchedCount;
    BLineMemoryManager        lmm;
    BHashEntryMemoryManager2D hemm;
};



// Declare these static members.
//
BHashEntryList            *BQuad::list = NULL;
BQuadMemoryManager        *BQuad::MemoryManager = NULL;
int                        BQuad::npts = 0;
int                        BTri::npts = 0;
BTriMemoryManager         *BTri::MemoryManager  = NULL;
int                        BLine::npts = 0;
BLineMemoryManager        *BLine::MemoryManager  = NULL;
BHashEntryList            *BHashEntry::list = NULL;
BHashEntryMemoryManager   *BHashEntry::MemoryManager = NULL;
BHashEntryList2D          *BHashEntry2D::list = NULL;
BHashEntryMemoryManager2D *BHashEntry2D::MemoryManager = NULL;


//
// Function prototypes
//

static void AddPolygon(int, vtkIdType *, int, int, BHashEntryList2D &);
static void AddPixel(vtkIdType *, int, int, BHashEntryList2D &);
static void AddTetrahedron(vtkIdType *, int, int, BHashEntryList &);
static void AddWedge(vtkIdType *, int, int, BHashEntryList &);
static void AddPyramid(vtkIdType *, int, int, BHashEntryList &);
static void AddHexahedron(vtkIdType *, int, int, BHashEntryList &);
static void AddVoxel(vtkIdType *, int, int, BHashEntryList &);
static int  LoopOverAllCells(vtkUnstructuredGrid *, BHashEntryList &, BHashEntryList2D &, bool &);
static void LoopOverUnhashedCells(vtkUnstructuredGrid *, vtkPolyData *,
                                  vtkCellData *, vtkCellData *);


// ****************************************************************************
//  Method: BQuad::RegisterBHashEntryList
//
//  Purpose:
//      Registers the hash entry list with the BQuad class.  This is necessary
//      because a triangle may collide with a quadrilateral and the result
//      (another triangle) needs to be added back into the hash as its own
//      entry.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
BQuad::RegisterBHashEntryList(BHashEntryList *l)
{
    list = l;
}


// ****************************************************************************
//  Method: BQuad::RegisterMemoryManager
//
//  Purpose:
//      Registers the memory manager with the BQuad class.  Whenever a quad is
//      freed from the hash, the newly freed quad is put back on a queue with
//      the memory manager via this reference.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
BQuad::RegisterMemoryManager(BQuadMemoryManager *mm)
{
    MemoryManager = mm;
}


// ****************************************************************************
//  Method: BQuad::AssignNodes
//
//  Purpose:
//      Tells the BQuad what its nodes will be.  Also sorts the nodes in
//      numerical order and stores off the original ordering.  The return value
//      is the lowest-indexed node, which is not stored with the quad, but is
//      instead used as the hash key.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

int
BQuad::AssignNodes(const int *n)
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
//  Method: BQuad::OutputCell
//
//  Purpose:
//      Outputs a vtkQuad into the poly data object.
//
//  Programmer: Hank Childs
//  Creation:   October 23, 2002
//
// ****************************************************************************

void
BQuad::OutputCell(int node0, vtkPolyData *pd, vtkCellData *in_cd,
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
//  Method: BQuad::Equals
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
BQuad::Equals(BQuad *q)
{
    if (q->nodes[0] == nodes[0] && q->nodes[1] == nodes[1] &&
        q->nodes[2] == nodes[2])
    {
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: BQuad::Equals
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
BQuad::Equals(BTri *t)
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
//  Method: BQuad::AddInRemainingTriangle
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
BQuad::AddInRemainingTriangle(BTri *t, int node_0)
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
//  Method: BQuad::AddInRemainingTriangle
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
//    Jeremy Meredith, Fri May 30 14:57:46 PDT 2003
//    Added cell_value.
//
// ****************************************************************************

void
BQuad::AddInRemainingTriangle(int n, int node_0)
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
    list->AddTri(tmp_nodes, orig_zone, cell_value);
}


// ****************************************************************************
//  Method: BTri::RegisterMemoryManager
//
//  Purpose:
//      Registers the memory manager with the BTri class.  Whenever a tri is
//      freed from the hash, the newly freed tri is put back on a queue with
//      the memory manager via this reference.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
BTri::RegisterMemoryManager(BTriMemoryManager *mm)
{
    MemoryManager = mm;
}


// ****************************************************************************
//  Method: BTri::AssignNodes
//
//  Purpose:
//      Tells the BTri what its nodes will be.  Also sorts the nodes in
//      numerical order and stores off the original ordering.  The return value
//      is the lowest-indexed node, which is not stored with the tri, but is
//      instead used as the hash key.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

int
BTri::AssignNodes(const int *n)
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

/* *** See comment in BQuad::AssignNodes to explain this previous hashing
   *** scheme.
    int nmod4 = nodes[0] % 4;
    int val = nmod4*npts + smallest;
    return val;
 */
    return smallest;
}


// ****************************************************************************
//  Method: BTri::OutputCell
//
//  Purpose:
//      Outputs a vtkTri into the poly data object.
//
//  Programmer: Hank Childs
//  Creation:   October 23, 2002
//
// ****************************************************************************

void
BTri::OutputCell(int node0, vtkPolyData *pd, vtkCellData *in_cd,
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
//  Method: BTri::Equals
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
BTri::Equals(BQuad *q)
{
    //
    // The quad already knows how to do this, so let it do the dirty work.
    //
    return q->Equals(this);
}


// ****************************************************************************
//  Method: BTri::AddInRemainingTriangle
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
BTri::AddInRemainingTriangle(BQuad *q, int node_0)
{
    q->AddInRemainingTriangle(this, node_0);
}


// ****************************************************************************
//  Method: BLine::RegisterMemoryManager
//
//  Purpose:
//      Registers the memory manager with the BLine class.  Whenever a line is
//      freed from the hash, the newly freed line is put back on a queue with
//      the memory manager via this reference.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

void
BLine::RegisterMemoryManager(BLineMemoryManager *mm)
{
    MemoryManager = mm;
}


// ****************************************************************************
//  Method: BLine::AssignNodes
//
//  Purpose:
//      Tells the BLine what its nodes will be.  Also sorts the nodes in
//      numerical order and stores off the original ordering.  The return value
//      is the lowest-indexed node, which is not stored with the line, but is
//      instead used as the hash key.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

int
BLine::AssignNodes(const int *n)
{
    int smallest = 0;
    if (n[0] < n[1])
    {
        ordering_case = T01;
        smallest = n[0];
        nodes[0] = n[1];
    }
    else
    {
        ordering_case = T10;
        smallest = n[1];
        nodes[0] = n[0];
    }

    return smallest;
}


// ****************************************************************************
//  Method: BLine::OutputCell
//
//  Purpose:
//      Outputs a vtkLine into the poly data object.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

void
BLine::OutputCell(int node0, vtkPolyData *pd, vtkCellData *in_cd,
                 vtkCellData *out_cd)
{
    vtkIdType n[2];
    int *list = line_reorder_list[ordering_case];
    n[0] = (list[0] == -1 ? node0 : nodes[list[0]]);
    n[1] = (list[1] == -1 ? node0 : nodes[list[1]]);
    int newId = pd->InsertNextCell(VTK_LINE, 2, n);
    out_cd->CopyData(in_cd, orig_zone, newId);
}


// ****************************************************************************
//  Method: BHashEntry constructor
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

BHashEntry::BHashEntry()
{
    point_index = -1;
    last_good_entry = 0;
    extension = NULL;
}


// ****************************************************************************
//  Method: BHashEntry::AddTri
//
//  Purpose:
//      Adds a triangle to the hash entry.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
BHashEntry::AddTri(BTri *f)
{
    bool foundFace = LocateAndRemoveTri(f);

    if (!foundFace)
    {
        ActuallyAddTri(f);
    }
}
        

// ****************************************************************************
//  Method: BHashEntry::AddQuad
//
//  Purpose:
//      Adds a quad to the hash entry.
//
//  Programmer: Hank Childs
//  Creation:   October 21, 2002
//
// ****************************************************************************

void
BHashEntry::AddQuad(BQuad *f)
{
    bool foundFace = LocateAndRemoveQuad(f);

    if (!foundFace)
    {
        ActuallyAddQuad(f);
    }
}
        

// ****************************************************************************
//  Method: BHashEntry::RemoveEntry
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
BHashEntry::RemoveEntry(int ind)
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
//  Method: BHashEntry::LocateAndRemoveQuad
// 
//  Purpose:
//      Locates a quad in the hash entry.  If the quad already exists
//      with the same value, removes it and tells the caller not to add
//      the new one.  If the quad already exists with a different value,
//      it marks the existing one as valid but still doesn't add the new
//      one.  It keeps the quad with the smaller cell data value.
//
//  Programmer: Jeremy Meredith
//  Creation:   May 30, 2003
//
// ****************************************************************************

bool
BHashEntry::LocateAndRemoveQuad(BQuad *f)
{
    for (int i = 0 ; i < last_good_entry ; i++)
    {
        if (face_type & face_mask[i])
        {
            BQuad *q = faces[i].quad;
            if (q->Equals(f))
            {
                if (q->GetCellValue() < f->GetCellValue())
                {
                    list->IncrementMatchedCount();
                    q->SetMatched(true);
                    list->RemoveFace();
                    f->ReRegisterMemory();
                }
                else if (q->GetCellValue() > f->GetCellValue())
                {
                    list->IncrementMatchedCount();
                    f->SetMatched(true);
                    list->RemoveFace();
                    RemoveEntry(i);
                    q->ReRegisterMemory();
                    return false;
                }
                else
                {
                    list->RemoveFace();
                    list->RemoveFace();
                    RemoveEntry(i);
                    q->ReRegisterMemory();
                    f->ReRegisterMemory();
                }
                return true;
            }
        }
        else
        {
            BTri *t = faces[i].tri;
            if (t->Equals(f))
            {
                if (t->GetCellValue() < f->GetCellValue())
                {
                    list->IncrementMatchedCount();
                    t->SetMatched(true);
                    list->RemoveFace();
                    f->ReRegisterMemory();
                }
                else if (t->GetCellValue() > f->GetCellValue())
                {
                    list->IncrementMatchedCount();
                    f->SetMatched(true);
                    list->RemoveFace();
                    RemoveEntry(i);
                    f->AddInRemainingTriangle(t, point_index);
                    t->ReRegisterMemory();
                    return false;
                }
                else
                {
                    list->RemoveFace();
                    list->RemoveFace();
                    RemoveEntry(i);
                    f->AddInRemainingTriangle(t, point_index);
                    t->ReRegisterMemory();
                    f->ReRegisterMemory();
                }
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
//  Method: BHashEntry::LocateAndRemoveQuad
// 
//  Purpose:
//      Locates a triangle in the hash entry and removes it if it exists.
//
//  Programmer: Jeremy Meredith
//  Creation:   May 30, 2003
//
// ****************************************************************************

bool
BHashEntry::LocateAndRemoveTri(BTri *f)
{
    for (int i = 0 ; i < last_good_entry ; i++)
    {
        if (face_type & face_mask[i])
        {
            BQuad *q = faces[i].quad;
            if (q->Equals(f))
            {
                if (q->GetCellValue() < f->GetCellValue())
                {
                    list->IncrementMatchedCount();
                    q->SetMatched(true);
                    list->RemoveFace();
                    f->ReRegisterMemory();
                }
                else if (q->GetCellValue() > f->GetCellValue())
                {
                    list->IncrementMatchedCount();
                    f->SetMatched(true);
                    list->RemoveFace();
                    RemoveEntry(i);
                    q->AddInRemainingTriangle(f, point_index);
                    q->ReRegisterMemory();
                    return false;
                }
                else
                {
                    list->RemoveFace();
                    list->RemoveFace();
                    RemoveEntry(i);
                    q->AddInRemainingTriangle(f, point_index);
                    q->ReRegisterMemory();
                    f->ReRegisterMemory();
                }
                return true;
            }
        }
        else
        {
            BTri *t = faces[i].tri;
            if (t->Equals(f))
            {
                if (t->GetCellValue() < f->GetCellValue())
                {
                    list->IncrementMatchedCount();
                    t->SetMatched(true);
                    list->RemoveFace();
                    f->ReRegisterMemory();
                }
                else if (t->GetCellValue() > f->GetCellValue())
                {
                    list->IncrementMatchedCount();
                    f->SetMatched(true);
                    list->RemoveFace();
                    RemoveEntry(i);
                    t->ReRegisterMemory();
                    return false;
                }
                else
                {
                    list->RemoveFace();
                    list->RemoveFace();
                    RemoveEntry(i);
                    t->ReRegisterMemory();
                    f->ReRegisterMemory();
                }
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
//  Method: BHashEntry::ActuallyAddQuad
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
BHashEntry::ActuallyAddQuad(BQuad *f)
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
        extension = MemoryManager->GetBHashEntry();
        extension->SetPointIndex(point_index);
    }
    extension->ActuallyAddQuad(f);
}


// ****************************************************************************
//  Method: BHashEntry::ActuallyAddTri
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
BHashEntry::ActuallyAddTri(BTri *f)
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
        extension = MemoryManager->GetBHashEntry();
        extension->SetPointIndex(point_index);
    }
    extension->ActuallyAddTri(f);
}


// ****************************************************************************
//  Method: BHashEntryMemoryManager constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

BHashEntryMemoryManager::BHashEntryMemoryManager()
{
    AllocateBHashEntryPool();
}


// ****************************************************************************
//  Method: BHashEntryMemoryManager destructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

BHashEntryMemoryManager::~BHashEntryMemoryManager()
{
    int size = hashpool.size();
    for (int i = 0 ; i < size ; i++)
    {
        delete [] hashpool[i];
    }
}


// ****************************************************************************
//  Method: BHashEntryMemoryManager::AllocateBHashEntryPool
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
BHashEntryMemoryManager::AllocateBHashEntryPool(void)
{
    currentHashPool = new BHashEntry[POOL_SIZE];
    currentHash = 0;
    hashpool.push_back(currentHashPool);
}


// ****************************************************************************
//  Method: BHashEntry2D constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

BHashEntry2D::BHashEntry2D()
{
    point_index = -1;
    last_good_entry = 0;
    extension = NULL;
}


// ****************************************************************************
//  Method: BHashEntry2D::AddLine
//
//  Purpose:
//      Adds a line to the hash entry.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

void
BHashEntry2D::AddLine(BLine *l)
{
    bool foundLine = LocateAndRemoveLine(l);

    if (!foundLine)
    {
        ActuallyAddLine(l);
    }
}
        

// ****************************************************************************
//  Method: BHashEntry2D::RemoveEntry
//
//  Purpose:
//      This removes an entry from the hash list.  It swaps the last entry
//      with the argument ('ind'), and updates the bit logic saying whether
//      each face is a triangle or a quadrilateral.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

void
BHashEntry2D::RemoveEntry(int ind)
{
    last_good_entry--;
    lines[ind] = lines[last_good_entry];
}


// ****************************************************************************
//  Method: BHashEntry2D::LocateAndRemoveLine
// 
//  Purpose:
//      Locates a line in the hash entry.  If the line already exists
//      with the same value, removes it and tells the caller not to add
//      the new one.  If the line already exists with a different value,
//      it marks the existing one as valid but still doesn't add the new
//      one (or else it removes the old one and tells the caller to still
//      add the new one; this depends on which value is lower).  It keeps
//      the line with the smaller cell data value.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

bool
BHashEntry2D::LocateAndRemoveLine(BLine *l2)
{
    for (int i = 0 ; i < last_good_entry ; i++)
    {
        BLine *l1 = lines[i];
        if (l1->Equals(l2))
        {
            if (l1->GetCellValue() < l2->GetCellValue())
            {
                list->IncrementMatchedCount();
                l1->SetMatched(true);
                list->RemoveLine();
                l2->ReRegisterMemory();
            }
            else if (l1->GetCellValue() > l2->GetCellValue())
            {
                list->IncrementMatchedCount();
                l2->SetMatched(true);
                list->RemoveLine();
                RemoveEntry(i);
                l1->ReRegisterMemory();
                return false;
            }
            else
            {
                list->RemoveLine();
                list->RemoveLine();
                RemoveEntry(i);
                l1->ReRegisterMemory();
                l2->ReRegisterMemory();
            }
            return true;
        }
    }
    if (extension != NULL)
    {
        return extension->LocateAndRemoveLine(l2);
    }

    return false;
}


// ****************************************************************************
//  Method: BHashEntry2D::ActuallyAddLine
//
//  Purpose:
//      After determining that this line is unique, this actually adds it to
//      the hash entry.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

void
BHashEntry2D::ActuallyAddLine(BLine *l)
{
    if (last_good_entry < LINES_PER_HASH_ENTRY)
    {
         lines[last_good_entry] = l;
         last_good_entry++;
         return;
    }

    if (extension == NULL)
    {
        extension = MemoryManager->GetBHashEntry();
        extension->SetPointIndex(point_index);
    }
    extension->ActuallyAddLine(l);
}


// ****************************************************************************
//  Method: BHashEntryMemoryManager2D constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

BHashEntryMemoryManager2D::BHashEntryMemoryManager2D()
{
    AllocateBHashEntryPool();
}


// ****************************************************************************
//  Method: BHashEntryMemoryManager2D destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

BHashEntryMemoryManager2D::~BHashEntryMemoryManager2D()
{
    int size = hashpool.size();
    for (int i = 0 ; i < size ; i++)
    {
        delete [] hashpool[i];
    }
}


// ****************************************************************************
//  Method: BHashEntryMemoryManager2D::AllocateBHashEntryPool2D
//
//  Purpose:
//      Creates a pool of hash entries in a large chunk.  This is to reduce
//      the memory costs.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

void
BHashEntryMemoryManager2D::AllocateBHashEntryPool(void)
{
    currentHashPool = new BHashEntry2D[POOL_SIZE];
    currentHash = 0;
    hashpool.push_back(currentHashPool);
}


// ****************************************************************************
//  Method: BQuadMemoryManager constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

BQuadMemoryManager::BQuadMemoryManager()
{
    freequadindex = 0;
}


// ****************************************************************************
//  Method: BQuadMemoryManager destructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

BQuadMemoryManager::~BQuadMemoryManager()
{
    int npools = quadpool.size();
    for (int i = 0 ; i < npools ; i++)
    {
        BQuad *pool = quadpool[i];
        delete [] pool;
    }
}


// ****************************************************************************
//  Method: BQuadMemoryManager::AllocateQuadPool
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
BQuadMemoryManager::AllocateQuadPool(void)
{
    if (freequadindex == 0)
    {
        BQuad *newlist = new BQuad[POOL_SIZE];
        quadpool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
            freequadlist[i] = &(newlist[i]);
        }
        freequadindex = POOL_SIZE;
    }
}


// ****************************************************************************
//  Method: BTriMemoryManager constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

BTriMemoryManager::BTriMemoryManager()
{
    freetriindex = 0;
}


// ****************************************************************************
//  Method: BTriMemoryManager destructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

BTriMemoryManager::~BTriMemoryManager()
{
    int npools = tripool.size();
    for (int i = 0 ; i < npools ; i++)
    {
        BTri *pool = tripool[i];
        delete [] pool;
    }
}


// ****************************************************************************
//  Method: BTriMemoryManager::AllocateTriPool
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
BTriMemoryManager::AllocateTriPool(void)
{
    if (freetriindex <= 0)
    {
        BTri *newlist = new BTri[POOL_SIZE];
        tripool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
            freetrilist[i] = &(newlist[i]);
        }
        freetriindex = POOL_SIZE;
    }
}


// ****************************************************************************
//  Method: BLineMemoryManager constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

BLineMemoryManager::BLineMemoryManager()
{
    freelineindex = 0;
}


// ****************************************************************************
//  Method: BLineMemoryManager destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

BLineMemoryManager::~BLineMemoryManager()
{
    int npools = linepool.size();
    for (int i = 0 ; i < npools ; i++)
    {
        BLine *pool = linepool[i];
        delete [] pool;
    }
}


// ****************************************************************************
//  Method: BLineMemoryManager::AllocateLinePool
//
//  Purpose:
//      Creates a pool of lines in a large chunk.  This is to reduce
//      the memory costs.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

void
BLineMemoryManager::AllocateLinePool(void)
{
    if (freelineindex <= 0)
    {
        BLine *newlist = new BLine[POOL_SIZE];
        linepool.push_back(newlist);
        for (int i = 0 ; i < POOL_SIZE ; i++)
        {
            freelinelist[i] = &(newlist[i]);
        }
        freelineindex = POOL_SIZE;
    }
}


// ****************************************************************************
//  Method: BHashEntryList constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

BHashEntryList::BHashEntryList(int np)
{
    npts = np;
    nhashes = npts;
    nfaces = 0;
    matchedCount = 0;
    list = new BHashEntry*[nhashes];
    for (int i = 0 ; i < nhashes ; i++)
    {
        list[i] = NULL;
    }
    BHashEntry::RegisterMemoryManager(&hemm);
    BHashEntry::RegisterBHashEntryList(this);
    BQuad::RegisterMemoryManager(&qmm);
    BTri::RegisterMemoryManager(&tmm);
    BQuad::RegisterBHashEntryList(this);
}


// ****************************************************************************
//  Method: BHashEntryList constructor
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
// ****************************************************************************

BHashEntryList::~BHashEntryList()
{
    if (list != NULL)
    {
        delete [] list;
        list = NULL;
    }
}


// ****************************************************************************
//  Method: BHashEntryList::AddTri
//
//  Purpose:
//      Constructs a 'BTri' object out of the arguments and attempts to add it
//      to whatever BHashEntry object the 'BTri' hashes to.  The BHashEntry will
//      either add it, or identify it as a duplicate and remove it and the
//      duplicate.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added cell_value.
//
// ****************************************************************************

void
BHashEntryList::AddTri(const int *node_list, int orig_zone, int cell_value)
{
    nfaces++;
    BTri *tri = tmm.GetFreeTri();
    int hash_index = tri->AssignNodes(node_list);
    tri->SetOriginalZone(orig_zone);
    tri->SetCellValue(cell_value);
    if (list[hash_index] == NULL)
    {
        list[hash_index] = hemm.GetBHashEntry();
        list[hash_index]->SetPointIndex(hash_index);
    }
    list[hash_index]->AddTri(tri);
}


// ****************************************************************************
//  Method: BHashEntryList::AddQuad
//
//  Purpose:
//      Constructs a 'BQuad' object out of the arguments and attempts to add it
//      to whatever BHashEntry object the 'BQuad' hashes to.  The BHashEntry will
//      either add it, or identify it as a duplicate and remove it and the
//      duplicate.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added cell_value.
//
// ****************************************************************************

void
BHashEntryList::AddQuad(const int *node_list, int orig_zone, int cell_value)
{
    nfaces++;
    BQuad *quad = qmm.GetFreeQuad();
    int hash_index = quad->AssignNodes(node_list);
    quad->SetOriginalZone(orig_zone);
    quad->SetCellValue(cell_value);
    if (list[hash_index] == NULL)
    {
        list[hash_index] = hemm.GetBHashEntry();
        list[hash_index]->SetPointIndex(hash_index%npts);
    }
    list[hash_index]->AddQuad(quad);
}


// ****************************************************************************
//  Method: BHashEntryList::CreateOutputCells
//
//  Purpose:
//      Goes through each of the hash entries and has it output its faces as
//      VTK objects.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added check for single-valued data to also output unmatched polygons.
//
// ****************************************************************************

void
BHashEntryList::CreateOutputCells(vtkPolyData *output, vtkCellData *in_cd,
                                 vtkCellData *out_cd, bool isSingleValue)
{
    for (int i = 0 ; i < nhashes ; i++)
    {
        if (list[i] != NULL)
        {
            list[i]->CreateOutputCells(output, in_cd, out_cd, isSingleValue);
        }
    }
}


// ****************************************************************************
//  Method: BHashEntry::CreateOutputCells
//
//  Purpose:
//      Goes through each of the faces and has them output themselves as VTK 
//      objects.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Only output matched polygons.  Added check for single-valued data to
//    also output unmatched polygons, but I disabled it because it was
//    causing problems on multi-domain files.
//
// ****************************************************************************

void
BHashEntry::CreateOutputCells(vtkPolyData *output, vtkCellData *in_cd,
                             vtkCellData *out_cd, bool isSingleValue)
{
    for (int i = 0 ; i < last_good_entry ; i++)
    {
        bool isQuad = face_type & face_mask[i];
        if (isQuad)
        {
            BQuad *q = faces[i].quad;
            if (q->GetMatched())
                q->OutputCell(point_index, output, in_cd, out_cd);
        }
        else
        {
            BTri *t = faces[i].tri;
            if (t->GetMatched())
                t->OutputCell(point_index, output, in_cd, out_cd);
        }
    }
    if (extension != NULL)
    {
        extension->CreateOutputCells(output, in_cd, out_cd, isSingleValue);
    }
}


// ****************************************************************************
//  Method: BHashEntryList2D constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   Wed Jun 11 13:06:56 PDT 2003
//
// ****************************************************************************

BHashEntryList2D::BHashEntryList2D(int np)
{
    npts = np;
    nhashes = npts;
    nlines = 0;
    matchedCount = 0;
    list = new BHashEntry2D*[nhashes];
    for (int i = 0 ; i < nhashes ; i++)
    {
        list[i] = NULL;
    }
    BHashEntry2D::RegisterMemoryManager(&hemm);
    BHashEntry2D::RegisterBHashEntryList(this);
    BLine::RegisterMemoryManager(&lmm);
}


// ****************************************************************************
//  Method: BHashEntryList2D constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
// ****************************************************************************

BHashEntryList2D::~BHashEntryList2D()
{
    if (list != NULL)
    {
        delete [] list;
        list = NULL;
    }
}


// ****************************************************************************
//  Method: BHashEntryList2D::AddLine
//
//  Purpose:
//      Constructs a 'BLine' object out of the arguments and attempts to add it
//      to the BHashEntry2D object the BLine hashes to.  The BHashEntry2D will
//      either add it, or identify it as a duplicate and remove it and the
//      duplicate.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
//  Modifications:
//
// ****************************************************************************

void
BHashEntryList2D::AddLine(const int *node_list, int orig_zone, int cell_value)
{
    nlines++;
    BLine *line = lmm.GetFreeLine();
    int hash_index = line->AssignNodes(node_list);
    line->SetOriginalZone(orig_zone);
    line->SetCellValue(cell_value);
    if (list[hash_index] == NULL)
    {
        list[hash_index] = hemm.GetBHashEntry();
        list[hash_index]->SetPointIndex(hash_index);
    }
    list[hash_index]->AddLine(line);
}


// ****************************************************************************
//  Method: BHashEntryList2D::CreateOutputCells
//
//  Purpose:
//      Goes through each of the hash entries and has it output its lines as
//      VTK objects.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
//  Modifications:
//
// ****************************************************************************

void
BHashEntryList2D::CreateOutputCells(vtkPolyData *output, vtkCellData *in_cd,
                                    vtkCellData *out_cd, bool isSingleValue)
{
    for (int i = 0 ; i < nhashes ; i++)
    {
        if (list[i] != NULL)
        {
            list[i]->CreateOutputCells(output, in_cd, out_cd, isSingleValue);
        }
    }
}


// ****************************************************************************
//  Method: BHashEntry2D::CreateOutputCells
//
//  Purpose:
//      Goes through each of the lines and has them output themselves as VTK 
//      objects.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
//  Modifications:
//
// ****************************************************************************

void
BHashEntry2D::CreateOutputCells(vtkPolyData *output, vtkCellData *in_cd,
                                vtkCellData *out_cd, bool isSingleValue)
{
    for (int i = 0 ; i < last_good_entry ; i++)
    {
        BLine *l = lines[i];
        if (l->GetMatched())
            l->OutputCell(point_index, output, in_cd, out_cd);
    }
    if (extension != NULL)
    {
        extension->CreateOutputCells(output, in_cd, out_cd, isSingleValue);
    }
}


vtkStandardNewMacro(vtkUnstructuredGridBoundaryFilter); 

void
vtkUnstructuredGridBoundaryFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}

// ****************************************************************************
//  Method: vtkUnstructuredGridBoundaryFilter::Execute
//
//  Purpose:
//      Finds the faces that are external to the unstructured grid input.
//
//  Programmer: Hank Childs
//  Creation:   November 4, 2002
//
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Made changes to allow inter-subset boundaries.
//
//    Jeremy Meredith, Thu Jun 12 09:18:34 PDT 2003
//    Added logic to match polygons (and return lines) as well as matching
//    polyhedra (and returning faces).
//
// ****************************************************************************

void
vtkUnstructuredGridBoundaryFilter::Execute()
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

    int ntotalpts = input->GetNumberOfPoints();
    BHashEntryList   list(ntotalpts);
    BHashEntryList2D list2d(ntotalpts);

    //
    // This does the work of looping over all the cells and storing them in
    // our hash table.
    //
    bool isSingleValue;
    int numCopyCells = LoopOverAllCells(input, list, list2d, isSingleValue);

    //
    // Count up how many output cells we will have.
    //
    int numOutFaces = list.GetNumberOfMatchedFaces();
    int numOutLines = list2d.GetNumberOfMatchedLines();
    int numOutCells = numCopyCells + numOutFaces + numOutLines;
    int numOutConn  = (4+1)*numCopyCells +
                      (4+1)*numOutFaces  +
                      (2+1)*numOutLines;

    //
    // Now create our output cells.
    //
    output->Allocate(numOutCells, numOutConn);
    outputCD->CopyAllocate(cd, numOutCells);
    list.CreateOutputCells(output, cd, outputCD, isSingleValue);
    list2d.CreateOutputCells(output, cd, outputCD, isSingleValue);
    if (numCopyCells > 0)
    {
        LoopOverUnhashedCells(input, output, cd, outputCD);
    }
}


// ****************************************************************************
//  Function: LoopOverUnhashedCells
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
//    Jeremy Meredith, Thu Jun 24 14:15:52 PDT 2004
//    Added pixel support.
//
// ****************************************************************************

void LoopOverUnhashedCells(vtkUnstructuredGrid *input, vtkPolyData *output,
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
            newCellId = output->InsertNextCell(cellType, npts, pixel_ids);
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
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added cell value from the avtSubsets array to use for face creation.
//    Added check for single-valued data to also output unmatched polygons.
//    Added ability to hash 2D cells for edge-matching.
//
// ****************************************************************************

int
LoopOverAllCells(vtkUnstructuredGrid *input, BHashEntryList &list,
                 BHashEntryList2D &list2d, bool &isSingleValue)
{
    vtkCellArray *Connectivity = input->GetCells();
    if (Connectivity == NULL)
    {
        return 0;
    }

    vtkDataArray *boundaryArray = input->GetCellData()->GetArray("avtSubsets");
    if (boundaryArray == NULL ||
        boundaryArray->GetDataType() != VTK_INT)
    {
        return 0;
    }

    int *cellData = (int*)(boundaryArray->GetVoidPointer(0));
    isSingleValue = true;

    int         numUnhashedCells = 0;
    vtkIdType   cellId;
    vtkIdType   npts;
    vtkIdType   *pts;
    for (cellId=0, Connectivity->InitTraversal();
         Connectivity->GetNextCell(npts,pts);
         cellId++)
    {
        int cellType = input->GetCellType(cellId);
        int cellVal  = cellData[cellId];

        if (cellVal != cellData[0])
            isSingleValue = false;

        switch (cellType)
        {
          case VTK_VERTEX:
          case VTK_POLY_VERTEX:
          case VTK_LINE:
          case VTK_POLY_LINE:
          case VTK_TRIANGLE_STRIP:
            numUnhashedCells++;
            break;

          case VTK_TRIANGLE:
          case VTK_QUAD:
          case VTK_POLYGON:
            AddPolygon(npts, pts, cellId, cellVal, list2d);
            break;

          case VTK_PIXEL:
            AddPixel(pts, cellId, cellVal, list2d);
            break;

          case VTK_TETRA:
            AddTetrahedron(pts, cellId, cellVal, list);
            break;

          case VTK_VOXEL:
            AddVoxel(pts, cellId, cellVal, list);
            break;

          case VTK_HEXAHEDRON:
            AddHexahedron(pts, cellId, cellVal, list);
            break;

          case VTK_WEDGE:
            AddWedge(pts, cellId, cellVal, list);
            break;

          case VTK_PYRAMID:
            AddPyramid(pts, cellId, cellVal, list);
            break;
        }
    }

    return numUnhashedCells;
}


// ****************************************************************************
//  Function: AddPolygon
//
//  Purpose:
//      Adds all of the lines from a polygon to our hash entry list.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 11, 2003
//
//  Modifications:
//
// ****************************************************************************

void
AddPolygon(int npts, vtkIdType *pts, int cellId, int cellVal,
           BHashEntryList2D &list)
{
    vtkIdType nodes[2];
    for (int l=0; l<npts; l++)
    {
        nodes[0] = pts[l];
        nodes[1] = pts[(l+1)%npts];
        list.AddLine(nodes, cellId, cellVal);
    }
}


// ****************************************************************************
//  Function: AddPixel
//
//  Purpose:
//      Adds all of the lines from a pixel to our hash entry list.
//
//  Programmer: Jeremy Meredith
//  Creation:   June 24, 2004
//
//  Modifications:
//
// ****************************************************************************

void
AddPixel(vtkIdType *pts, int cellId, int cellVal,
           BHashEntryList2D &list)
{
    vtkIdType nodes[2];

    nodes[0] = pts[0];
    nodes[1] = pts[1];
    list.AddLine(nodes, cellId, cellVal);
    nodes[0] = pts[1];
    nodes[1] = pts[3];
    list.AddLine(nodes, cellId, cellVal);
    nodes[0] = pts[2];
    nodes[1] = pts[3];
    list.AddLine(nodes, cellId, cellVal);
    nodes[0] = pts[0];
    nodes[1] = pts[2];
    list.AddLine(nodes, cellId, cellVal);
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
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added cellVal.
//
// ****************************************************************************

void
AddTetrahedron(vtkIdType *pts, int cellId, int cellVal, BHashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[2];
    nodes[1] = pts[1];
    nodes[2] = pts[0];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[3];
    nodes[1] = pts[2];
    nodes[2] = pts[0];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[3];
    nodes[1] = pts[1];
    nodes[2] = pts[2];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[3];
    nodes[1] = pts[0];
    nodes[2] = pts[1];
    list.AddTri(nodes, cellId, cellVal);
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
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added cellVal.
//
// ****************************************************************************

void
AddVoxel(vtkIdType *pts, int cellId, int cellVal, BHashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[0];
    nodes[1] = pts[4];
    nodes[2] = pts[6];
    nodes[3] = pts[2];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[1];
    nodes[1] = pts[3];
    nodes[2] = pts[7];
    nodes[3] = pts[5];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[0];
    nodes[1] = pts[1];
    nodes[2] = pts[5];
    nodes[3] = pts[4];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[2];
    nodes[1] = pts[6];
    nodes[2] = pts[7];
    nodes[3] = pts[3];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[1];
    nodes[1] = pts[0];
    nodes[2] = pts[2];
    nodes[3] = pts[3];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[4];
    nodes[1] = pts[5];
    nodes[2] = pts[7];
    nodes[3] = pts[6];
    list.AddQuad(nodes, cellId, cellVal);
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
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added cellVal.
//
// ****************************************************************************

void
AddHexahedron(vtkIdType *pts, int cellId, int cellVal, BHashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[0];
    nodes[1] = pts[4];
    nodes[2] = pts[7];
    nodes[3] = pts[3];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[1];
    nodes[1] = pts[2];
    nodes[2] = pts[6];
    nodes[3] = pts[5];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[0];
    nodes[1] = pts[1];
    nodes[2] = pts[5];
    nodes[3] = pts[4];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[3];
    nodes[1] = pts[7];
    nodes[2] = pts[6];
    nodes[3] = pts[2];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[0];
    nodes[1] = pts[3];
    nodes[2] = pts[2];
    nodes[3] = pts[1];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[4];
    nodes[1] = pts[5];
    nodes[2] = pts[6];
    nodes[3] = pts[7];
    list.AddQuad(nodes, cellId, cellVal);
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
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added cellVal.
//
// ****************************************************************************

void
AddWedge(vtkIdType *pts, int cellId, int cellVal, BHashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[0];
    nodes[1] = pts[1];
    nodes[2] = pts[2];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[3];
    nodes[1] = pts[5];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[0];
    nodes[1] = pts[3];
    nodes[2] = pts[4];
    nodes[3] = pts[1];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[1];
    nodes[1] = pts[4];
    nodes[2] = pts[5];
    nodes[3] = pts[2];
    list.AddQuad(nodes, cellId, cellVal);
    nodes[0] = pts[2];
    nodes[1] = pts[5];
    nodes[2] = pts[3];
    nodes[3] = pts[0];
    list.AddQuad(nodes, cellId, cellVal);
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
//  Modifications:
//    Jeremy Meredith, Fri May 30 14:58:55 PDT 2003
//    Added cellVal.
//
// ****************************************************************************

void
AddPyramid(vtkIdType *pts, int cellId, int cellVal, BHashEntryList &list)
{
    vtkIdType nodes[4];
    nodes[0] = pts[0];
    nodes[1] = pts[1];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[1];
    nodes[1] = pts[2];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[2];
    nodes[1] = pts[3];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[3];
    nodes[1] = pts[0];
    nodes[2] = pts[4];
    list.AddTri(nodes, cellId, cellVal);
    nodes[0] = pts[0];
    nodes[1] = pts[3];
    nodes[2] = pts[2];
    nodes[3] = pts[1];
    list.AddQuad(nodes, cellId, cellVal);
}


