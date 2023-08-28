// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//    Jeremy Meredith, Fri Feb 13 11:05:06 EST 2009
//    Added calculation of output vf's per material, if requested.
//
//    Jeremy Meredith, Tue Jun 18 11:56:22 EDT 2013
//    Output actual volumes/areas, not VF's, and return total vol/area.
//
//    Kathleen Biagas, Thu Aug 11 2022
//    Removed MIRConnectivity from constructor as it isn't used.
//
// ****************************************************************************

class IsovolumeCellReconstructor : public CellReconstructor
{
  public:
    IsovolumeCellReconstructor(vtkDataSet*, avtMaterial*, ResampledMat&,
                               int, int, ZooMIR&);

    double ReconstructCell(int, int, int, vtkIdType*, double*);
};

#endif
