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
//                             avtTerminatingSink.h                          //
// ************************************************************************* //

#ifndef AVT_TERMINATING_SINK_H
#define AVT_TERMINATING_SINK_H

#include <pipeline_exports.h>

#include <avtDataObjectSink.h>
#include <avtContract.h>

class     avtWebpage;


typedef  bool (*GuideFunction)(void *, int);


// ****************************************************************************
//  Class: avtTerminatingSink
//
//  Purpose:
//      This sink object serves as the terminator of a pipeline.  It 
//      understands that there are many pipelines and what its pipeline index
//      is.  It also understands that streaming may occur and
//      that it may have to execute a pipeline multiple times.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep 28 13:18:47 PDT 2001
//    Added DynamicLoadBalanceCleanUp.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers 
//    have problems with them.
//
//    Hank Childs, Wed Mar  2 11:16:01 PST 2005
//    Take a full-blown pipeline specification rather than a data spec and a
//    pipeline index.
//
//    Hank Childs, Thu Dec 21 09:43:22 PST 2006
//    Add support for debug dumps
//
//    Hank Childs, Fri Jun 15 16:11:00 PDT 2007
//    Add support for indentation of debug dumps.
//
//    Cyrus Harrison, Wed Feb 13 11:31:58 PST 2008
//    Global debug dump flag was migrated to avtDebugDumpOptions.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

class PIPELINE_API avtTerminatingSink : virtual public avtDataObjectSink
{
  public:
                              avtTerminatingSink();
    virtual                  ~avtTerminatingSink();

    void                      Execute(avtContract_p);

    static void               SetGuideFunction(GuideFunction, void *);
    static void               GetGuideFunction(GuideFunction &, void *&);

    static void               AddDumpReference(const char *, const char *, int);

  protected:
    virtual void              InputIsReady(void);
    virtual void              StreamingCleanUp(void);

    static avtWebpage        *webpage;

    void                      FinalizeWebpage(void);
    void                      InitializeWebpage(void);

  private:
    static GuideFunction      guideFunction;
    static void              *guideFunctionArgs;
};


#endif


