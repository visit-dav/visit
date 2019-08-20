// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtR2Fcount.h                             //
// ************************************************************************* //

#ifndef AVT_R2F_COUNT_H
#define AVT_R2F_COUNT_H

#include <dbin_exports.h>

#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Fcount
//
//  Purpose:
//      Turns a derived data relation into a derived data function by
//      consistently counting the number of elements.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Jan 10 16:02:19 EST 2008
//
// ****************************************************************************

class AVTDBIN_API avtR2Fcount : public avtR2Foperator
{
  public:
                           avtR2Fcount(int);
    virtual               ~avtR2Fcount();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);

  protected:
    float                 *count;
};


#endif


