// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "avtShapeletBasisSet.h"
#include <fstream>
#include <sstream>
using namespace std;

// ****************************************************************************
//  Method:  avtShapeletBasisSet::avtShapeletBasisSet
//
//  Purpose:
//     Constructs a basis set suitable for decomposition and reconstruction.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasisSet::avtShapeletBasisSet(double beta,
                                         int nmax, 
                                         int width,
                                         int height)
: beta(beta),
  nmax(nmax),
  width(width), height(height)
{
    basisSize = (nmax*nmax + nmax)/2;
    vector<avtShapeletBasis1D *> x_basis_set;
    vector<avtShapeletBasis1D *> y_basis_set;
    
    x_basis_set.resize(nmax);
    for(int i=0;i<nmax;i++)
        x_basis_set[i] = new avtShapeletBasis1D(beta,i,width);
     
    // only create separate 1D y basis if needed   
    if(width != height)
    {
        y_basis_set.resize(nmax);
        for(int i=0;i<nmax;i++)
            y_basis_set[i] = new avtShapeletBasis1D(beta,i,height);
    }
    // resize basisSet with proper # of entries:

    basisSet.resize(basisSize);
    int idx = 0;
    for(int j=0;j<=nmax;j++)
    {
        for(int i=0;i<nmax-j;i++)
        {
            if(width == height)
                basisSet[idx] = new avtShapeletBasis2D(x_basis_set[i],
                                                       x_basis_set[j]);
            else
                basisSet[idx] = new avtShapeletBasis2D(y_basis_set[i],
                                                       y_basis_set[j]);
            idx++;
        }
    }
    
    for(size_t i=0;i<x_basis_set.size();i++)
        delete x_basis_set[i];
    for(size_t i=0;i<y_basis_set.size();i++)
        delete y_basis_set[i];
}


// ****************************************************************************
//  Method:  avtShapeletBasisSet::~avtShapeletBasisSet
//
//  Purpose:
//     Shapelet Basis destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasisSet::~avtShapeletBasisSet()
{
    for(size_t i=0;i<basisSet.size();i++)
        delete basisSet[i];
}

// ****************************************************************************
//  Method:  avtShapeletBasisSet::GetBasis
//
//  Purpose:
//     Gets a 2D basis object from the set. Returns NULL if the n1/n2 combo
//     is invalid.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasis2D  *
avtShapeletBasisSet::GetBasis(int n1, int n2)
{
    // mapping: basis(i,j) is @ 2(i-1) + j+1
    if( (n1 <0 || n1 >= nmax) || (n2 <0 || n2 >= nmax) )
        return NULL;
    int idx = (nmax - n2);
    idx = basisSize - (idx*idx + idx)/2 + n1;
    return basisSet[idx];
}
