// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                    avtIVPRK4.h                            //
// ************************************************************************* //

#ifndef AVT_IVPRK4_H
#define AVT_IVPRK4_H

#include <avtIVPSolver.h>
#include <ivp_exports.h>

// ****************************************************************************
//  Class: avtIVPRK4
//
//  Purpose:
//      An implementation of avtIVPSolver which models the
//      RK4 single-step method.
//
//  Programmer: Christoph Garth
//  Creation:   March 6, 2012
//
// ****************************************************************************

class IVP_API avtIVPRK4: public avtIVPSolver
{
  public:
    avtIVPRK4();
    ~avtIVPRK4();

    // begin a new IVP solution
    virtual void     Reset( const double& t_start,
                            const avtVector &y_start,
                            const avtVector &v_start = avtVector(0,0,0) );

    // perform a single integration step
    // adaptive stepsize control retries until success or underflow
    virtual Result   Step(avtIVPField* field,
                          double t_max,
                          avtIVPStep* ivpstep = NULL);

    virtual void    OnExitDomain();

    virtual avtIVPRK4* Clone() const
    {
        return new avtIVPRK4( *this );
    }

  protected:
    // state serialization
    virtual void     AcceptStateVisitor(avtIVPStateHelper &aiss);
};

#endif
