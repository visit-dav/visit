/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                            avtLCSIC.h                                    //
// ************************************************************************* //

#ifndef AVT_LCS_IC_H
#define AVT_LCS_IC_H

#include <avtIntegralCurve.h>

// ****************************************************************************
//  Class: avtLCSIC
//
//  Purpose:
//      A derived type of avtIntegralCurve. This class 
//      decides how to terminate a LCS.
//
//  Programmer: Allen Sanderson
//  Creation:   August 14, 2013
//
//  Modifications:
//
// ****************************************************************************

class IVP_API avtLCSIC : public avtIntegralCurve
{
public:

    avtLCSIC(int maxSteps, bool doDistance, double maxDistance,
              bool doTime, double maxTime,
              const avtIVPSolver* model, 
              Direction dir, const double& t_start, 
              const avtVector &p_start, const avtVector &v_start,
              int ID);

    avtLCSIC();
    virtual ~avtLCSIC();

    virtual void    Serialize(MemStream::Mode mode, MemStream &buff, 
                              avtIVPSolver *solver, SerializeFlags serializeFlags);
    virtual bool    UseFixedTerminationTime(void) { return doTime; };
    virtual double  FixedTerminationTime(void)    { return maxTime; };
    virtual bool    UseFixedTerminationDistance(void) { return doDistance; };
    virtual double  FixedTerminationDistance(void)    { return maxDistance; };

    bool            TerminatedBecauseOfMaxSteps(void) 
                                 { return terminatedBecauseOfMaxSteps; };

  protected:
    avtLCSIC( const avtLCSIC& );
    avtLCSIC& operator=( const avtLCSIC& );
    
  public:
    virtual bool   CheckForTermination(avtIVPStep& step, avtIVPField *);

    virtual int    GetNumSteps()  { return numSteps; }
    virtual double GetTime()      { return time; }
    virtual double GetArcLength() { return arcLength; }
    virtual double GetDistance()  { return distance; }
    virtual double GetSummation0() { return summation0; }
    virtual double GetSummation1() { return summation1; }

    virtual void     AnalyzeStep( avtIVPStep &step, avtIVPField *field);
  
    bool LessThan(const avtIntegralCurve *ic) const;

    avtIntegralCurve* MergeIntegralCurveSequence(std::vector<avtIntegralCurve *> &v);

    avtVector GetStartPoint() { return p_start; }
    avtVector GetEndPoint() { return p_end; }

  public:
    unsigned int     maxSteps;
  protected:
    unsigned int     numSteps;
    bool             doDistance;
    double           maxDistance;
    bool             doTime;
    double           maxTime;
    bool             terminatedBecauseOfMaxSteps;

    double           time;
    double           arcLength;
    double           distance;
    double           summation0;
    double           summation1;

    avtVector p_start, p_end;
};

#endif
