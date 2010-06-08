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
//                      avtStateRecorderIntegralCurve.C                      //
// ************************************************************************* //

#include <avtStateRecorderIntegralCurve.h>

#include <list>
#include <iostream>
#include <limits>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <vtkPlane.h>
#include <avtVector.h>
#include <algorithm>


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve constructor
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
// ****************************************************************************

avtStateRecorderIntegralCurve::avtStateRecorderIntegralCurve(const avtIVPSolver* model, const double& t_start,
                             const avtVector &p_start, int ID) :
    avtIntegralCurve(model, t_start, p_start, ID), scalarValueType(NONE)
{
    intersectionsSet = false;
    numIntersections = 0;

    sequenceCnt = 0;
    serializeFlags = 0;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve constructor
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
// ****************************************************************************

avtStateRecorderIntegralCurve::avtStateRecorderIntegralCurve() :
    scalarValueType(NONE)
{
    intersectionsSet = false;
    numIntersections = 0;
    sequenceCnt = 0;
    serializeFlags = 0;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve destructor
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
// ****************************************************************************

avtStateRecorderIntegralCurve::~avtStateRecorderIntegralCurve()
{
    for(iterator si = begin(); si != end(); si++)
         delete *si;
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::GetVariableIdx
//
//  Purpose:
//      Lookup the index of a variable.
//
//  Programmer: Dave Pugmire
//  Creation:   December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

int
avtStateRecorderIntegralCurve::GetVariableIdx(const std::string &var) const
{
    for (int i = 0; i < scalars.size(); i++)
        if (scalars[i] == var)
            return i;

    return -1;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::AnalyzeStep
//
//  Purpose:
//      Analyzes the current step.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
// ****************************************************************************

void
avtStateRecorderIntegralCurve::AnalyzeStep(avtIVPStep *step,
                         const avtIVPField* field,
                         avtIVPSolver::TerminateType termType,
                         double end, avtIVPSolver::Result *result)
{
    if (intersectionsSet)
        HandleIntersections(step, termType, end, result);

    // record step if it was successful
    if (*result == avtIVPSolver::OK ||
        *result == avtIVPSolver::TERMINATE)
    {
        //Set scalar value, if any...
        if (scalarValueType & VORTICITY)
            step->ComputeVorticity(field);
        if (scalarValueType & SPEED)
            step->ComputeSpeed(field);
        if (!scalars.empty())
            step->ComputeScalarVariables(scalars, field);

        avtIVPStep *s2 = new avtIVPStep(*step);
        _steps.push_back(s2);
    }
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::begin
//
//  Purpose:
//      Returns the first iterator.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtStateRecorderIntegralCurve::iterator
avtStateRecorderIntegralCurve::begin() const
{
    return _steps.begin();
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::end
//
//  Purpose:
//      Returns the last iterator.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

avtStateRecorderIntegralCurve::iterator
avtStateRecorderIntegralCurve::end() const
{
    return _steps.end();
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::size
//
//  Purpose:
//      Returns the number of iterations to do.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

size_t
avtStateRecorderIntegralCurve::size() const
{
    return _steps.size();
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::SetIntersectionObject
//
//  Purpose:
//      Defines an object for streamline intersection.
//
//  Programmer: Dave Pugmire
//  Creation:   August 10, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Store plane equation.
//
// ****************************************************************************

void
avtStateRecorderIntegralCurve::SetIntersectionObject(vtkObject *obj)
{
    // Only plane supported for now.
    if (!obj->IsA("vtkPlane"))
        EXCEPTION1(ImproperUseException, "Only plane supported.");

    intersectionsSet = true;
    avtVector intersectPlanePt = avtVector(((vtkPlane *)obj)->GetOrigin());
    avtVector intersectPlaneNorm = avtVector(((vtkPlane *)obj)->GetNormal());

    intersectPlaneNorm.normalize();
    intersectPlaneEq[0] = intersectPlaneNorm.x;
    intersectPlaneEq[1] = intersectPlaneNorm.y;
    intersectPlaneEq[2] = intersectPlaneNorm.z;
    intersectPlaneEq[3] = intersectPlanePt.length();
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::HandleIntersections
//
//  Purpose:
//      Defines an object for streamline intersection.
//
//  Programmer: Dave Pugmire
//  Creation:   August 10, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Don't record intersection points, just count them.
//
//   Dave Pugmire, Fri Feb 19 16:57:04 EST 2010
//   Replace _steps.size()==0 with _steps.empty()
//
// ****************************************************************************

void
avtStateRecorderIntegralCurve::HandleIntersections(avtIVPStep *step,
                                   avtIVPSolver::TerminateType termType,
                                   double end,
                                   avtIVPSolver::Result *result)
{
    if (step == NULL || _steps.empty())
        return;
    
    avtIVPStep *step0 = _steps.back();

    if (IntersectPlane(step0->front(), step->front()))
    {
        numIntersections++;
        if (termType == avtIVPSolver::INTERSECTIONS &&
            numIntersections >= (int)end)
        {
            *result = avtIVPSolver::TERMINATE;
        }
    }
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::IntersectPlane
//
//  Purpose:
//      Intersect streamline with a plane.
//
//  Programmer: Dave Pugmire
//  Creation:   August 10, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Aug 18 08:47:40 EDT 2009
//   Don't record intersection points, just count them.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

bool
avtStateRecorderIntegralCurve::IntersectPlane(const avtVector &p0, const avtVector &p1)
{
    double distP0 = intersectPlaneEq[0] * p0.x +
                    intersectPlaneEq[1] * p0.y +
                    intersectPlaneEq[2] * p0.z +
                    intersectPlaneEq[3];

    double distP1 = intersectPlaneEq[0] * p1.x +
                    intersectPlaneEq[1] * p1.y +
                    intersectPlaneEq[2] * p1.z +
                    intersectPlaneEq[3];

#define SIGN(x) ((x) < 0.0 ? -1 : 1)

    // If either point on the plane, or points on opposite
    // sides of the plane, the line intersects.
    if (distP0 == 0.0 || distP1 == 0.0 ||
        SIGN(distP0) != SIGN(distP1))
    {
        return true;
    }

    return false;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::Serialize
//
//  Purpose:
//      Serializes a streamline so it can be sent to another processor.
//
//  Programmer: Hank Childs
//  Creation:   June 4, 2010
//
//  Modifications:
//
//    Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//    Add portions for sequence tracking, which were previously in the base
//    class.
//
// ****************************************************************************

void
avtStateRecorderIntegralCurve::Serialize(MemStream::Mode mode, MemStream &buff, 
                         avtIVPSolver *solver)
{
    // Have the base class serialize its part
    avtIntegralCurve::Serialize(mode, buff, solver);

    bool serializeSteps = serializeFlags&SERIALIZE_STEPS;
    if (DebugStream::Level5())
        debug5<<"  avtStateRecorderIntegralCurve::Serialize "<<(mode==MemStream::READ?"READ":"WRITE")<<" serSteps= "<<serializeSteps<<endl;
    
    buff.io(mode, scalarValueType);
    buff.io(mode, numIntersections);

    // R/W the steps.
    if (mode == MemStream::WRITE)
    {
        size_t sz = _steps.size();
        if (serializeSteps)
        {
            buff.io(mode, sz);
            for (iterator si = _steps.begin(); si != _steps.end(); si++)
                (*si)->Serialize(mode, buff);
        }
        else
        {
            sz = 0;
            buff.io(mode, sz);
        }
    }
    else
    {
        _steps.clear();
        size_t sz = 0;
        buff.io( mode, sz );
        debug5<<"Read step cnt= "<<sz<<endl;
        for ( size_t i = 0; i < sz; i++ )
        {
            avtIVPStep *s = new avtIVPStep;
            s->Serialize( mode, buff );
            _steps.push_back( s );
        }
    }

    if ((serializeFlags & SERIALIZE_INC_SEQ) && mode == MemStream::WRITE)
    {
        long seqCnt = sequenceCnt+1;
        buff.io(mode, seqCnt);
    }
    else
        buff.io(mode, sequenceCnt);

    serializeFlags = 0;
    if (DebugStream::Level5())
        debug5 << "DONE: avtStateRecorderIntegralCurve::Serialize. sz= "<<buff.buffLen() << endl;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::MergeIntegralCurveSequence
//
//  Purpose:
//      Merge a vector of streamline sequences into a single streamline.
//      This is destructive, extra streamlines are deleted.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Sorting can be done independant of streamline direction. Changed streamline
//   step from list to vector.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Move this method from avtStreamlineWrapper.
//
// ****************************************************************************

avtIntegralCurve *
avtStateRecorderIntegralCurve::MergeIntegralCurveSequence(std::vector<avtIntegralCurve *> &v2)
{
    std::vector<avtStateRecorderIntegralCurve *> v;
    for (int j = 0 ; j < v2.size() ; j++)
        v.push_back((avtStateRecorderIntegralCurve *) v2[j]);

    if (v.size() == 0)
        return NULL;
    else if (v.size() == 1)
        return v[0];

    //Sort the streamlines by Id,seq.
    std::sort(v.begin(), v.end(), 
              avtStateRecorderIntegralCurve::IdRevSeqCompare);

    //Make sure all ids are the same.
    if (v.front()->id != v.back()->id)
        return NULL;

    //We want to merge others into the "last" sequence, since it has the right
    //sover state, sequenceCnt, etc. The vector is reverse sorted, so we can
    //merge them in order.

    avtStateRecorderIntegralCurve *s = v.front();

    for (int i = 1; i < v.size(); i++)
    {
        std::vector<avtIVPStep*>::reverse_iterator si;
        for (si = v[i]->_steps.rbegin(); si != v[i]->_steps.rend(); si++)
        {
            avtIVPStep *step = new avtIVPStep(*(*si));
            s->_steps.insert(s->_steps.begin(), step);
        }

        delete v[i];
    }


    v.resize(0);
    return s;
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::IdSeqCompare
//
//  Purpose:
//      Sort streamlines by id, then sequence number.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//    Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//    Move this method from avtStreamlineWrapper.
//
// ****************************************************************************
bool
avtStateRecorderIntegralCurve::IdSeqCompare(const avtIntegralCurve *icA,
                                            const avtIntegralCurve *icB)
{
    avtStateRecorderIntegralCurve *sicA = (avtStateRecorderIntegralCurve *) icA;
    avtStateRecorderIntegralCurve *sicB = (avtStateRecorderIntegralCurve *) icB;

    if (sicA->id == sicB->id)
        return sicA->sequenceCnt < sicB->sequenceCnt;

    return sicA->id < sicB->id;
}

// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::IdRevSeqCompare
//
//  Purpose:
//      Sort streamlines by id, then reverse sequence number.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
//  Modifications:
//
//    Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//    Move this method from avtStreamlineWrapper.
//
// ****************************************************************************
bool
avtStateRecorderIntegralCurve::IdRevSeqCompare(const avtIntegralCurve *icA,
                                               const avtIntegralCurve *icB)
{
    avtStateRecorderIntegralCurve *sicA = (avtStateRecorderIntegralCurve *) icA;
    avtStateRecorderIntegralCurve *sicB = (avtStateRecorderIntegralCurve *) icB;

    if (sicA->id == sicB->id)
        return sicA->sequenceCnt > sicB->sequenceCnt;

    return sicA->id < sicB->id;
}


// ****************************************************************************
//  Method: avtStateRecorderIntegralCurve::SameCurve
//
//  Purpose:
//      Checks to see if two curves are the same.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2010
//
// ****************************************************************************

bool
avtStateRecorderIntegralCurve::SameCurve(avtIntegralCurve *ic)
{
    avtStateRecorderIntegralCurve *sic = (avtStateRecorderIntegralCurve *) ic;
    return (id == sic->id) && (sequenceCnt == sic->sequenceCnt);
}


