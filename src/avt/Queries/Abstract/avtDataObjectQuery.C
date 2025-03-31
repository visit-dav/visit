// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Kathleen Bonnell, Tue Jul  8 18:03:40 PDT 2008
//    Set default values for timeCurveSpecs.
//
//    Hank Childs, Fri Dec 24 17:52:28 PST 2010
//    Initialize parallelizingOverTime.
//
//    Kathleen Biagas, Wed Sep 11, 2024
//    Add 'outputCurveLabel' to timeCurveSpecs.
//
// ****************************************************************************

avtDataObjectQuery::avtDataObjectQuery()
{
    timeVarying = false;
    parallelizingOverTime = false;
    querySILR = NULL;

    // derived classes should overide these as necessary
    timeCurveSpecs["useTimeForXAxis"] = true;
    timeCurveSpecs["useVarForYAxis"] = false;
    timeCurveSpecs["nResultsToStore"] = 1;
    timeCurveSpecs["outputCurveLabel"] = "";
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
        progressCallback(progressCallbackArgs, GetType(), GetDescription(),
                         current, total);
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
//    Hank Childs, Thu Feb  8 14:39:07 PST 2007
//    Do not initialize the progress callback if there is more than 1 timestep.
//    The terminating source will do the initialization in that case.
//    (And the QueryOverTimeFilter will call this method once for each
//     timestep, which means the progress will be re-initialized
//     inappropriately.)
//
//    Hank Childs, Thu Jan 31 16:35:44 PST 2008
//    Improve the test for whether or not to call the initialize progress
//    callback.
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

        if (!timeVarying)
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
//  Modifications:
//    Kathleen Bonnell, Fri Sep  3 08:33:47 PDT 2004
//    As the base class for all queries, made this the least restrictive.
//    Any derived type can override this method to make it more restrictive.
//
// ****************************************************************************

void
avtDataObjectQuery::VerifyInput()
{
    if (!GetInput()->GetInfo().GetValidity().GetQueryable())
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


// ****************************************************************************
//  Method: avtDataObjectQuery::SetSILRestriction
//
//  Purpose:
//    Creates a new avtSILRestriction from the passed restriction .
//
//  Programmer:  Kathleen Bonnell
//  Creation:    January 3, 2005
//
// ****************************************************************************

void
avtDataObjectQuery::SetSILRestriction(const avtSILRestriction_p silr)
{
    querySILR = new avtSILRestriction(silr);
}


// ****************************************************************************
//  Method: avtDataObjectQuery::GetTimeCurveSpecs
//
//  Purpose:
//    Sets some flags used by QueryOverTime.
//    Defined here so derived types don't have to.
//    The default type of time curve is a Single curve using Time for
//    the X-axis.
//
//  Arguments:
//    QueryAttributes, in case query needs info from the atts for
//    for filling in the timeCurveSpecs.
//
//  Notes:
//    If useTimeForXAxis is true, then nResultsToStore should be 1 unless
//    multiple curves are desired (not yet implemented by QOT).
//
//    If useTimeForXAxis is false, then nResultsToStore should be 2^n where
//    n is the number of curves desired. Odd-indexed results will be used
//    for X-axis, even-indexed results will be used for Y Axis.
//
//    If useVarForYAxis is true, then the query variable and its units will
//    be used to set the Y-axis label.
//
//    If useVarForYAxis is false, then the Y-axis label is the query name
//    or short description.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    January 3, 2005
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul  8 15:39:30 PDT 2008
//    Modified to use a MapNode and added 'useVarForYAxis' element.
//
//    Kathleen Biagas, Wed Sep 11, 2024
//    Added QueryAttributes argument, some queries may need info from the
//    atts for filling in new element 'outputCurveLabel'.
//
// ****************************************************************************

const MapNode&
avtDataObjectQuery::GetTimeCurveSpecs(const QueryAttributes *)
{
    // The defaults are listed below, derived classes should overide
    // whichever of these as necessary
    //
    // timeCurveSpecs["useTimeForXAxis"] = true;
    // timeCurveSpecs["useVarForYAxis"] = false;
    // timeCurveSpecs["nResultsToStore"] = 1;
    // timeCurveSpecs["outputCurveLabel"] = "";
    return timeCurveSpecs;
}

