// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtDataBinningConstructor.h                       //
// ************************************************************************* //

#ifndef AVT_DATA_BINNING_CONSTRUCTOR_H
#define AVT_DATA_BINNING_CONSTRUCTOR_H

#include <dbin_exports.h>

#include <avtTerminatingDatasetSink.h>

#include <vector>
#include <string>


class     ConstructDataBinningAttributes;

class     avtDataBinning;


// ****************************************************************************
//  Class: avtDataBinningConstructor
//
//  Purpose:
//      This is a data set sink that will generate a data binning.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Renamed from DDF to DataBinning.
//
// ****************************************************************************

class AVTDBIN_API avtDataBinningConstructor : public virtual avtTerminatingDatasetSink
{
  public:
                           avtDataBinningConstructor();
    virtual               ~avtDataBinningConstructor();
  
    avtDataBinning        *ConstructDataBinning(ConstructDataBinningAttributes *,
                                                avtContract_p,
                                                bool mustReExecute = true);
};


#endif


