/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                       avtStateRecorderIntegralCurve.h                     //
// ************************************************************************* //

#ifndef AVT_STATE_RECORDER_INTEGRAL_CURVE_H
#define AVT_STATE_RECORDER_INTEGRAL_CURVE_H

#include <avtIntegralCurve.h>


// ****************************************************************************
//  Class: avtStateRecorderIntegralCurve
//
//  Purpose:
//      A derived type of avtIntegralCurve.  This class records the state for
//      every step of the integral curve.  As the steps may occur on separate
//      processors, it also contains code to collect the steps and reconstitute 
//      them on the originating processor.
//
//  Programmer: Hank Childs (refactoring of work from others)
//  Creation:   June 4, 2010
//
//  Modifications:
//
//    Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//    Place sequence tracking code from base class into this class.
//
// ****************************************************************************

class IVP_API avtStateRecorderIntegralCurve : public avtIntegralCurve
{
  public:
    enum ScalarValueType {NONE=0, SPEED=1, VORTICITY=2, SCALAR_VARIABLE=4};

    typedef std::vector<avtIVPStep*>::const_iterator iterator;
    avtStateRecorderIntegralCurve(const avtIVPSolver* model, 
                                  const double& t_start, 
                                  const avtVector &p_start, int ID);
    avtStateRecorderIntegralCurve();
    virtual ~avtStateRecorderIntegralCurve();

    void      SetScalarValueType(ScalarValueType t) {scalarValueType = t;}
    void      SetIntersectionObject(vtkObject *obj);
    
    // Integration steps.
    size_t    size()  const;
    iterator  begin() const;
    iterator  end()   const;

    virtual void      Serialize(MemStream::Mode mode, MemStream &buff, 
                                avtIVPSolver *solver);
    virtual void      PrepareForSend(void)
                           { serializeFlags |= SERIALIZE_INC_SEQ; };
    virtual bool      SameCurve(avtIntegralCurve *ic);

    int       GetVariableIdx(const std::string &var) const;

    static avtIntegralCurve*
                      MergeIntegralCurveSequence(
                              std::vector<avtIntegralCurve *> &v);
    static bool IdSeqCompare(const avtIntegralCurve *slA,
                             const avtIntegralCurve *slB);
    static bool IdRevSeqCompare(const avtIntegralCurve *slA,
                                const avtIntegralCurve *slB);

  protected:
    avtStateRecorderIntegralCurve( const avtStateRecorderIntegralCurve& );
    avtStateRecorderIntegralCurve& operator=( const avtStateRecorderIntegralCurve& );
    
    void      HandleIntersections(avtIVPStep *step,
                                  avtIVPSolver::TerminateType termType,
                                  double end,
                                  avtIVPSolver::Result *result);
    bool      IntersectPlane(const avtVector &p0, const avtVector &p1);

  public:
    // Integration steps.
    std::vector<avtIVPStep*> _steps;

    unsigned long serializeFlags;
    long sequenceCnt;

  protected:
    virtual void AnalyzeStep(avtIVPStep *step,
                         const avtIVPField* field,
                         avtIVPSolver::TerminateType termType,
                         double end, avtIVPSolver::Result *result);

    // Intersection points.
    bool intersectionsSet;
    int numIntersections;
    double     intersectPlaneEq[4];

    ScalarValueType    scalarValueType;

  public:
    std::vector<std::string> scalars;
};


#endif 


