// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtR2Fsum.h                               //
// ************************************************************************* //

#ifndef AVT_R2F_SUM_H
#define AVT_R2F_SUM_H

#include <dbin_exports.h>

#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Fsum
//
//  Purpose:
//      Turns a derived data relation into a derived data function by
//      consistently taking the sum.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

class AVTDBIN_API avtR2Fsum : public avtR2Foperator
{
  public:
                           avtR2Fsum(int);
    virtual               ~avtR2Fsum();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);

  protected:
    float                 *sum;
};


#endif


