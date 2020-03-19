// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtShapeletBasis2D.h>
#include <math.h>
using namespace std;


// ****************************************************************************
//  Method:  avtShapeletTemplate1D::avtShapeletTemplate1D
//
//  Purpose:
//     Constructs a sampled 2D Shapelet Basis function from two 1D basis
//     functions.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasis2D::avtShapeletBasis2D(const avtShapeletBasis1D *x_basis,
                                       const avtShapeletBasis1D *y_basis)
: beta(x_basis->Beta()), 
  n1(x_basis->N()), n2(y_basis->N()),
  width(x_basis->Length()), height(y_basis->Length())
{
    values.resize(width * height);
    // construct the basis with the tensor product of the two 1D basis
    // functions
    int idx=0;
    for(int j=0;j<height;j++)
    {
        for(int i=0;i<width;i++)
        {
            values[idx] = x_basis->ValueAt(i) * y_basis->ValueAt(j);
            idx++;
        }
    }

}


// ****************************************************************************
//  Method:  avtShapeletBasis2D::~avtShapeletBasis2D
//
//  Purpose:
//     Shaplet Basis destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 6, 2007
//
// ****************************************************************************

avtShapeletBasis2D::~avtShapeletBasis2D()
{;}

