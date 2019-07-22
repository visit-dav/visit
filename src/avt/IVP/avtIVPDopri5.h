// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtIVPDopri5.h                             //
// ************************************************************************* //

#ifndef AVT_IVPDOPRI5_H
#define AVT_IVPDOPRI5_H

#include <avtIVPSolver.h>

#include <ivp_exports.h>

// ****************************************************************************
//  Class: avtIVPDopri5
//
//  Purpose:
//      An implementation of avtIVPSolver which models the 5th-order 
//      Runge-Kutta dense-output scheme given by Dormand & Prince
//
//      It follows this article:
//      @article{Prince:1981:High,
//       Author = {Prince, P. J. and Dormand, J. R.},
//       Journal = {Journal of Computational and Applied Mathematics},
//       Number = {1},
//       Title = {High order embedded Runge-Kutta formulae},
//       Volume = {7},
//       Year = {1981}}
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//    Dave Pugmire, Tue Aug 19, 17:38:03 EDT 2008
//    Chagned how distanced based termination is computed.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    Reworked the termination code. Added a type enum and value. Made num steps
//    a termination criterion.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

class IVP_API avtIVPDopri5: public avtIVPSolver
{
  public:
                     avtIVPDopri5();
                     avtIVPDopri5(const double& t_start, 
                                  const avtVector& y_start);
                     ~avtIVPDopri5();

    // begin a new IVP solution
    virtual void     Reset( const double& t_start,
                            const avtVector& y_start,
                            const avtVector& v_start = avtVector(0,0,0) );

    // perform a single integration step
    // adaptive stepsize control retries until success or underflow
    virtual Result   Step(avtIVPField* field, double t_max,
                          avtIVPStep* ivpstep = NULL);

    virtual void     SetTolerances(const double& reltol, const double& abstol);
    
    virtual avtIVPDopri5* Clone() const { return new avtIVPDopri5( *this ); }

  protected: 
    // Guess an initial step size if none is specified by the user
    double           GuessInitialStep(const avtIVPField* field, 
                                      const double& h_max,
                                      const double& t_max);

    // state serialization
    virtual void     AcceptStateVisitor(avtIVPStateHelper &aiss);

  private:
    double reltol;
    double abstol;

    double h_init;

    unsigned int n_accepted;
    unsigned int n_rejected;
    unsigned int n_steps;
    unsigned int n_eval;

    double facold;      // stepsize control stabilization
    double hlamb;       // stiffness detection
    int iasti;
    int nonsti;

    avtVector k1;
};

#endif
