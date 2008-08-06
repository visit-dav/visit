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
//                                avtIVPAdamsBashforth.h                      //
// ************************************************************************* //

#ifndef AVT_IVPADAMSBASHFORTH_H
#define AVT_IVPADAMSBASHFORTH_H

#include <avtIVPSolver.h>
#include <ivp_exports.h>

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
// ****************************************************************************

class IVP_API avtIVPAdamsBashforth: public avtIVPSolver
{
  public:
    avtIVPAdamsBashforth();
    ~avtIVPAdamsBashforth();

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
    virtual void     SetMaximumStepSize( const double& hMax );

    virtual void     SetTolerances(const double& reltol, const double& abstol);
    virtual avtIVPAdamsBashforth* Clone() const
    {
        return new avtIVPAdamsBashforth( *this );
    }

  protected:
    // state serialization
    virtual void     AcceptStateVisitor(avtIVPStateHelper &aiss);
    
    int              AdamsMoulton4Steps(const avtIVPField* field,
                                        avtVec x,
                                        double t,
                                        double h,
                                        double tolerance,
                                        int iterations);
    avtVec           AdamsBashforth5Steps(avtVec &y,
                                          double h );
    
    int              Adams5Steps(const avtIVPField* field,
                                 avtVec x0,
                                 double t,
                                 double h,
                                 avtVec *y_bashforth,
                                 double tolerance,
                                 int iterations );
    bool              HasConverged( avtVec &y0,
                                    avtVec &y1,
                                    double epsilon );

  private:
    void Initialize(const avtIVPField* field);
    
    double tol;
    double h, h_max;
    double t;
    avtVec fns[4];
    avtVec x;
    avtVec ys[2];
    int initialized;
};

#endif
