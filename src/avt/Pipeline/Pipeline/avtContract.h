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
//                               avtContract.h                               //
// ************************************************************************* //

#ifndef AVT_CONTRACT_H
#define AVT_CONTRACT_H

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataRequest.h>

class avtWebpage;
class avtContract;
typedef ref_ptr<avtContract> avtContract_p;


// ****************************************************************************
//  Class: avtContract
//
//  Purpose:
//      This is the specification of a pipeline.  This includes the
//      specification of which data you want, as well as a pipeline index to
//      be used later for load balancing and a boolean value indicating if
//      streaming is possible.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Thu Jul 26 12:35:52 PDT 2001
//    Added ShouldUseDynamicLoadBalancing.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Sun Mar 13 09:49:16 PST 2005
//    Added haveStructuredMeshOptimizations.
//
//    Hank Childs, Fri Jun 15 12:41:41 PDT 2007
//    Added support for DebugDump.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and modify definition of assignment 
//    operator to prevent accidental use of default, bitwise copy 
//    implementations.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Sun Mar  9 06:36:49 PST 2008
//    Add new data member for on demand streaming.
//
// ****************************************************************************

class PIPELINE_API avtContract
{
  public:
                        avtContract(avtDataRequest_p, int);
                        avtContract(avtContract_p);
                        avtContract(avtContract_p, avtDataRequest_p);
    virtual            ~avtContract();

    bool                ShouldUseStreaming(void) { return canDoStreaming; };
    void                NoStreaming(void) { canDoStreaming = false; };
    void                SetDataRequest(avtDataRequest_p ds) { data = ds; };

    bool                ShouldUseLoadBalancing(void)  
                               { return  useLoadBalancing && 
                                        !doingOnDemandStreaming; };
    void                UseLoadBalancing(bool);

    void                SetHaveRectilinearMeshOptimizations(bool b)
                               { haveRectilinearMeshOptimizations = b; };
    bool                GetHaveRectilinearMeshOptimizations(void)
                               { return haveRectilinearMeshOptimizations; };
    void                SetHaveCurvilinearMeshOptimizations(bool b)
                               { haveCurvilinearMeshOptimizations = b; };
    bool                GetHaveCurvilinearMeshOptimizations(void)
                               { return haveCurvilinearMeshOptimizations; };
                  
    bool                DoingOnDemandStreaming(void)
                               { return doingOnDemandStreaming; };
    void                SetOnDemandStreaming(bool b)
                               { doingOnDemandStreaming = b; };

    avtDataRequest_p    GetDataRequest(void)   { return data; };
    int                 GetPipelineIndex(void) { return pipelineIndex; };

    void                AddFilter(void)  { nFilters++; };
    int                 GetNFilters(void)  { return nFilters; };

    avtContract        &operator=(const avtContract &);
    void                DebugDump(avtWebpage *);

  protected:
    avtDataRequest_p    data;
    int                 pipelineIndex;
    bool                canDoStreaming;
    bool                doingOnDemandStreaming;
    bool                useLoadBalancing;
    bool                haveCurvilinearMeshOptimizations;
    bool                haveRectilinearMeshOptimizations;
    int                 nFilters;

  private:
    // This method is defined to prevent accidental use of a bitwise copy
    // implementation.  If you want to re-define it to do something
    // meaningful, that's fine.
    avtContract(const avtContract &) {;};
};


#endif


