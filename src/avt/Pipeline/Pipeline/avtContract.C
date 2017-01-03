/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                               avtContract.C                               //
// ************************************************************************* //

#include <avtContract.h>

#include <string.h>

#include <avtWebpage.h>
#include <VisItStreamUtil.h>

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
//    Dave Pugmire, Tue May 25 10:15:35 EDT 2010
//    Add domain single domain replication to all processors.
//
//    Hank Childs, Thu Aug 26 11:08:02 PDT 2010
//    Print out extents calculation members
//
// ****************************************************************************
ostream& operator<<(ostream &os, const avtContract& c)
{
    os << "avtContract information:\n"
       << "\tpipeline index: " << c.pipelineIndex << "\n"
       << "\tstreaming possible: " << bool2str(c.canDoStreaming) << "\n"
       << "\tstreaming: " << bool2str(c.doingOnDemandStreaming) << "\n"
       << "\treplicateSingleDoms: " << bool2str(c.replicateSingleDomainOnAllProcessors) << "\n"
       << "\tload balancing: " << bool2str(c.useLoadBalancing) << "\n"
       << "\tcalculate mesh extents: " << bool2str(c.calculateMeshExtents) << "\n";
   if (c.needExtentsForTheseVariables.size() == 0)
   {
       os << "\tcalculate extents for these variables: <none>\n";
   }
   else
   {
       os << "\tcalculate extents for these variables:";
       for (size_t i = 0 ; i < c.needExtentsForTheseVariables.size() ; i++)
           os << c.needExtentsForTheseVariables[i] << "; ";
       os << "\n";
   }

   os  << "\tmesh optimizations:" << "\n"
       << "\t\tcurvilinear: " << bool2str(c.haveCurvilinearMeshOptimizations)
       << "\n\t\trectilinear: " << bool2str(c.haveRectilinearMeshOptimizations)
       << "\n\tfilters: " << c.nFilters
       << "\n\tline type: " << c.lineType
//       << "\n\tattributeMap: " << c.attributeMap
       << std::endl;

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
//    Dave Pugmire, Tue May 25 10:15:35 EDT 2010
//    Add domain single domain replication to all processors.
//
//    Hank Childs, Thu Aug 26 11:08:02 PDT 2010
//    Initialize extents information.
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
    replicateSingleDomainOnAllProcessors = false;
    calculateMeshExtents = true;
    lineType         = 0;
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

avtContract::avtContract(avtContract_p ps)
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
//    Dave Pugmire, Tue May 25 10:15:35 EDT 2010
//    Add domain single domain replication to all processors.
//
//    Hank Childs, Thu Aug 26 11:08:02 PDT 2010
//    Copy extents information.
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
    replicateSingleDomainOnAllProcessors = ps.replicateSingleDomainOnAllProcessors;
    calculateMeshExtents = ps.calculateMeshExtents;
    needExtentsForTheseVariables = ps.needExtentsForTheseVariables;

    lineType     = ps.lineType;
    attributeMap = ps.attributeMap;

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
//  Method: avtContract::ShouldCalculateVariableExtents
//
//  Purpose:
//      Determine if we should calculate the extents of a given variable.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2010
//
// ****************************************************************************

bool
avtContract::ShouldCalculateVariableExtents(const std::string &s)
{
    for (size_t i = 0 ; i < needExtentsForTheseVariables.size() ; i++)
        if (needExtentsForTheseVariables[i] == s)
            return true;
    return false;
}


// ****************************************************************************
//  Method: avtContract::SetCalculateVariableExtents
//
//  Purpose:
//      Declares whether we should calculate the extents of a given variable.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2010
//
// ****************************************************************************

void
avtContract::SetCalculateVariableExtents(const std::string &s, bool v)
{
    if (v)
    {
        bool alreadyHaveIt = false;
        for (size_t i = 0 ; i < needExtentsForTheseVariables.size() ; i++)
            if (needExtentsForTheseVariables[i] == s)
                alreadyHaveIt = true;
        if (!alreadyHaveIt)
            needExtentsForTheseVariables.push_back(s);
    }
    else
    {
        std::vector<std::string> newList;
        for (size_t i = 0 ; i < needExtentsForTheseVariables.size() ; i++)
            if (needExtentsForTheseVariables[i] != s)
                newList.push_back(needExtentsForTheseVariables[i]);
        needExtentsForTheseVariables = newList;
    }
}


// ****************************************************************************
//  Method: avtContract::GetAttribute
//
//  Purpose:
//      Get an operator or plot attribute to the contract so that 
//      communication can happen within the pipeline.
//
//  Programmer: Allen Sanderson
//  Creation:   June 16, 2014
//
// ****************************************************************************

MapNode*
avtContract::GetAttribute( std::string key )
{
  if( attributeMap.HasEntry( key ) )
  {
    return attributeMap.GetEntry( key );
  }
  else
    return NULL;
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
//    Hank Childs, Thu Aug 26 11:08:02 PDT 2010
//    Dump out extents info.
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
    webpage->AddTableEntry2("Replicating single domain on all processors",
                            YesOrNo(replicateSingleDomainOnAllProcessors));
    webpage->AddTableEntry2("Calculate extents of mesh",
                            YesOrNo(calculateMeshExtents));
    if (needExtentsForTheseVariables.size() == 0)
        strcpy(str, "none");
    else
    {
        strcpy(str, needExtentsForTheseVariables[0].c_str());
        for (size_t i = 1 ; i < needExtentsForTheseVariables.size() ; i++)
        {
            strcpy(str+strlen(str), "; ");
            strcpy(str+strlen(str), needExtentsForTheseVariables[i].c_str());
        }
    }
    webpage->AddTableEntry2("Variables to calculate extents for", str);
    sprintf(str, "%d", nFilters);
    webpage->AddTableEntry2("Number of known filters", str);
    webpage->EndTable();

    data->DebugDump(webpage);
}


// ****************************************************************************
//  Method: avtContract::Print
//
//  Purpose:
//       Contract print function. Uses overloaed << operator.
//
//  Programmer: Cyrus Harrison
//  Creation:   June 5, 2001
//
// ****************************************************************************

void
avtContract::Print(ostream &os)
{
    os << *this;
}
