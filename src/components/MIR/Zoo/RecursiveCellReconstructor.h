#ifndef RECURSIVE_CELL_RECONSTRUCTOR_H
#define RECURSIVE_CELL_RECONSTRUCTOR_H

#include "CellReconstructor.h"

// ****************************************************************************
//  Class:  RecursiveCellReconstructor
//
//  Purpose:
//    This is the normal, recursive-clipping zoo-based material interface
//    reconstruction.  This class was created to re-use most of the base
//    cell reconstructor for a new isosurface reconstructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 18, 2005
//
// ****************************************************************************

class RecursiveCellReconstructor : public CellReconstructor
{
  public:
    RecursiveCellReconstructor(vtkDataSet*, avtMaterial*, ResampledMat&,
                               int, int, MIRConnectivity&, ZooMIR&);

    void ReconstructCell(int, int, int, int*);
};

#endif
