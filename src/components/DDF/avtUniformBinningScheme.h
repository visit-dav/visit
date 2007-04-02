// ************************************************************************* //
//                          avtUniformBinningScheme.h                        //
// ************************************************************************* //

#ifndef AVT_UNIFORM_BINNING_SCHEME_H
#define AVT_UNIFORM_BINNING_SCHEME_H

#include <ddf_exports.h>

#include <avtBinningScheme.h>


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

class AVTDDF_API avtUniformBinningScheme : public avtBinningScheme
{
  public:
                           avtUniformBinningScheme(int, const float *,
                                                   const int *);
    virtual               ~avtUniformBinningScheme();
  
    virtual int            GetBinId(const float *) const;
    virtual int            GetNumberOfBins(void) const { return nBins; };
    virtual vtkDataSet    *CreateGrid(void) const;

  protected:
    int                    ntuples;
    float                 *ranges;
    int                   *nvals;
    int                    nBins;
};


#endif


