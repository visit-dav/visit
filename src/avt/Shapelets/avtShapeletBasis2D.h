// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SHAPELET_BASIS_2D_H
#define AVT_SHAPELET_BASIS_2D_H
#include <shapelets_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include "avtShapeletBasis1D.h"

// ****************************************************************************
//  Class:  avtShapeletBasis2D
//
//  Purpose:
//    Provides a sampled 2D Shapelet Basis function.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 7, 2007
//
//  Modifications:
//
// ****************************************************************************

class AVTSHAPELETS_API avtShapeletBasis2D
{
  public:
    avtShapeletBasis2D(const avtShapeletBasis1D *x_basis,
                       const avtShapeletBasis1D *y_basis);

    virtual ~avtShapeletBasis2D();
    
    double        Beta()                const { return beta;} 
    int           N1()                  const { return n1;}
    int           N2()                  const { return n2;}
    int           Width()               const { return width;}
    int           Height()              const { return height;}
    double        ValueAt(int i, int j) const { return values[i+width*j];}
    double        operator[](int i)     const { return values[i];}
    const double *Values()              const { return &values[0];}
    
  private:
    double      beta;
    int         n1;
    int         n2;

    int         width;
    int         height;
    
    doubleVector  values;

};



#endif

