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
// ****************************************************************************

class IVP_API avtIVPDopri5: public avtIVPSolver
{
  public:
                     avtIVPDopri5();
                     avtIVPDopri5(const double& t_start, 
                                  const avtVecRef& y_start);
                     ~avtIVPDopri5();

    // begin a new IVP solution
    virtual void     Reset( const double& t_start, const avtVecRef& y_start );

    // perform a single integration step
    // adaptive stepsize control retries until success or underflow
    virtual Result   Step(const avtIVPField* field, const double& t_max, 
                          avtIVPStep* ivpstep = NULL);

    virtual avtVec   GetCurrentY() const;
    virtual double   GetCurrentT() const;

    virtual void     SetCurrentY( const avtVec &newY );
    virtual void     SetCurrentT( double newT );

    virtual void     SetNextStepSize( const double& h );
    virtual double   GetNextStepSize() const;
    virtual void     SetMaximumStepSize( const double& h );

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

    double h;
    double h_max;
    double h_init;

    double t, t_max;

    unsigned int n_accepted;
    unsigned int n_rejected;
    unsigned int n_steps;
    unsigned int n_eval;

    double facold;      // stepsize control stabilization
    double hlamb;       // stiffness detection
    int iasti;
    int nonsti;

    avtVec y;
    avtVec k1;
};

#endif


