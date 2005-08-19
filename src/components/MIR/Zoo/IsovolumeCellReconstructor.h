#ifndef ISOVOLUME_CELL_RECONSTRUCTOR_H
#define ISOVOLUME_CELL_RECONSTRUCTOR_H

#include "CellReconstructor.h"

// ****************************************************************************
//  Class:  IsovolumeCellReconstructor
//
//  Purpose:
//    This is a cell reconstructor that uses the zoo-based clipping methods
//    to create an interface reconstruction based solely on an isosurface
//    of the volume fractions.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 18, 2005
//
// ****************************************************************************

class IsovolumeCellReconstructor : public CellReconstructor
{
  public:
    IsovolumeCellReconstructor(vtkDataSet*, avtMaterial*, ResampledMat&,
                               int, int, MIRConnectivity&, ZooMIR&);

    void ReconstructCell(int, int, int, int*);
};

#endif
