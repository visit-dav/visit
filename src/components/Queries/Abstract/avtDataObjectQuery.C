// ************************************************************************* //
//                           avtDataObjectQuery.C                            //
// ************************************************************************* //

#include <avtDataObjectQuery.h>
#include <NonQueryableInputException.h>



ProgressCallback     avtDataObjectQuery::progressCallback = NULL;
void                *avtDataObjectQuery::progressCallbackArgs = NULL;

InitializeProgressCallback
                     avtDataObjectQuery::initializeProgressCallback = NULL;
void                *avtDataObjectQuery::initializeProgressCallbackArgs=NULL;


// ****************************************************************************
//  Method: avtDataObjectQuery::RegisterInitializeProgressCallback
//
//  Purpose:
//      Registers the InitializeProgressCallback.  This will be called before
//      any pipeline execution occurs to declare how many filters there are in
//      the whole pipeline.
//
//  Arguments:
//      pc      The initialize progress callback.
//      args    The arguments to the initialize progress callback.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 27, 2002 
//
// ****************************************************************************
 
void
avtDataObjectQuery::RegisterInitializeProgressCallback(
                                     InitializeProgressCallback pc, void *args)
{
    initializeProgressCallback     = pc;
    initializeProgressCallbackArgs = args;
}

// ****************************************************************************
//  Method: avtDataObjectQuery::RegisterProgressCallback
//
//  Purpose:
//      Registers the ProgressCallback.  This will be called during a 
//      PerformQuery as some portion (that can be easily identified) is 
//      completed.
//
//  Arguments:
//      pc      The progress callback.
//      args    The arguments to the progress callback.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 27, 2002 
//
// ****************************************************************************
 
void
avtDataObjectQuery::RegisterProgressCallback(ProgressCallback pc, void *args)
{
    progressCallback     = pc;
    progressCallbackArgs = args;
}

// ****************************************************************************
//  Method: avtDataObjectSource::UpdateProgress
//
//  Purpose:
//      Updates the progress of the query through a callback.
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
//  Programmer:       Kathleen Bonnell 
//  Creation:         September 27, 2002 
//
// ****************************************************************************
 
void
avtDataObjectQuery::UpdateProgress(int current, int total)
{
    if (progressCallback != NULL)
    {
        progressCallback(progressCallbackArgs, GetType(), GetDescription(), current, total);
    }
}


// ****************************************************************************
//  Method: avtDataObjectQuery::Init
//
//  Purpose:
//      Initialize progress call back with the number of stages in the Query. 
//
//  Programmer:       Kathleen Bonnell 
//  Creation:         September 27, 2002 
//
// ****************************************************************************

void
avtDataObjectQuery::Init()
{
    if (initializeProgressCallback != NULL) 
    {
        //
        // Each filter is a stage, plus a stage for the query. 
        //
        int nstages = GetNFilters() + 1;

        initializeProgressCallback(initializeProgressCallbackArgs,nstages);
    }
}


// ****************************************************************************
//  Method: avtDataObjectQuery::GetNFilters
//
//  Purpose:
//    A stub that allows derived types not to define this.  Returns the
//    number of filters that the query requires to be executed.  
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    September 27, 2002 
//
// ****************************************************************************

int
avtDataObjectQuery::GetNFilters()
{
    return 0;
}

// ****************************************************************************
//  Method: avtDataObjectQuery::ChangedInput
//
//  Purpose:
//    Catches the hook from the base class that the input has changed.                             
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 22, 2002 
//
// ****************************************************************************

void
avtDataObjectQuery::ChangedInput()
{
    //
    // Give the derived types an opportunity to throw an exception if they
    // don't like the input.
    //
    VerifyInput(); 
}


// ****************************************************************************
//  Method: avtDataObjectQuery::VerifyInput
//
//  Purpose:
//    This is a chance for the derived types to verify a new input.  
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 22, 2002 
//
// ****************************************************************************

void
avtDataObjectQuery::VerifyInput()
{
    //
    //   A few general tests for all query types.
    //
    if ((!GetInput()->GetInfo().GetValidity().GetQueryable()) ||
        (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0))
    {
        EXCEPTION0(NonQueryableInputException);
    }
}

