// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtIVPLeapfrog.h                      //
// ************************************************************************* //

#ifndef AVT_IVPLEAPFROG_H
#define AVT_IVPLEAPFROG_H

#include <avtIVPSolver.h>
#include <ivp_exports.h>

// ****************************************************************************
//  Class: avtIVPLeapfrog
//
//  Purpose:
//      An implementation of avtIVPSolver which models the
//      Leapfrog single step method.
//
//  Creationist: Allen Sanderson
//  Creation:   20 June 2011
//
// ****************************************************************************

class IVP_API avtIVPLeapfrog: public avtIVPSolver
{
  public:
    avtIVPLeapfrog();
    ~avtIVPLeapfrog();

    // begin a new IVP solution
    virtual void     Reset( const double& t_start,
                            const avtVector &y_start,
                            const avtVector& v_start = avtVector(0,0,0) );

    // perform a single integration step
    // adaptive stepsize control retries until success or underflow
    virtual Result   Step(avtIVPField* field, double t_max,
                          avtIVPStep* ivpstep = NULL);
    virtual void    OnExitDomain();

    virtual avtIVPLeapfrog* Clone() const
    {
        return new avtIVPLeapfrog( *this );
    }

  protected:
    bool firstStep;

    // state serialization
    virtual void     AcceptStateVisitor(avtIVPStateHelper &aiss);
};

#endif
