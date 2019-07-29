// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 avtR2Frms.h                               //
// ************************************************************************* //

#ifndef AVT_R2F_RMS_H
#define AVT_R2F_RMS_H

#include <dbin_exports.h>
#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Frms
//
//  Purpose:
//      Turns a derived data relation into a derived data function via
//      an "rms" calculation.
//
//      rms = sqrt((x0^2 + x1^2 + x2^2 + ... + xN^2)/N)
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Aug 10 10:34:46 PDT 2010
//
//
//  Modifications:
//
// ****************************************************************************

class AVTDBIN_API avtR2Frms : public avtR2Foperator
{
  public:
                           avtR2Frms(int, double);
    virtual               ~avtR2Frms();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);

  protected:
    double                *running_total;
    int                   *count;
};


#endif


