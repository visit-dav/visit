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
//                            avtDataObjectSource.C                          //
// ************************************************************************* //

#include <avtDataObjectSource.h>

#include <AbortException.h>


//
// Static variables.
//

AbortCallback           avtDataObjectSource::abortCallback = NULL;
void                   *avtDataObjectSource::abortCallbackArgs = NULL;
ProgressCallback        avtDataObjectSource::progressCallback = NULL;
void                   *avtDataObjectSource::progressCallbackArgs = NULL;


// ****************************************************************************
//  Method: avtDataObjectSource constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectSource::avtDataObjectSource()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectSource::~avtDataObjectSource()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectSource::ReleaseData
//
//  Purpose:
//      Makes the output release any data that it has as a memory savings.
//
//  Programmer: Hank Childs
//  Creation:   February 19, 2005
//
// ****************************************************************************

void
avtDataObjectSource::ReleaseData(void)
{
    GetOutput()->ReleaseData();
}


// ****************************************************************************
//  Method: avtDataObjectSource::RegisterAbortCallback
//
//  Purpose:
//      Registers an AbortCallback.  Before (and sometimes during) each 
//      Execute, this will be called to see if execution should be aborted.
//
//  Arguments:
//      ab      The abort callback.
//      args    The arguments to the abort callback.
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 14:58:42 PDT 2001
//    Pushed method from derived type avtFilter to this base type.
//
// ****************************************************************************

void
avtDataObjectSource::RegisterAbortCallback(AbortCallback ab, void *args)
{
    abortCallback     = ab;
    abortCallbackArgs = args;
}


// ****************************************************************************
//  Method: avtDataObjectSource::RegisterProgressCallback
//
//  Purpose:
//      Registers the ProgressCallback.  This will be called during an Execute
//      as some portion (that can be easily identified) is completed.
//
//  Arguments:
//      pc      The progress callback.
//      args    The arguments to the progress callback.
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 14:58:42 PDT 2001
//    Pushed method from derived type avtFilter to this base type.
//
// ****************************************************************************

void
avtDataObjectSource::RegisterProgressCallback(ProgressCallback pc, void *args)
{
    progressCallback     = pc;
    progressCallbackArgs = args;
}


// ****************************************************************************
//  Method: avtDataObjectSource::CheckAbort
//
//  Purpose:
//      Goes through the callback mechanism to determine if we should abort the
//      Update/Execute cycle.
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 14:58:42 PDT 2001
//    Pushed method from derived type avtFilter to this base type.
//
// ****************************************************************************

void
avtDataObjectSource::CheckAbort(void)
{
    if (abortCallback != NULL)
    {
        if (abortCallback(abortCallbackArgs))
        {
            EXCEPTION0(AbortException);
        }
    }
}


// ****************************************************************************
//  Method: avtDataObjectSource::UpdateProgress
//
//  Purpose:
//      Updates the progress of the filter through a callback.
//
//  Arguments:
//      current       The current node of a data tree that was just finished.
//      total         The total number of trees.
//      typ           The class type doing the work.
//      descr         The description of the work being done.
//
//
//  Notes:            This method is also used to indicate that a filter is
//                    totally done.  When that happens, the callback is called
//                    with total == 0.  Also, the name of description can be
//                    NULL.
//
//  Programmer:       Hank Childs
//  Creation:         June 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 14:58:42 PDT 2001
//    Pushed method from derived type avtFilter to this base type.
//
//    Jeremy Meredith, Wed Sep 19 13:57:43 PDT 2001
//    Added a check for an Abort as part of every update progress call.
//
// ****************************************************************************

void
avtDataObjectSource::UpdateProgress(int current, int total, const char *typ,
                                    const char *descr)
{
    CheckAbort();
    if (progressCallback != NULL)
    {
        progressCallback(progressCallbackArgs, typ, descr, current, total);
    }
}


