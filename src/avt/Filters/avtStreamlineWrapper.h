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
//                              avtStreamlineWrapper.h                       //
// ************************************************************************* //

#ifndef AVT_STREAMLINE_WRAPPER_H
#define AVT_STREAMLINE_WRAPPER_H

#include <vector>
#include <visitstream.h>
#include <MemStream.h>
#include <filters_exports.h>
#include <avtIVPSolver.h>

class avtStreamline;
class vtkPolyData;
class avtVector;
class avtIVPSolver;

// ****************************************************************************
// Class: DomainType
//
// Purpose:
//    Encapsulate the a domain/timestep.
//    
//
// Programmer: Dave Pugmire
// Creation:   Tue Mar 10 12:41:11 EDT 2009
//
// Modifications:
//
//   Dave Pugmire, Mon May 11 12:41:51 EDT 2009
//   Fix operator< so that that std::map works.
//
// ****************************************************************************

class AVTFILTERS_API DomainType
{
  public:
    DomainType() :domain(-1), timeStep(0) {}
    DomainType(const int &d) :domain(d), timeStep(0) {}
    DomainType(const int &d, const int &t) :domain(d), timeStep(t) {}
    ~DomainType() {}

    void operator=(const DomainType &dt)
    {
        domain=dt.domain;
        timeStep=dt.timeStep;
    }
    
    bool operator==(const DomainType &dt) const
    {
        return (domain == dt.domain &&
                timeStep == dt.timeStep);
    }
    bool operator<(const DomainType &dt) const
    {
        return (domain < dt.domain) ||
               (!(domain < dt.domain) && timeStep < dt.timeStep);
    }

    //Members
    int domain, timeStep;

    friend ostream& operator<<(ostream &out, const DomainType &d);
};

// ****************************************************************************
// Class: avtStreamlineWrapper
//
// Purpose:
//    A helper class that encapsulates a streamline and other related
//    information.
//
// Programmer: Dave Pugmire
// Creation:   Mon Jan 26 13:25:58 EST 2009
//
// Modifications:
//
// Dave Pugmire, Mon Feb  2 14:41:25 EST 2009
// Moved GetVTKPolyData to avtStreamlinePolyDataFilter.
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Generalized domain to include domain/time.
//
//   Dave Pugmire, Tue Aug 18 09:10:49 EDT 2009
//   Add ability to restart integration of streamlines.
//
// ****************************************************************************

class AVTFILTERS_API avtStreamlineWrapper
{
  public:
    enum Status
    {
        UNSET=-1,
        TERMINATE,
        OUTOFBOUNDS
    };

    enum Dir
    {
        FWD=0,
        BWD
    };

    avtStreamlineWrapper();
    avtStreamlineWrapper(avtStreamline *s, Dir slDir=FWD, int ID=-1);
    ~avtStreamlineWrapper();

    void UpdateDomainCount(DomainType &dom);
    void ComputeStatistics();
    void GetStartPoint(avtVector &pt, double &t) const;
    void GetEndPoint(avtVector &pt, double &t) const;
    void GetStartPoint(avtVector &pt) const {double t; GetStartPoint(pt,t); }
    void GetEndPoint(avtVector &pt) const {double t; GetEndPoint(pt,t); }

    void Debug();
    void Serialize(MemStream::Mode mode, MemStream &buff, avtIVPSolver *solver);

    double termination;
    avtIVPSolver::TerminateType terminationType;
    avtStreamline *sl;

    // Helpers needed for computing streamlines
    std::vector<DomainType> seedPtDomainList;
    DomainType domain;
    Status status;
    Dir dir;
    
    // statistical bookeeping.
    std::vector<int> domainVisitCnts;
    int maxCnt, sum, numDomainsVisited;
    int numTimesCommunicated;
    int id;
    long long sortKey;
};

#endif
