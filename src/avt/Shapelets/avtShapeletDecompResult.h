// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_SHAPELET_DECOMP_RESULT_H
#define AVT_SHAPELET_DECOMP_RESULT_H
#include <shapelets_exports.h>
#include <visitstream.h>
#include <vectortypes.h>

// ****************************************************************************
//  Class:  avtShapeletDecompResult
//
//  Purpose:
//    Holds the a shapelet decomposition result for an image. 
//
//  Programmer:  Cyrus Harrison
//  Creation:    December 12, 2007
//
//  Modifications:
//    Cyrus Harrison, Wed Jun 11 15:29:15 PDT 2008
//    Added support to retain the original data extents.
//
// ****************************************************************************

class AVTSHAPELETS_API avtShapeletDecompResult
{
  public:
    avtShapeletDecompResult(double beta, 
                            int nmax, 
                            int width, 
                            int height,
                            const doubleVector& extents,
                            const doubleVector& coeffs);

    avtShapeletDecompResult(const avtShapeletDecompResult&);
    avtShapeletDecompResult &operator=(const avtShapeletDecompResult&);
    virtual ~avtShapeletDecompResult();

    double              Beta()    const       { return beta;} 
    int                 NMax()    const       { return  nmax;}
    int                 Width()   const       { return width;}
    int                 Height()  const       { return height;}
    const doubleVector &Extents() const       { return extents;}
    const doubleVector &Coefficients() const  { return coeffs;}
    double              Coefficient(int,int) const;
    

    // pretty print 
    std::string         ToString() const;

  private:
    double       beta;
    int          nmax;
    int          width;
    int          height;
    doubleVector extents;
    doubleVector coeffs;
};



#endif

