#ifndef ZOOMIR_H
#define ZOOMIR_H

#include <MIR.h>
#include <MIRConnectivity.h>
#include <Array.h>

#define FREE_ENTRY_LIST_SIZE 16384
#define POOL_SIZE 256

struct ResampledMat;

// ****************************************************************************
//  Class:  ZooMIR
//
//  Purpose:
//    Split cells using material volume fractions using the new
//    more-ideal 3D ClipCases.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Sep 18 11:17:24 PDT 2003
//    Implemented 2D MIR.  Moved Reconstruct3DMesh's code into
//    ReconstructMesh and made both the 2D and 2D cases point to it.
//
//    Brad Whitlock, Tue Sep 23 09:44:40 PDT 2003
//    Added friend access for EdgeHashTable so it can handle EdgeHashEntry
//    objects on Windows.
//
// ****************************************************************************
class MIR_API ZooMIR : public MIR
{
  public:
    ZooMIR();
    virtual ~ZooMIR();

    // do the processing
    bool         ReconstructMesh(vtkDataSet *, avtMaterial *, int);
    virtual bool Reconstruct3DMesh(vtkDataSet *, avtMaterial *);
    virtual bool Reconstruct2DMesh(vtkDataSet *, avtMaterial *);

    // material select everything -- all variables, the mesh, and the material
    // if requested.
    virtual vtkDataSet *GetDataset(std::vector<int>, vtkDataSet *, 
                                   std::vector<avtMixedVariable *>, bool);

    virtual bool SubdivisionOccurred()   {return !noMixedZones;}
    virtual bool NotAllCellsSubdivided() {return false; }

  protected:


    struct ReconstructedCoord
    {
        float x,y,z;
        float weight[MAX_NODES_PER_ZONE];
        int origzone;
    };

    struct ReconstructedZone
    {
        int origzone;
        int startindex;
        int mix_index;
        int mat;
        unsigned char celltype;
        unsigned char nnodes;
    };

    class EdgeHashEntry
    {
      public:
        EdgeHashEntry() : id1(-1), id2(-1), ptId(-1), next(NULL) { }
        ~EdgeHashEntry() { }
 
        void            SetEndpoints(int i1, int i2) { id1 = i1; id2 = i2;              }
        void            SetPointId(int i)            { ptId = i;                        }
        bool            IsMatch(int i1, int i2)      { return (i1 == id1 && i2 == id2); }
        int             GetPointId(void)             { return ptId;                     }
        EdgeHashEntry  *GetNext(void)                { return next;                     }
        void            SetNext(EdgeHashEntry *n)    { next = n;                        }
 
      protected:
        int             id1, id2;
        int             ptId;
        EdgeHashEntry  *next;
    };

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
        EdgeHashTable(int);
        virtual          ~EdgeHashTable();
 
        int               AddPoint(int, int, float);
        EdgeHashEntry    *GetEdge(int, int);
      protected:
        int                             nHashes;
        EdgeHashEntry                 **hashes;
        EdgeHashEntryMemoryManager      emm;

        int               GetKey(int, int);
    };

    friend class CellReconstructor;
    friend class EdgeHashTable;

  protected:
    int                                     origNPoints;
    std::vector<float>                      origXCoords;
    std::vector<float>                      origYCoords;
    std::vector<float>                      origZCoords;
    Array<ReconstructedCoord>               coordsList;
    Array<ReconstructedZone>                zonesList;
    Array<vtkIdType>                        indexList;

    int                                     dimension;
    int                                     nMaterials;
    bool                                    noMixedZones;

    std::vector<int>                        mapMatToUsedMat;
    std::vector<int>                        mapUsedMatToMat;

    vtkPoints                              *outPts;
    vtkDataSet                             *mesh;

  protected:
    bool ReconstructCleanMesh(vtkDataSet *, avtMaterial *);
    void SetUpCoords();

};



#endif
