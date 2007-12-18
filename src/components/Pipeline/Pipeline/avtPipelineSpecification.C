/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                          avtPipelineSpecification.C                       //
// ************************************************************************* //

#include <avtPipelineSpecification.h>

#include <avtWebpage.h>

// ****************************************************************************
//  Method: avtPipelineSpecification constructor
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
// ****************************************************************************

avtPipelineSpecification::avtPipelineSpecification(avtDataSpecification_p d,
                                                   int pi)
{
    data             = new avtDataSpecification(d);
    pipelineIndex    = pi;
    canDoDynamic     = true;
    useLoadBalancing = true;
    nFilters         = 0;
    haveCurvilinearMeshOptimizations = false;
    haveRectilinearMeshOptimizations = false;
}


// ****************************************************************************
//  Method: avtPipelineSpecification copy constructor
//
//  Arguments:
//      ps       A pipeline specification to copy.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2001
//
// ****************************************************************************

avtPipelineSpecification::avtPipelineSpecification(
                                                 avtPipelineSpecification_p ps)
{
    *this = **ps;
}


// ****************************************************************************
//  Method: avtPipelineSpecification copy constructor
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

avtPipelineSpecification::avtPipelineSpecification(
                      avtPipelineSpecification_p ps, avtDataSpecification_p ds)
{
    *this = **ps;
    data  = new avtDataSpecification(ds);
}


// ****************************************************************************
//  Method: avtPipelineSpecification destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtPipelineSpecification::~avtPipelineSpecification()
{
    ;
}


// ****************************************************************************
//  Method: avtPipelineSpecification assignment operator
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
// ****************************************************************************

avtPipelineSpecification &
avtPipelineSpecification::operator=(const avtPipelineSpecification &ps)
{
    data             = new avtDataSpecification(ps.data);
    pipelineIndex    = ps.pipelineIndex;
    canDoDynamic     = ps.canDoDynamic;
    useLoadBalancing = ps.useLoadBalancing;
    nFilters         = ps.nFilters;
    haveCurvilinearMeshOptimizations = ps.haveCurvilinearMeshOptimizations;
    haveRectilinearMeshOptimizations = ps.haveRectilinearMeshOptimizations;

    return *this;
}


// ****************************************************************************
//  Method: avtPipelineSpecification::UseLoadBalancing
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
avtPipelineSpecification::UseLoadBalancing(bool newVal)
{
    useLoadBalancing = newVal;
}


// ****************************************************************************
//  Method: avtPipelineSpecification::DebugDump
//
//  Purpose:
//      Dumps data members to a webpage.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2007
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
avtPipelineSpecification::DebugDump(avtWebpage *webpage)
{
    char str[1024];

    webpage->AddSubheading("Pipeline attributes");
    webpage->StartTable();
    webpage->AddTableHeader2("Field", "Value");
    sprintf(str, "%d", pipelineIndex);
    webpage->AddTableEntry2("Pipeline index", str);
    webpage->AddTableEntry2("Supports dynamic load balancing", 
                            YesOrNo(canDoDynamic));
    webpage->AddTableEntry2("Should use load balancing", 
                            YesOrNo(useLoadBalancing));
    webpage->AddTableEntry2("Have curvilinear optimizations", 
                            YesOrNo(haveCurvilinearMeshOptimizations));
    webpage->AddTableEntry2("Have rectilinear optimizations", 
                            YesOrNo(haveRectilinearMeshOptimizations));
    sprintf(str, "%d", nFilters);
    webpage->AddTableEntry2("Number of known filters", str);
    webpage->EndTable();

    data->DebugDump(webpage);
}


