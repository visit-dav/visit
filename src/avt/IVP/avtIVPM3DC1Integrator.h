/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
    virtual void     Reset( const double& t_start, const avtVector& y_start );

    // perform a single integration step
    // adaptive stepsize control retries until success or underflow
    virtual Result   Step(const avtIVPField* field,
                          const TerminateType &termType,
                          const double &end,
                          avtIVPStep* ivpstep = NULL);

    virtual void    OnExitDomain();

    virtual avtVector GetCurrentY() const;
    virtual double GetCurrentT() const;
    virtual double GetNextStepSize() const;
    virtual double GetMaximumStepSize() const;

    virtual void   SetCurrentY( const avtVector &newY );
    virtual void   SetCurrentT( double newT );
    virtual void   SetNextStepSize( const double& h );
    virtual void   SetMaximumStepSize( const double& hMax );

    virtual void   SetMaximumDegenerateIterations( const unsigned int& max );

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
    int numStep;
    double tol;
    double h, h_max;
    double t, d;
    unsigned int max_degenerate_iterations;
    unsigned int degenerate_iterations;
    double stiffness_eps;
    avtVector yCur;
    avtVector ys[2];
};

#endif
