// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtR2Fstddev.h                             //
// ************************************************************************* //

#ifndef AVT_R2F_STDDEV_H
#define AVT_R2F_STDDEV_H

#include <dbin_exports.h>

#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Fstddev
//
//  Purpose:
//      Turns a derived data relation into a derived data function using the
//      statistical operator "standard deviation".
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2006
//
// ****************************************************************************

class AVTDBIN_API avtR2Fstddev : public avtR2Foperator
{
  public:
                           avtR2Fstddev(int, double);
    virtual               ~avtR2Fstddev();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);
    virtual int            GetNumberOfPasses(void) { return 2; };

  protected:
    double                *running_total_ave;
    double                *running_total_stddev;
    int                   *count;
    int                    pass;
};


#endif


