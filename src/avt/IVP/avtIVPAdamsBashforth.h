// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtIVPAdamsBashforth.h                      //
// ************************************************************************* //

#ifndef AVT_IVPADAMSBASHFORTH_H
#define AVT_IVPADAMSBASHFORTH_H

#include <avtIVPSolver.h>
#include <ivp_exports.h>

#define ADAMS_BASHFORTH_NSTEPS 5

// ****************************************************************************
//  Class: avtIVPAdamsBashforth
//
//  Purpose:
//      An implementation of avtIVPSolver which models the 5th-order 
//      Adams-Bashforth multi-step method.
//
//  Programmer: Dave Pugmire
//  Creation:   August 5, 2008
//
//  Modifications:
//    Dave Pugmire, Fri Aug  8 16:05:34 EDT 2008
//    Improved version of A-B solver that builds function history from
//    initial HK4 steps.
//
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Changed how distanced based termination is computed.
//
//    Dave Pugmire, Wed Aug 20, 12:54:44 EDT 2008
//    Add a tolerance and counter for handling stiffness detection.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion.
//
//    Dave Pugmire, Tue Feb 24 10:49:33 EST 2009
//    Replaced Euler step with RK4 step. Removed the Moulton corrector.
//
//    Dave Pugmire, Mon Mar  9 15:35:05 EDT 2009
//    Fix serialization for parallel integration.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

class IVP_API avtIVPAdamsBashforth: public avtIVPSolver
{
  public:
    avtIVPAdamsBashforth();
    ~avtIVPAdamsBashforth();

    // begin a new IVP solution
    virtual void     Reset( const double& t_start,
                            const avtVector &y_start,
                            const avtVector& v_start = avtVector(0,0,0) );

    // perform a single integration step
    // adaptive stepsize control retries until success or underflow
    virtual Result Step(avtIVPField* field, double t_max,
                        avtIVPStep* ivpstep = NULL);

    virtual void   OnExitDomain();

    virtual void   SetTolerances(const double& reltol, const double& abstol);

    virtual avtIVPAdamsBashforth* Clone() const
    {
        return new avtIVPAdamsBashforth( *this );
    }

  protected:
    // state serialization
    virtual void     AcceptStateVisitor(avtIVPStateHelper &aiss);
    
  private:
    int abCIndex, abNSteps;
    avtVector history[ADAMS_BASHFORTH_NSTEPS];
//    avtVector dhistory[ADAMS_BASHFORTH_NSTEPS];
};

#endif
