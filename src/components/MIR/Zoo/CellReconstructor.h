#ifndef CELL_RECONSTRUCTOR_H
#define CELL_RECONSTRUCTOR_H

#include <ZooMIR.h>

// ****************************************************************************
//  Class:  CellReconstructor
//
//  Purpose:
//    Reconstructs a cell from the given mesh/material.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
// ****************************************************************************
class CellReconstructor
{
  public:
    CellReconstructor(vtkDataSet*, avtMaterial*, ResampledMat&, int, int,
                      MIRConnectivity&, ZooMIR&);
    ~CellReconstructor();
    void ReconstructCell(int, int, int, int*);

  protected:
    vtkDataSet                             *mesh;
    avtMaterial                            *mat;
    ResampledMat                           &rm;
    int                                     nPoints;
    int                                     nCells;
    MIRConnectivity                        &conn;
    ZooMIR                                 &mir;
    int                                     nMaterials;
  protected:
    int           cellid;
    int           celltype;
    int          *ids;
    int           nids; 

    int           nodeIndices[MAX_NODES_PER_ZONE];
    int           nodeIndexLimit[MAX_NODES_PER_ZONE];
    float        *nodeVFs[MAX_NODES_PER_ZONE];
    float         newVFs[MAX_NODES_PER_ZONE];
    float         vfDiff[MAX_NODES_PER_ZONE];
    int          *mix_index;
    int           interpIDs[4];
    float         interpVFs[4];

    int             startIndex;
    unsigned char  *splitCase;
    int             numOutput;
    typedef int     edgeIndices[2];
    edgeIndices    *vertices_from_edges;

    struct TempCell
    {
        int mat;
        int celltype;
        int nnodes;
        int mix_index;
        int ids[MAX_NODES_PER_ZONE];
        int localids[MAX_NODES_PER_ZONE];
        float vfs[MAX_NODES_PER_ZONE];
    };

    ZooMIR::EdgeHashTable  edges;
    Array<TempCell>        outlist;
    Array<TempCell>        tmplist;

    void CreateCentroidPoint(TempCell&, int, int, int, int, int, int*);
    void CreateOutputShape(TempCell&, TempCell&, int, int, int*, int);

};

#endif
