// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   avtRelativeValueSamplePointArbitrator.h                 //
// ************************************************************************* //

#ifndef AVT_RELATIVE_VALUE_SAMPLE_POINT_ARBITRATOR_H
#define AVT_RELATIVE_VALUE_SAMPLE_POINT_ARBITRATOR_H

#include <pipeline_exports.h>

#include <avtSamplePointArbitrator.h>

class    avtRelativeValue;


// ****************************************************************************
//  Class: avtRelativeValueSamplePointArbitrator
//
//  Purpose:
//      Decides which sample point to use when multiple cells map to the same
//      sample.  This will choose the one with higher relative value.  Or
//      lower relative value.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2005
//
// ****************************************************************************

class PIPELINE_API avtRelativeValueSamplePointArbitrator 
    : public avtSamplePointArbitrator
{
  public:
                      avtRelativeValueSamplePointArbitrator(bool, int);
    virtual          ~avtRelativeValueSamplePointArbitrator();

    virtual bool      ShouldOverwrite(double, double);

  protected:
    bool              lessThan;
};


#endif


