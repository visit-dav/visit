/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef ZOOMIR_H
#define ZOOMIR_H

#include <MIR.h>
#include <MIRConnectivity.h>
#include <VisItArray.h>

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
//    Jeremy Meredith, Wed Oct 15 16:47:49 PDT 2003
//    Added space for a material to get passed in to GetDataset.
//
//    Jeremy Meredith, Tue Oct 21 11:24:42 PDT 2003
//    Added data member to track original number of materials.
//
//    Jeremy Meredith, Thu Aug 18 17:59:46 PDT 2005
//    Added a new member of the edge hash entries -- the material number.
//    This is because we are now also using this for an isovolume MIR method
//    where we do NOT want the edge points to be reused across materials.
//
//    Mark C. Miller, Thu Feb  9 21:06:10 PST 2006
//    Renamed Array class to VisItArray to avoid name collisions with
//    third-party libs
//
//    Hank Childs, Fri Jun  9 13:47:29 PDT 2006
//    Reorder member initializers to match declaration order (and remove
//    compiler warning).
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
                                   std::vector<avtMixedVariable *>, bool,
                                   avtMaterial * = NULL);

    virtual bool SubdivisionOccurred()   {return !noMixedZones;}
    virtual bool NotAllCellsSubdivided() {return false; }

  protected:


    struct ReconstructedCoord
    {
        double x,y,z;
        double weight[MAX_NODES_PER_ZONE];
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
        EdgeHashEntry() : id1(-1), id2(-1), matno(-1), ptId(-1), next(NULL) { }
        ~EdgeHashEntry() { }
 
        void            SetEndpoints(int i1, int i2) { id1 = i1; id2 = i2;              }
        void            SetMatNo(int m)              { matno = m;                       }
        void            SetPointId(int i)            { ptId = i;                        }
        bool            IsMatch(int i1, int i2,int m){ return (i1 == id1 && i2 == id2 && m == matno); }
        int             GetPointId(void)             { return ptId;                     }
        EdgeHashEntry  *GetNext(void)                { return next;                     }
        void            SetNext(EdgeHashEntry *n)    { next = n;                        }
 
      protected:
        int             id1, id2;
        int             matno;
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
 
        EdgeHashEntry    *GetEdge(int, int, int);
      protected:
        int                             nHashes;
        EdgeHashEntry                 **hashes;
        EdgeHashEntryMemoryManager      emm;

        int               GetKey(int, int, int);
    };

    friend class CellReconstructor;
    friend class RecursiveCellReconstructor;
    friend class IsovolumeCellReconstructor;
    friend class EdgeHashTable;

  protected:
    int                                     origNPoints;
    std::vector<float>                      origXCoords;
    std::vector<float>                      origYCoords;
    std::vector<float>                      origZCoords;
    VisItArray<ReconstructedCoord>          coordsList;
    VisItArray<ReconstructedZone>           zonesList;
    VisItArray<vtkIdType>                   indexList;

    int                                     dimension;
    int                                     nMaterials;
    int                                     nOrigMaterials;
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
