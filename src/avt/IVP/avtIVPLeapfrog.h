/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

    virtual double    GetCurrentT() const;
    virtual avtVector GetCurrentY() const;
    virtual avtVector GetCurrentV() const;

    virtual void     SetCurrentT( double newT );
    virtual void     SetCurrentY( const avtVector &newY );
    virtual void     SetCurrentV( const avtVector &newV );

    virtual void     SetNextStepSize( const double& h );
    virtual double   GetNextStepSize() const;
    virtual void     SetMaximumStepSize( const double& hMax );

    virtual void     SetTolerances(const double& reltol, const double& abstol);
    virtual avtIVPLeapfrog* Clone() const
    {
        return new avtIVPLeapfrog( *this );
    }

  protected:
    // state serialization
    virtual void     AcceptStateVisitor(avtIVPStateHelper &aiss);
    
    void             UpdateHistory( const avtVector &yNew );

  private:
    int numStep;
    double tol;
    double h, h_max;
    double t, d;
    avtVector yCur;
    avtVector vCur;
};

#endif
