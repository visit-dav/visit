// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SHAPELET_DECOMPOSE_H
#define AVT_SHAPELET_DECOMPOSE_H
#include <shapelets_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <avtShapeletBasisSet.h>
#include <avtShapeletDecompResult.h>

#include <vtkRectilinearGrid.h>

// ****************************************************************************
//  Class:  avtShapeletDecompose
//
//  Purpose:
//    Decomposes an image into shapelet coefficents using the overlap
//    integral method. Other methods should be evaluated in the future.
//    (ex: least squares method)
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
//  Modifications:
//
// ****************************************************************************

class AVTSHAPELETS_API avtShapeletDecompose
{
  public:
    avtShapeletDecompose();
    virtual ~avtShapeletDecompose();
    
    avtShapeletDecompResult *Execute(vtkRectilinearGrid *rgrid,
                                     const std::string &var_name,
                                     avtShapeletBasisSet *basis_set);
};



#endif

