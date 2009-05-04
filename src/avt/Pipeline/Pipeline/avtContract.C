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
//                               avtContract.C                               //
// ************************************************************************* //

#include <avtContract.h>

#include <avtWebpage.h>


static const char* bool2str(bool b) { return b ? "yes" : "no"; }

// ****************************************************************************
//  Function: operator<<
//
//  Arguments:
//      os       output stream to write to
//      c        contract information to query.
//
//  Programmer:  Tom Fogal
//  Creation:    May 3, 2009
//
//  Modifications:
//
// ****************************************************************************
ostream& operator<<(ostream &os, const avtContract& c)
{
    os << "avtContract information:"
       << "\tpipeline index: " << c.pipelineIndex << "\n"
       << "\tstreaming possible: " << bool2str(c.canDoStreaming) << "\n"
       << "\tstreaming: " << bool2str(c.doingOnDemandStreaming) << "\n"
       << "\tload balancing: " << bool2str(c.useLoadBalancing) << "\n"
       << "\tmesh optimizations:" << "\n"
       << "\t\tcurvilinear: " << bool2str(c.haveCurvilinearMeshOptimizations)
       << "\n\t\trectilinear: " << bool2str(c.haveRectilinearMeshOptimizations)
       << "\n\tfilters: " << c.nFilters << std::endl;
    return os;
}

// ****************************************************************************
//  Method: avtContract constructor
//
//  Arguments:
//      d        The data specification for this pipeline.
//      pi       The index of the pipeline.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 20 17:18:19 PDT 2001
//    Initialize nFilters.
//
//    Hank Childs, Thu Mar  3 16:36:50 PST 2005
//    Don't share a reference to the data specification, because we may modify
//    it.
//
//    Hank Childs, Sun Mar 13 09:49:16 PST 2005
//    Initialize haveStructuredMeshOptimizations.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Sun Mar  9 08:02:29 PST 2008
//    Initialize doingOnDemandStreaming.
//
// ****************************************************************************

avtContract::avtContract(avtDataRequest_p d, int pi)
{
    data             = new avtDataRequest(d);
    pipelineIndex    = pi;
    canDoStreaming   = true;
    useLoadBalancing = true;
    nFilters         = 0;
    haveCurvilinearMeshOptimizations = false;
    haveRectilinearMeshOptimizations = false;
    doingOnDemandStreaming           = false;
}


// ****************************************************************************
//  Method: avtContract copy constructor
//
//  Arguments:
//      ps       A pipeline specification to copy.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2001
//
// ****************************************************************************

avtContract::avtContract(
                                                 avtContract_p ps)
{
    *this = **ps;
}


// ****************************************************************************
//  Method: avtContract copy constructor
//
//  Arguments:
//      ps       A pipeline specification to copy.
//
//  Programmer:  Hank Childs
//  Creation:    June 5, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Mar  3 16:36:50 PST 2005
//    Don't share a reference to the data specification, because we may modify
//    it.
//
// ****************************************************************************

avtContract::avtContract(avtContract_p ps, avtDataRequest_p ds)
{
    *this = **ps;
    data  = new avtDataRequest(ds);
}


// ****************************************************************************
//  Method: avtContract destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtContract::~avtContract()
{
    ;
}


// ****************************************************************************
//  Method: avtContract assignment operator
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 20 17:30:28 PDT 2001
//    Add nFilters.
//
//    Hank Childs, Thu Mar  3 16:36:50 PST 2005
//    Don't share a reference to the data specification, because we may modify
//    it.
//
//    Hank Childs, Sun Mar 13 09:49:16 PST 2005
//    Add haveStructuredMeshOptimizations.
//
//    Hank Childs, Tue Dec 18 14:11:56 PST 2007
//    Added const qualified argument.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Sun Mar  9 08:02:29 PST 2008
//    Added doingOnDemandStreaming.
//
// ****************************************************************************

avtContract &
avtContract::operator=(const avtContract &ps)
{
    data             = new avtDataRequest(ps.data);
    pipelineIndex    = ps.pipelineIndex;
    canDoStreaming   = ps.canDoStreaming;
    useLoadBalancing = ps.useLoadBalancing;
    nFilters         = ps.nFilters;
    haveCurvilinearMeshOptimizations = ps.haveCurvilinearMeshOptimizations;
    haveRectilinearMeshOptimizations = ps.haveRectilinearMeshOptimizations;
    doingOnDemandStreaming = ps.doingOnDemandStreaming;

    return *this;
}


// ****************************************************************************
//  Method: avtContract::UseLoadBalancing
//
//  Purpose:
//      Allows load balancing to be turned off for situations like when you
//      know you are in serial (on the viewer), or if you are in the Execute
//      portion of a filter and want to set up a pipeline inside of it and
//      don't want any further load balancing to take place.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2001
//
// ****************************************************************************

void
avtContract::UseLoadBalancing(bool newVal)
{
    useLoadBalancing = newVal;
}


// ****************************************************************************
//  Method: avtContract::DebugDump
//
//  Purpose:
//      Dumps data members to a webpage.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2007
//
//  Modifications:
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

static const char *
YesOrNo(bool b)
{
    static const char *yes_str = "yes";
    static const char *no_str  = "no";
    if (b)
        return yes_str;

    return no_str;
}


void
avtContract::DebugDump(avtWebpage *webpage)
{
    char str[1024];

    webpage->AddSubheading("Pipeline attributes");
    webpage->StartTable();
    webpage->AddTableHeader2("Field", "Value");
    sprintf(str, "%d", pipelineIndex);
    webpage->AddTableEntry2("Pipeline index", str);
    webpage->AddTableEntry2("Can do streaming", 
                            YesOrNo(canDoStreaming));
    webpage->AddTableEntry2("Should use load balancing", 
                            YesOrNo(useLoadBalancing));
    webpage->AddTableEntry2("Have curvilinear optimizations", 
                            YesOrNo(haveCurvilinearMeshOptimizations));
    webpage->AddTableEntry2("Have rectilinear optimizations", 
                            YesOrNo(haveRectilinearMeshOptimizations));
    webpage->AddTableEntry2("Doing on demand streaming", 
                            YesOrNo(doingOnDemandStreaming));
    sprintf(str, "%d", nFilters);
    webpage->AddTableEntry2("Number of known filters", str);
    webpage->EndTable();

    data->DebugDump(webpage);
}


