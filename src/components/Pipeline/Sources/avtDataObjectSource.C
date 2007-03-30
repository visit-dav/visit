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


