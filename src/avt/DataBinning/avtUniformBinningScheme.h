// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtUniformBinningScheme.h                        //
// ************************************************************************* //

#ifndef AVT_UNIFORM_BINNING_SCHEME_H
#define AVT_UNIFORM_BINNING_SCHEME_H

#include <dbin_exports.h>

#include <avtBinningScheme.h>


// ****************************************************************************
//  Class: avtUniformBinningScheme
//
//  Purpose:
//      This module creates a binning that can be used for neighborhood
//      information when constructing a data binning.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Added a method to return the number of dimensions.
//
// ****************************************************************************

class AVTDBIN_API avtUniformBinningScheme : public avtBinningScheme
{
  public:
                           avtUniformBinningScheme(int, const float *,
                                                   const int *);
    virtual               ~avtUniformBinningScheme();
  
    virtual int            GetBinId(const float *) const;
    virtual int            GetNumberOfBins(void) const { return nBins; };
    virtual int            GetNumberOfDimensions(void) const 
                                                       { return ntuples; };
    virtual vtkDataSet    *CreateGrid(void) const;

  protected:
    int                    ntuples;
    float                 *ranges;
    int                   *nvals;
    int                    nBins;
};


#endif


