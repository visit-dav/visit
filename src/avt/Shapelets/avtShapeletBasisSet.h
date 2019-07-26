// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SHAPELET_BASIS_SET_H
#define AVT_SHAPELET_BASIS_SET_H
#include <shapelets_exports.h>
#include <visitstream.h>
#include <vectortypes.h>
#include <avtShapeletBasis2D.h>

// ****************************************************************************
//  Class:  avtShapeletBasisSet
//
//  Purpose:
//    Efficiently generates a set of 2D Shaplet Basis functions, suitable for
//    decomposition.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
//  Modifications:
//
// ****************************************************************************

class AVTSHAPELETS_API avtShapeletBasisSet
{
  public:
    avtShapeletBasisSet(double beta, int nmax, int width, int height);
    virtual ~avtShapeletBasisSet();
    
    double  Beta()   const { return beta;} 
    int     NMax()   const { return  nmax;}
    
    int     Width()  const { return width;}
    int     Height() const { return height;}

    avtShapeletBasis2D  *GetBasis(int n1, int n2);

    void                 ToVisIt(const std::string &base_fname);
  
  private:
    std::vector<avtShapeletBasis2D *> basisSet;
    
    double      beta;
    int         nmax;
    int         width;
    int         height;
    int         basisSize;
};



#endif

