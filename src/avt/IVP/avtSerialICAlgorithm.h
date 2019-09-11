// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtSerialICAlgorithm.h                         //
// ************************************************************************* //

#ifndef AVT_SERIAL_IC_ALGORITHM_H
#define AVT_SERIAL_IC_ALGORITHM_H

#include "avtICAlgorithm.h"

// ****************************************************************************
// Class: avtSerialICAlgorithm
//
// Purpose:
//    A class for performing serial integral curve integration.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of integral curves.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Change Execute to RunAlgorithm.
//
//   Dave Pugmire, Thu Dec  2 11:21:06 EST 2010
//   Add CheckNextTimeStepNeeded.
//
// ****************************************************************************

class avtSerialICAlgorithm : public avtICAlgorithm
{
  public:
    avtSerialICAlgorithm( avtPICSFilter *picsFilter );
    virtual ~avtSerialICAlgorithm();
    
    virtual const char*       AlgoName() const {return "Serial";}
    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              RestoreInitialize(std::vector<avtIntegralCurve *> &, int curTimeSlice);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &ics);

  protected:
    virtual void              RunAlgorithm();
    void                      ActivateICs();
};

#endif
