// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "avtShapeletDecompResult.h"
#include <fstream>
#include <sstream>
using namespace std;

// ****************************************************************************
//  Method:  avtShapeletDecompResult::avtShapeletDecompResult
//
//  Purpose:
//     Constructs a shapelet decomposition result.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
//  Modifications:
//    Cyrus Harrison, Wed Jun 11 15:29:15 PDT 2008
//    Added support to retain the original data extents.
//
// ****************************************************************************

avtShapeletDecompResult::avtShapeletDecompResult(double beta,
                                                 int nmax, 
                                                 int width,
                                                 int height,
                                                 const doubleVector &extents,
                                                 const doubleVector &coeffs)
: beta(beta),
  nmax(nmax),
  width(width), height(height),
  extents(extents),
  coeffs(coeffs)
{;}

// ****************************************************************************
//  Method:  avtShapeletDecompResult::avtShapeletDecompResult
//
//  Purpose:
//     Copy constructor for shaplet decomposition result.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
// ****************************************************************************

avtShapeletDecompResult::avtShapeletDecompResult
(const avtShapeletDecompResult& dresult)
: beta(dresult.beta),
  nmax(dresult.nmax),
  width(dresult.width), height(dresult.height),
  coeffs(dresult.coeffs)
{;}

// ****************************************************************************
//  Method:  avtShapeletDecompResult::avtShapeletDecompResult
//
//  Purpose:
//     Assignment operator for shapelet decomp result.
//  
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
//  Modifications:
//    Kathleen Bonnell, Wed Jan  2 08:15:07 PST 2008
//    Added return.
//
// ****************************************************************************
avtShapeletDecompResult &
avtShapeletDecompResult::operator=(const avtShapeletDecompResult& dresult)
{
    if(this != &dresult)
    {
        beta   = dresult.beta;
        nmax   = dresult.nmax;
        width  = dresult.width;
        height = dresult.height;
        coeffs = dresult.coeffs;
    }
    return *this;
}


// ****************************************************************************
//  Method:  avtShapeletDecompResult::~avtShapeletDecompResult
//
//  Purpose:
//     Shapelet decomp result destructor.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
// ****************************************************************************

avtShapeletDecompResult::~avtShapeletDecompResult()
{;}

// ****************************************************************************
//  Method:  avtShapeletDecompResult::Coefficent
//
//  Purpose:
//     Gets the coefficent value of basis function n1,n2
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

double 
avtShapeletDecompResult::Coefficient(int n1, int n2) const
{
    // mapping: basis(i,j) is @ 2(i-1) + j+1
    if( (n1 <0 || n1 >= nmax) || (n2 <0 || n2 >= nmax) )
        return 0.0;
    int idx = (nmax - n2);
    idx = static_cast<int>(coeffs.size()) - (idx*idx + idx)/2 + n1;
    return coeffs[idx];
}


// ****************************************************************************
//  Method:  avtShapeletDecompResult::ToString
//
//  Purpose:
//     Creates human readable string rep for the shapelet decomp result.
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 13, 2007
//
// ****************************************************************************

string
avtShapeletDecompResult::ToString() const
{
    ostringstream oss;
    oss <<  "Beta   = " << beta   << endl
        <<  "NMax   = " << nmax   << endl
        <<  "Width  = " << width  << endl
        <<  "Height = " << height << endl
        <<  "Coefficients:" << endl
        <<  "[coeffs.size] = " << coeffs.size() << endl;
    for(size_t i=0; i < coeffs.size(); i++)
        oss << coeffs[i] << " ";
    oss  << endl;
    return oss.str();
}



