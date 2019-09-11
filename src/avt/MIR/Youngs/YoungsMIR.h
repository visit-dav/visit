// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef YOUNGS_MIR_H
#define YOUNGS_MIR_H

#include <MIR.h>
#include <MIRConnectivity.h>
#include <VisItArray.h>
#include <vector>

// ****************************************************************************
//  Class:  YoungsMIR
//
//  Purpose:
//    Youngs MIR algorithm.  Wraps code contributed by Thierry Carrard.
//    This algorithm does not try to fix up connectivity between cells.
//    As such, cells with interfaces will have boundary surfaces/lines
//    at the cell edges for cells which have interfaces.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  2, 2009
//
//  Modifications:
//    Kathleen Biagas, Wed Dec 18 11:20:33 PST 2013
//    Add nmats to track original number of materials.
//
// ****************************************************************************
class MIR_API YoungsMIR : public MIR
{
  public:
    YoungsMIR();
    virtual ~YoungsMIR();

    // do the processing
    bool         ReconstructMesh(vtkDataSet *, avtMaterial *, int);
    virtual bool Reconstruct3DMesh(vtkDataSet *, avtMaterial *);
    virtual bool Reconstruct2DMesh(vtkDataSet *, avtMaterial *);

    // material select everything -- all variables, the mesh, and the material
    // if requested.
    virtual vtkDataSet *GetDataset(std::vector<int>, vtkDataSet *, 
                                   std::vector<avtMixedVariable *>, bool,
                                   avtMaterial * = NULL);

    virtual bool SubdivisionOccurred()   {return true; }
    virtual bool NotAllCellsSubdivided() {return false; }

  protected:
    vtkDataSet **output;
    std::vector<int> mapMatToUsedMat;
    std::vector<int> mapUsedMatToMat;
    int nmats;
};



#endif
