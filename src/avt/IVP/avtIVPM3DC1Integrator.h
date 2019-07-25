// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIVPM3DC1Integrator.h                      //
// ************************************************************************* //

#ifndef AVT_IVP_M3D_C1_INTEGRATOR_H
#define AVT_IVP_M3D_C1_INTEGRATOR_H

#include <avtIVPSolver.h>
#include <ivp_exports.h>

// ****************************************************************************
//  Class: avtIVPM3DC1Integrator
//
//  Purpose:
//      An implementation of avtIVPSolver which models the Newton's Method
//      for M3D C1 Elements.
//
//  Programmer: Allen Sanderson
//  Creation:   October 24, 2009
//
// ****************************************************************************

class IVP_API avtIVPM3DC1Integrator: public avtIVPSolver
{
  public:
    avtIVPM3DC1Integrator();
    ~avtIVPM3DC1Integrator();

    // begin a new IVP solution
    virtual void     Reset( const double& t_start,
                            const avtVector& y_start,
                            const avtVector& v_start = avtVector(0,0,0) );

    // perform a single integration step
    // adaptive stepsize control retries until success or underflow
    virtual Result   Step(avtIVPField* field, double t_max,
                          avtIVPStep* ivpstep = NULL);

    virtual void    OnExitDomain();

    virtual void   SetTolerances(const double& reltol, const double& abstol);

    virtual avtIVPM3DC1Integrator* Clone() const
    {
        return new avtIVPM3DC1Integrator( *this );
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
    avtVector ys[2];
};

#endif
