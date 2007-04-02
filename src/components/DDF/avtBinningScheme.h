// ************************************************************************* //
//                             avtBinningScheme.h                            //
// ************************************************************************* //

#ifndef AVT_BINNING_SCHEME_H
#define AVT_BINNING_SCHEME_H

#include <ddf_exports.h>

class     vtkDataSet;


// ****************************************************************************
//  Class: avtBinningScheme
//
//  Purpose:
//      This module creates a binning that can be used for neighborhood
//      information when constructing a DDR or a DDF.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

class AVTDDF_API avtBinningScheme
{
  public:
                           avtBinningScheme();
    virtual               ~avtBinningScheme();
  
    virtual int            GetBinId(const float *) const = 0;
    virtual int            GetNumberOfBins(void) const = 0;

    virtual vtkDataSet    *CreateGrid(void) const = 0;
};


#endif


