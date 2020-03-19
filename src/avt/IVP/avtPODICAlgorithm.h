// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtPODICAlgorithm.h                          //
// ************************************************************************* //

#ifndef AVT_POD_IC_ALGORITHM_H
#define AVT_POD_IC_ALGORITHM_H

#ifdef PARALLEL
#include "avtParICAlgorithm.h"

// ****************************************************************************
// Class:  avtPODICAlgorithm
//
// Purpose: Parallelize over Data IC algorithm.
//   
//
// Programmer:  Dave Pugmire
// Creation:    March 21, 2012
//
// ****************************************************************************


class avtPODICAlgorithm : public avtParICAlgorithm
{
  public:
    avtPODICAlgorithm(avtPICSFilter *picsFilter, int count);
    virtual ~avtPODICAlgorithm();
    virtual const char*       AlgoName() const {return "POD";}

    virtual void              Initialize(std::vector<avtIntegralCurve *> &);
    virtual void              AddIntegralCurves(std::vector<avtIntegralCurve*> &sls);

  protected:
    virtual void              RunAlgorithm();
    virtual void              PreRunAlgorithm();
    bool                      HandleCommunication();
    
    int maxCount;
};

#endif
#endif
