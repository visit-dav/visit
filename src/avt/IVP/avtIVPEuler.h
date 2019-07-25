// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtIVPEuler.h                      //
// ************************************************************************* //

#ifndef AVT_IVPEULER_H
#define AVT_IVPEULER_H

#include <avtIVPSolver.h>
#include <ivp_exports.h>

// ****************************************************************************
//  Class: avtIVPEuler
//
//  Purpose:
//      An implementation of avtIVPSolver which models the
//      Euler multi-step method.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
// ****************************************************************************

class IVP_API avtIVPEuler: public avtIVPSolver
{
  public:
    avtIVPEuler();
    ~avtIVPEuler();

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

    virtual avtIVPEuler* Clone() const
    {
        return new avtIVPEuler( *this );
    }

  protected:
    // state serialization
    virtual void     AcceptStateVisitor(avtIVPStateHelper &aiss);
};

#endif
