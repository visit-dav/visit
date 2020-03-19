// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtBinningScheme.h                            //
// ************************************************************************* //

#ifndef AVT_BINNING_SCHEME_H
#define AVT_BINNING_SCHEME_H

#include <dbin_exports.h>

#include <ConstructDataBinningAttributes.h>

class     vtkDataSet;


// ****************************************************************************
//  Class: avtBinningScheme
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
//    Added the concept of out-of-bounds behavior.
//
// ****************************************************************************

class AVTDBIN_API avtBinningScheme
{
  public:
                           avtBinningScheme();
    virtual               ~avtBinningScheme();
  
    virtual int            GetBinId(const float *) const = 0;
    virtual int            GetNumberOfBins(void) const = 0;
    virtual int            GetNumberOfDimensions(void) const = 0;

    void                   SetOutOfBoundsBehavior(ConstructDataBinningAttributes::OutOfBoundsBehavior o)  
                                 { oobb = o; };

    virtual vtkDataSet    *CreateGrid(void) const = 0;

  protected:
    ConstructDataBinningAttributes::OutOfBoundsBehavior   oobb;
};


#endif


