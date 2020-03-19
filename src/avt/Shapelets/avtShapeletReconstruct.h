// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SHAPELET_RECONSTRUCT_H
#define AVT_SHAPELET_RECONSTRUCT_H
#include <shapelets_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <avtShapeletBasisSet.h>
#include <avtShapeletDecompResult.h>
#include <vtkRectilinearGrid.h>

// ****************************************************************************
//  Class:  avtShapeletReconstruct
//
//  Purpose:
//    Reconstructs an image from a shapelet decomposition.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
//  Modifications:
//
// ****************************************************************************

class AVTSHAPELETS_API avtShapeletReconstruct
{
  public:
    avtShapeletReconstruct();
    virtual ~avtShapeletReconstruct();

    vtkRectilinearGrid  *Execute(avtShapeletDecompResult *decomp,
                                 const std::string &var_name);
    
    vtkRectilinearGrid  *Execute(avtShapeletDecompResult *decomp,
                                 const std::string &var_name,
                                 avtShapeletBasisSet *basis_set);                                 
};



#endif

