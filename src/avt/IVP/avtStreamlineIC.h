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
//                            avtStreamlineIC.h                              //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_IC_H
#define AVT_STREAMLINE_IC_H

#include <avtStateRecorderIntegralCurve.h>

// ****************************************************************************
//  Class: avtStreamlineIC
//
//  Purpose:
//      A derived type of avtStateRecorderIntegralCurve.  This class 
//      decides how to terminate a streamline.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2010
//
//  Modifications:
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Add avtIVPField argument to CheckForTermination.  Also add data members
//    to help determine if a warning should be issued.
//
// ****************************************************************************

class IVP_API avtStreamlineIC : public avtStateRecorderIntegralCurve
{
public:

    avtStreamlineIC(int maxSteps, bool doDistance, double maxDistance,
                    bool doTime, double maxTime,
                    unsigned char mask, const avtIVPSolver* model, 
                    Direction dir, const double& t_start, 
                    const avtVector &p_start, const avtVector &v_start,
                    int ID);

    avtStreamlineIC();
    virtual ~avtStreamlineIC();

    virtual void  Serialize(MemStream::Mode mode, MemStream &buff, 
                                avtIVPSolver *solver);
    virtual bool    UseFixedTerminationTime(void) { return doTime; };
    virtual double  FixedTerminationTime(void)    { return maxTime; };

    bool            TerminatedBecauseOfMaxSteps(void) 
                                 { return terminatedBecauseOfMaxSteps; };
    double          SpeedAtTermination(void) 
                                 { return speedAtTermination; };

  protected:
    avtStreamlineIC( const avtStreamlineIC& );
    avtStreamlineIC& operator=( const avtStreamlineIC& );
    
  public:
    virtual bool     CheckForTermination(avtIVPStep& step, avtIVPField *);

    unsigned int     numSteps;
    unsigned int     maxSteps;
    bool             doDistance;
    double           maxDistance;
    bool             doTime;
    double           maxTime;
    bool             terminatedBecauseOfMaxSteps;
    double           speedAtTermination;
};

#endif 
