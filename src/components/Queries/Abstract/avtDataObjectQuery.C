// ************************************************************************* //
//                           avtDataObjectQuery.C                            //
// ************************************************************************* //

#include <avtDataObjectQuery.h>
#include <NonQueryableInputException.h>
#include <avtSILRestriction.h>



ProgressCallback     avtDataObjectQuery::progressCallback = NULL;
void                *avtDataObjectQuery::progressCallbackArgs = NULL;

InitializeProgressCallback
                     avtDataObjectQuery::initializeProgressCallback = NULL;
void                *avtDataObjectQuery::initializeProgressCallbackArgs=NULL;


// ****************************************************************************
//  Method: avtDataObjectQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications: 
//    Kathleen Bonnell, Wed Mar 31 15:52:54 PST 2004
//    Initialize new member timeVarying.
//
//    Kathleen Bonnell, Tue May  4 14:18:26 PDT 2004 
//    Initialize new member querySILR.
//
// ****************************************************************************

avtDataObjectQuery::avtDataObjectQuery()
{
    timeVarying = false;
    querySILR = NULL;
}


// ****************************************************************************
//  Method: avtDataObjectQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectQuery::~avtDataObjectQuery()
{
    ;
}


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
//  Modifications: 
//    Kathleen Bonnell, Wed Mar 31 15:52:54 PST 2004
//    Added nTimesteps argument.
// 
// ****************************************************************************

void
avtDataObjectQuery::Init(const int nTimesteps)
{
    if (initializeProgressCallback != NULL) 
    {
        //
        // Each filter is a stage, plus a stage for the query. 
        //
        int nstages = (GetNFilters() + 1) * nTimesteps;

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
//  Kathleen Bonnell, Fri Jul 11 16:33:16 PDT 2003
//  Retrieve units.
//
//  Kathleen Bonnell, Wed Jul 28 08:26:05 PDT 2004
//  Removed unit retrieval.  Derived types can set it themselves, as some
//  want var-units others want mesh-units.
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


// ****************************************************************************
//  Method: avtDataObjectQuery::SetSILRestriction
//
//  Purpose:
//    Creates a new avtSILRestriction from the passed Attributes. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    May 4, 2004 
//
// ****************************************************************************

void
avtDataObjectQuery::SetSILRestriction(const SILRestrictionAttributes *silAtts)
{
    querySILR = new avtSILRestriction(*silAtts);
}
