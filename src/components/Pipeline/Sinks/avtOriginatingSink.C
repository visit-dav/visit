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
//                            avtOriginatingSink.C                           //
// ************************************************************************* //

#include <avtOriginatingSink.h>

#include <snprintf.h>

#include <avtPipelineSpecification.h>
#include <avtParallel.h>
#include <avtWebpage.h>

#include <AbortException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>
#include <TimingsManager.h>


//
// Define static members.
//

GuideFunction    avtOriginatingSink::guideFunction     = NULL;
void            *avtOriginatingSink::guideFunctionArgs = NULL;
bool             avtOriginatingSink::debugDump         = false;
avtWebpage      *avtOriginatingSink::webpage           = NULL;


// ****************************************************************************
//  Method: avtOriginatingSink constructor
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Mar  2 11:17:20 PST 2005
//    Remove old data member pipelineIndex.
//
// ****************************************************************************

avtOriginatingSink::avtOriginatingSink()
{
}


// ****************************************************************************
//  Method: avtOriginatingSink destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtOriginatingSink::~avtOriginatingSink()
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingSink::Execute
//
//  Purpose:
//      Executes the pipeline.  This means possibly executing the pipeline
//      multiple times when dynamic load balancing is necessary.
//
//  Arguments:
//      spec    The pipeline specification this pipeline should use.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 20 18:09:20 PDT 2001
//    Added support for aborting execution.
//
//    Jeremy Meredith, Thu Jul 26 12:34:47 PDT 2001
//    Added merging of partial updates as per Hank's code.
//
//    Hank Childs, Mon Jul 30 15:08:33 PDT 2001
//    Only clone and repeatedly merge for the second Update and after.
//
//    Hank Childs, Sun Sep 16 14:49:53 PDT 2001
//    Add flow of control debugging information.
//
//    Jeremy Meredith, Wed Sep 19 13:59:14 PDT 2001
//    Removed check for AbortException since the proper action is to simply
//    rethrow it from this function anyway -- so just don't catch it.
//
//    Hank Childs, Fri Sep 28 13:18:47 PDT 2001
//    Added hook for cleaning up after dynamic load balancing.
//
//    Hank Childs, Sat Feb 19 14:46:05 PST 2005
//    Added timings for dynamic load balancing.
//
//    Hank Childs, Wed Mar  2 11:17:20 PST 2005
//    Take a pipeline specification rather than a data specification.
//
// ****************************************************************************

void
avtOriginatingSink::Execute(avtPipelineSpecification_p pipelineSpec)
{
    if (debugDump)
        InitializeWebpage();

    int pipelineIndex = pipelineSpec->GetPipelineIndex();
    if (pipelineIndex < 0)
    {
        //
        // We have to know what pipeline we are talking about.
        //
        EXCEPTION0(ImproperUseException);
    }

    avtDataObject_p input = GetInput();
    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    if (!guideFunction || pipelineIndex == 0)
    {
        debug4 << "No guide function registered with the originating sink,"
               << " doing normal Update." << endl;
        input->Update(pipelineSpec);
    }
    else
    {
        //
        // Handle the first execution differently since we don't want to do
        // extra work if we don't have to.
        //
        if (guideFunction(guideFunctionArgs, pipelineIndex))
        {
            debug4 << "Guide function indicated that we should do the "
                   << "first Update on pipeline " << pipelineIndex << "." 
                   << endl;
            int t = visitTimer->StartTimer();
            input->Update(pipelineSpec);
            visitTimer->StopTimer(t, "First pipeline update.");
        }

        //
        // Now make a copy and continue if there are going to be multiple
        // updates (ie dynamic load balancing mode).
        //
        int iter = 1;
        if (guideFunction(guideFunctionArgs, pipelineIndex))
        {
            avtDataObject_p dob = input->Clone();
            while (guideFunction(guideFunctionArgs, pipelineIndex))
            {
                debug4 << "Doing " << iter << " iteration Updating on "
                       << "pipeline " << pipelineIndex << "." << endl;
                int t = visitTimer->StartTimer();
                input->Update(pipelineSpec);
                char msg[1024];
                SNPRINTF(msg, 1024, "Iteration %d of dynamic LB update.",iter);
                visitTimer->StopTimer(t, msg);
                dob->Merge(*input);
                iter++;
            }
            input->Copy(*dob);
            int t2 = visitTimer->StartTimer();
            DynamicLoadBalanceCleanUp();
            visitTimer->StopTimer(t2, "Time to do DLB clean up");
        }
        debug4 << "Done with iterating Updates on pipeline "
               << pipelineIndex << endl;
    }

    InputIsReady();

    if (debugDump)
        FinalizeWebpage();
}


// ****************************************************************************
//  Method: avtOriginatingSink::InputIsReady
//
//  Purpose:
//      Called when Execute is done, so that derived types (like Mappers) that
//      need the input to up-to-date, can set up.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

void
avtOriginatingSink::InputIsReady(void)
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingSink::DynamicLoadBalanceCleanUp
//
//  Purpose:
//      A hook to allow derived types to perform some clean up after dynamic
//      load balancing.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2001
//
// ****************************************************************************

void
avtOriginatingSink::DynamicLoadBalanceCleanUp(void)
{
    ;
}


// ****************************************************************************
//  Method: avtOriginatingSink::SetGuideFunction
//
//  Purpose:
//      Sets the load balancer to be consulted when this originating sink
//      determines if it should do another update.
//
//  Arguments:
//      foo     The function that will call a load balancer.
//      args    The arguments for the function.
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
// ****************************************************************************

void
avtOriginatingSink::SetGuideFunction(GuideFunction foo, void *args)
{
    guideFunction     = foo;
    guideFunctionArgs = args;
}


// ****************************************************************************
//  Method: avtOriginatingSink::GetGuideFunction
//
//  Purpose:
//      Gets the load balancer to be consulted when this originating sink
//      determines if it should do another update.
//
//  Arguments:
//      foo     The function that will call a load balancer.
//      args    The arguments for the function.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2004
//
// ****************************************************************************

void
avtOriginatingSink::GetGuideFunction(GuideFunction &foo, void *&args)
{
    foo  = guideFunction;
    args = guideFunctionArgs;
}


// ****************************************************************************
//  Method: avtOriginatingSink::InitializeWebpage
//
//  Purpose:
//      Opens up a file stream to write a webpage to.  (For debugDump mode.)
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtOriginatingSink::InitializeWebpage(void)
{
    if (webpage != NULL)
    {
        debug1 << "NOTE: already an existing webpage ... maybe there were "
               << "nested execution calls ...\n" 
               << "or maybe there was an error the last time." << endl;
        delete webpage;
    }

    static int id = 0;
    char name[128];
    if (PAR_Size() > 1)
    {
        int rank = PAR_Rank();
        sprintf(name, "visit_dump_%d.%d.html", id, rank);
    }
    else
        sprintf(name, "visit_dump_%d.html", id);

    webpage = new avtWebpage(name);
    webpage->InitializePage("VisIt pipeline contents");
    char title[128];
    sprintf(title, "Pipeline %d", id);
    webpage->WriteTitle(title);

    id++;
}


// ****************************************************************************
//  Method: avtOriginatingSink::FinalizeWebpage
//
//  Purpose:
//      Closes file stream we were writing a webpage to.  (For debugDump mode.)
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
// ****************************************************************************

void
avtOriginatingSink::FinalizeWebpage(void)
{
    if (webpage == NULL)
    {
        debug1 << "NOTE: webpage has already been closed ... shouldn't "
               << "happen ... maybe there were "
               << "nested execution calls ...\n"  << endl;
        return;
    }

    webpage->FinalizePage();
    delete webpage;
    webpage = NULL;
}


// ****************************************************************************
//  Method: avtOriginatingSink::DumpString
//
//  Purpose:
//      Dumps a string to the webpage.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Jun 15 11:33:24 PDT 2007
//    Added "indentation_level" argument, which indicates if this filter is part
//    of the execution of another filter, and, if so, indents it.
//
// ****************************************************************************

void
avtOriginatingSink::AddDumpReference(const char *filename, const char *listing,
                                     int indentation_level)
{
    if (webpage == NULL)
    {
        debug1 << "Unable to dump info to webpage" << endl;
        return;
    }

    char entry[1024] = { '\0' };
    for (int i = 0 ; i < indentation_level ; i++)
    {
        strcpy(entry + strlen(entry), "--> ");
    }
    strcpy(entry + strlen(entry), listing);
    
    webpage->AddLink(filename, entry);
}


