// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CELL_RECONSTRUCTOR_H
#define CELL_RECONSTRUCTOR_H

#include <ZooMIR.h>
#include <VisItArray.h>

// ****************************************************************************
//  Class:  CellReconstructor
//
//  Purpose:
//    Reconstructs a cell from the given mesh/material.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 15, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 18 18:02:38 PDT 2005
//    I was able to re-use most of this class for a new isovolume based
//    reconstruction algorithm.  Everything stayed except I made
//    ReconstructCell pure-virtual, and I needed to keep track of whether
//    or not edge points were shared across materials.
//
//    Mark C. Miller, Thu Feb  9 21:06:10 PST 2006
//    Renamed Array class to VisItArray to avoid name collisions with
//    third-party libs
//
//    Jeremy Meredith, Fri Feb 13 10:56:43 EST 2009
//    Allowed ReconstructCell to take a new argument where it will, if
//    desired, output the volume fractions for the reconstructed materials.
//    Also, added helper function to calculate volume or area.
//
//    Jeremy Meredith, Tue Jun 18 11:56:22 EDT 2013
//    Output actual volumes/areas, not VF's, and return total vol/area,
//    in ReconstructCell.
//
//    Kathleen Biagas, Thu Aug 11 2022
//    Removed MIRConnectivity as it isn't used.
//
// ****************************************************************************
class CellReconstructor
{
  public:
    CellReconstructor(vtkDataSet*, avtMaterial*, ResampledMat&, int, int, bool,
                      ZooMIR&);
    virtual ~CellReconstructor();
    virtual double ReconstructCell(int, int, int, vtkIdType*, double*) = 0;

  protected:
    vtkDataSet                             *mesh;
    avtMaterial                            *mat;
    avtMaterial                            *origMat;
    ResampledMat                           &rm;
    int                                     nPoints;
    int                                     nCells;
    ZooMIR                                 &mir;
    int                                     nMaterials;

    static double CalculateVolumeOrAreaHelper(int celltype,double coords[][3]);

  protected:
    int           cellid;
    int           celltype;
    vtkIdType    *ids;
    int           nids;

    int           nodeIndices[MAX_NODES_PER_ZONE];
    int           nodeIndexLimit[MAX_NODES_PER_ZONE];
    float        *nodeVFs[MAX_NODES_PER_ZONE];
    float         newVFs[MAX_NODES_PER_ZONE];
    float         vfDiff[MAX_NODES_PER_ZONE];
    int          *mix_index;
    int           interpIDs[4];
    float         interpVFs[4];

    bool            allMaterialsSharePoints;
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
    VisItArray<TempCell>   outlist;
    VisItArray<TempCell>   tmplist;

    void CreateCentroidPoint(TempCell&, int, int, int, int, int, int*);
    void CreateOutputShape(TempCell&, TempCell&, int, int, int*, int);

};

#endif
