/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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
//                              avtStreamlineWrapper.C                       //
// ************************************************************************* //

#include "avtStreamlineWrapper.h"
#include <avtStreamline.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <DebugStream.h>
#include <avtVector.h>
#include <algorithm>

using namespace std;


ostream& operator<<(ostream &out, const DomainType &d)
{
    out<<"["<<d.domain<<", "<<d.timeStep<<"]";
    return out;
}

// ****************************************************************************
//  Method: avtStreamlineWrapper constructor
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Wed Aug 20 10:37:24 EST 2008
//   Initialize some previously unitialized member data.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add a seed point id attribute to each streamline.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Add sequenceCnt.
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Removed avtStreamlineWrapper:Dir
//
// ****************************************************************************

avtStreamlineWrapper::avtStreamlineWrapper()
{
    status = UNSET;
    domain = -1;
    sequenceCnt = 0;
    terminated = false;
    sl = NULL;
    maxCnt = sum= numDomainsVisited = 0;
    id = -1;
    sortKey = 0;
    
    termination = 0.0;
    terminationType = avtIVPSolver::TIME;
    serializeFlags = 0;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper constructor
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Wed Aug 20 10:37:24 EST 2008
//   Initialize some previously unitialized member data.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add a seed point id attribute to each streamline.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Add sequenceCnt.
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Removed avtStreamlineWrapper:Dir
//
// ****************************************************************************

avtStreamlineWrapper::avtStreamlineWrapper(avtStreamline *s, int ID)
{
    sl = s;
    status = UNSET;
    domain = -1;
    sequenceCnt = 0;
    maxCnt = sum= numDomainsVisited = 0;
    id = ID;
    terminated = false;
    termination = 0.0;
    terminationType = avtIVPSolver::TIME;
    serializeFlags = 0;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper destructor
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

avtStreamlineWrapper::~avtStreamlineWrapper()
{
    if (sl)
        delete sl;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::Debug
//
//  Purpose:
//      Outputs debug information.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Print out how many steps have been taken.
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    No longer have fwd/bwd solvers.
//
// ****************************************************************************

void
avtStreamlineWrapper::Debug()
{
    //debug1<<"slWrapper: id= "<<id;
    //debug1<<" #steps= "<<(sl?sl->size():0)<<endl;

    /*
    avtVec end;
    sl->PtEnd(end);
    debug1<<"******seed: "<<end;
    debug1<<" Dom= [ "<<domain<<", ";
    for (size_t i = 0; i < seedPtDomainList.size(); i++)
        debug1<<seedPtDomainList[i]<<", ";
    debug1<<"] ";
    debug1<< " steps= "<<sl->size()<<endl;
    */
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::Serialize
//
//  Purpose:
//      Serializes into a byte stream.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add a seed point id attribute to each streamline.
//
//   Dave Pugmire, Thu Sep 24 13:52:59 EDT 2009
//   Add serialization flags, sequenceCnt.
//
//   Dave Pugmire, Tue Feb 23 09:42:25 EST 2010
//   Removed avtStreamlineWrapper:Dir
//
// ****************************************************************************

void
avtStreamlineWrapper::Serialize(MemStream::Mode mode,
                                MemStream &buff, 
                                avtIVPSolver *solver)
{
    debug5 << "avtStreamlineWrapper::Serialize: id= "<<id<<endl;
    Debug();

    buff.io(mode, id);
    buff.io(mode, domain);
    buff.io(mode, status);
    buff.io(mode, terminated);
    buff.io(mode, termination);
    buff.io(mode, terminationType);
    
    if ((serializeFlags & SERIALIZE_INC_SEQ) && mode == MemStream::WRITE)
    {
        long seqCnt = sequenceCnt+1;
        buff.io(mode, seqCnt);
    }
    else
        buff.io(mode, sequenceCnt);


    //buff.io(mode, domainVisitCnts);
    if (mode == MemStream::READ)
    {
        if (sl)
            delete sl;
        sl = new avtStreamline;
    }

    sl->Serialize(mode, buff, solver, (serializeFlags & SERIALIZE_STEPS));

    serializeFlags = 0;
    debug5 << "DONE: avtStreamlineWrapper::Serialize: id= "<<id<<" sz= "<< buff.buffLen()
           << endl;
}

// ****************************************************************************
//  Method: avtStreamlineWrapper::GetStartPoint
//
//  Purpose:
//      Gets the starting point.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

void
avtStreamlineWrapper::GetStartPoint(avtVector &pt, double &t) const
{
    pt = sl->PtStart();
    t = sl->TMin();

    //debug5<<"avtStreamlineWrapper::GetStartPoint() = "<<pt<<" T= "<<t<<endl;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::GetEndPoint
//
//  Purpose:
//      Gets the ending point.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//    Dave Pugmire, Mon Feb 23, 09:11:34 EST 2009
//    No longer have fwd/bwd solvers.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
// ****************************************************************************

void
avtStreamlineWrapper::GetEndPoint(avtVector &pt, double &t) const
{
    sl->PtEnd(pt);
    t = sl->TMax();

    //debug5<<"avtStreamlineWrapper::GetEndPoint() = "<<pt<<" T= "<<t<<endl;
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::UpdateDomainCount
//
//  Purpose:
//      Updates the domain count.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifictaions:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time. Pathine cleanup.
//
// ****************************************************************************

void
avtStreamlineWrapper::UpdateDomainCount(DomainType &dom)
{
    /*
    if (dom.domain+1 > domainVisitCnts.size())
        domainVisitCnts.resize(dom.domain+1, 0);

    domainVisitCnts[dom.domain]++;
    */
}


// ****************************************************************************
//  Method: avtStreamlineWrapper::ComputeStatistics
//
//  Purpose:
//      Computes statistics on the number of domains visited.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
// ****************************************************************************

void
avtStreamlineWrapper::ComputeStatistics()
{
    maxCnt = 0;
    sum = 0;
    numDomainsVisited = 0;
    /*
    for (size_t i = 0; i < domainVisitCnts.size(); i++)
    {
        int cnt = domainVisitCnts[i];
        if (cnt > maxCnt)
            maxCnt = cnt;
        if (cnt > 0)
            numDomainsVisited++;
        sum += cnt;
    }
    */
}

// ****************************************************************************
//  Method: avtStreamlineWrapper::IdSeqCompare
//
//  Purpose:
//      Sort streamlines by id, then sequence number.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
// ****************************************************************************
bool
avtStreamlineWrapper::IdSeqCompare(const avtStreamlineWrapper *slA, 
                                   const avtStreamlineWrapper *slB)
{
    if (slA->id == slB->id)
        return slA->sequenceCnt < slB->sequenceCnt;
    
    return slA->id < slB->id;
}

// ****************************************************************************
//  Method: avtStreamlineWrapper::IdRevSeqCompare
//
//  Purpose:
//      Sort streamlines by id, then reverse sequence number.
//
//  Programmer: Dave Pugmire
//  Creation:   September 24, 2009
//
// ****************************************************************************
bool
avtStreamlineWrapper::IdRevSeqCompare(const avtStreamlineWrapper *slA, 
                                   const avtStreamlineWrapper *slB)
{
    if (slA->id == slB->id)
        return slA->sequenceCnt > slB->sequenceCnt;
    
    return slA->id < slB->id;
}

bool
avtStreamlineWrapper::DomainCompare(const avtStreamlineWrapper *slA,
                                    const avtStreamlineWrapper *slB)
{
    return slA->domain < slB->domain;
}

// ****************************************************************************
//  Method: avtStreamlineWrapper::MergeStreamlineSequence
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
// ****************************************************************************

avtStreamlineWrapper*
avtStreamlineWrapper::MergeStreamlineSequence(std::vector<avtStreamlineWrapper *> &v)
{
    if (v.size() == 0)
        return NULL;
    else if (v.size() == 1)
        return v[0];
    
    //Sort the streamlines by Id,seq.
    std::sort(v.begin(), v.end(), avtStreamlineWrapper::IdRevSeqCompare);
    
    //Make sure all ids are the same.
    if (v.front()->id != v.back()->id)
        return NULL;

    //We want to merge others into the "last" sequence, since it has the right
    //sover state, sequenceCnt, etc. The vector is reverse sorted, so we can
    //merge them in order.

    avtStreamlineWrapper *s = v.front();

    for (int i = 1; i < v.size(); i++)
    {
        std::vector<avtIVPStep*>::reverse_iterator si;
        for (si = v[i]->sl->_steps.rbegin(); si != v[i]->sl->_steps.rend(); si++)
        {
            avtIVPStep *step = new avtIVPStep(*(*si));
            s->sl->_steps.insert(s->sl->_steps.begin(), step);
        }
        
        delete v[i];
    }

    v.resize(0);
    return s;
}
