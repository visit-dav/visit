// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtR2Faverage.h                             //
// ************************************************************************* //

#ifndef AVT_R2F_AVERAGE_H
#define AVT_R2F_AVERAGE_H

#include <dbin_exports.h>

#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Faverage
//
//  Purpose:
//      Turns a derived data relation into a derived data function using the
//      statistical operator "average".
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:24:49 PST 2006
//    Add undefined value in constructor.
//
// ****************************************************************************

class AVTDBIN_API avtR2Faverage : public avtR2Foperator
{
  public:
                           avtR2Faverage(int, double);
    virtual               ~avtR2Faverage();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);

  protected:
    double                *running_total;
    int                   *count;
};


#endif


