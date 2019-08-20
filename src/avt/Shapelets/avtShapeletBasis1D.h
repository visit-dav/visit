// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SHAPELET_BASIS_1D_H
#define AVT_SHAPELET_BASIS_1D_H
#include <shapelets_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <string>

// ****************************************************************************
//  Class:  avtShapeletBasis1D
//
//  Purpose:
//    Represents a sampled dimensional 1D shapelet basis function.
//    The basis is sampled between [-length /2,+length/2] in increments of 1.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
//  Modifications:
//
// ****************************************************************************

class AVTSHAPELETS_API avtShapeletBasis1D
{
  public:
    avtShapeletBasis1D(double beta, int n, int length);
    virtual ~avtShapeletBasis1D();
    
    double        Beta()            const { return beta;} 
    int           N()               const { return n;}
    int           Length()          const { return length;}
    double        ValueAt(int i)    const { return values[i];}
    double        operator[](int i) const { return values[i];}
    const double *Values()          const { return &values[0];}

  private:
    double        beta;
    int           n;
    int           length;
    doubleVector  values;
};



#endif
