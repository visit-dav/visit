// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIVPNIMRODIntegrator.h                      //
// ************************************************************************* //

#ifndef AVT_IVP_NIMROD_C1_INTEGRATOR_H
#define AVT_IVP_NIMROD_C1_INTEGRATOR_H

#include <avtIVPSolver.h>
#include <ivp_exports.h>

// ****************************************************************************
//  Class: avtIVPNIMRODIntegrator
//
//  Purpose:
//      An implementation of avtIVPSolver which models the Newton's Method
//      for NIMROD C1 Elements.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

class IVP_API avtIVPNIMRODIntegrator: public avtIVPSolver
{
  public:
    avtIVPNIMRODIntegrator();
    ~avtIVPNIMRODIntegrator();

    // begin a new IVP solution
    virtual void     Reset( const double& t_start, const avtVector& y_start );

    // perform a single integration step
    // adaptive stepsize control retries until success or underflow
    virtual Result Step(avtIVPField* field, double t_max,
                        avtIVPStep* ivpstep = NULL);

    virtual void   OnExitDomain();

    virtual void   SetTolerances(const double& reltol, const double& abstol);
    virtual void   SetMaximumDegenerateIterations( const unsigned int& max );

    virtual avtIVPNIMRODIntegrator* Clone() const
    {
        return new avtIVPNIMRODIntegrator( *this );
    }

  protected:
    // state serialization
    virtual void     AcceptStateVisitor(avtIVPStateHelper &aiss);
    
    avtIVPSolver::Result  vpstep(const avtIVPField* field,
                                 avtVector &yCur, double h, avtVector &yInt);

    avtIVPSolver::Result partial_step(const avtIVPField* field,
                                      double *yInt, int iflow, double h, double *yNew);

    int advance(const avtIVPField* field,
                double *x, int iflow, int icomp, double h, double xacc);

    avtVector getBfield(const avtIVPField* field, avtVector y);

    int getBfield(const avtIVPField* field,
                  double *x, int iflow, int icomp,
                  double *Bout, int dflag, double *Bpout);

    int getBfield1(const avtIVPField* field,
                   double *x, int iflow, int icomp, double *Bout,
                   int dflag, double *Bpout);

    int getBfield2(const avtIVPField* field,
                     double *x, int iflow, int icomp, double *Bout,
                     int dflag, double *Bpout);

  private:
    unsigned int max_degenerate_iterations;
    unsigned int degenerate_iterations;
    double stiffness_eps;
    avtVector ys[2];
};
#endif
