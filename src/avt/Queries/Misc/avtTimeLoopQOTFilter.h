// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtTimeLoopQOTFilter.h                            //
// ************************************************************************* //

#ifndef AVT_TIME_LOOP_QOT_FILTER_H
#define AVT_TIME_LOOP_QOT_FILTER_H

#include <query_exports.h>

#include <avtQueryOverTimeFilter.h>

#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>

#include <QueryOverTimeAttributes.h>

#include <string>

class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtTimeLoopQOTFilter
//
//  Purpose:
//    Performs a query over time.
//
//  Note: This class was previously named avtQueryOverTimeFilter. Now that
//        there are multiple QOT filters to choose from, a base class has
//        been created with the name avtQueryOverTimeFilter, and this class
//        has been renamed avtTimeLoopQOTFilter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 19, 2004
//
//  Modifications:
//    Brad Whitlock, Wed Apr 14 14:56:45 PST 2004
//    Fixed for Windows.
//
//    Kathleen Bonnell, Tue May  4 14:21:37 PDT 2004
//    Removed SilUseSet in favor of SILRestrictionAttributes.
//
//    Kathleen Bonnell, Thu Jan  6 11:12:35 PST 2005
//    Added inheritance from avtTimeLoopFilter, which handles the stepping
//    through time.  Removed PostExecute method.  Added CreateFinalOutput,
//    ExecutionSuccessful (required by new inheritance).  Added qRes, times
//    (used to be stored by avtDataSetQuery::PerformQueryInTime).  Added
//    sucess and finalOutputCreated.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added CreatePolys method, and members useTimeForXAxis, nResultsToStore.
//
//    Kathleen Bonnell, Thu Jul 27 17:43:38 PDT 2006
//    Curves now represented as 1D RectilinearGrid, Renamed CreatedPolys to
//    CreateRGRid.
//
//    Hank Childs, Thu Feb  8 09:52:25 PST 2007
//    Try to correctly quote how many additional filters there will be
//    for one time step.
//
//    Kathleen Bonnell, Wed Nov 28 16:33:22 PST 2007
//    Added member 'label', to store a shorter y-axis label than the query
//    name if desired.
//
//    Kathleen Bonnell, Tue Jul  8 15:48:11 PDT 2008
//    Add useVarForYAxis.
//
//    Hank Childs, Fri Dec 24 17:59:02 PST 2010
//    Add method FilterSupportsTimeParallelization.
//
//    Kathleen Bonnell, Thu Feb 17 09:50:28 PST 2011
//    Replace CreateRGrid with CreateTree, to allow multiple outputs.
//
//    Alister Maguire, Wed May 23 09:21:45 PDT 2018
//    Added cacheIdx and useCache to allow plotting pick curves
//    from cached picks.
//
//    Alister Maguire, Mon Sep 23 11:13:48 MST 2019
//    Changed name from avtQueryOverTimeFilter to avtTimeLoopQOTFilter,
//    and added avtQueryOverTimeFilter inheritance. Also removed methods
//    that are now defined in avtQueryOverTimeFilter.
//
//    Kathleen Biagas, Wed Sep 11, 2024
//    Rename 'label' as 'yLabel', add 'outputLabel'.
//
// ****************************************************************************

class QUERY_API avtTimeLoopQOTFilter : public avtTimeLoopFilter,
                                       public avtQueryOverTimeFilter
{
  public:
                          avtTimeLoopQOTFilter(const AttributeGroup*);
    virtual              ~avtTimeLoopQOTFilter();

    static avtFilter     *Create(const AttributeGroup*);

    virtual const char   *GetType(void)  { return "avtTimeLoopQOTFilter"; }
    virtual const char   *GetDescription(void) { return "Querying over Time"; }

    virtual bool          FilterSupportsTimeParallelization(void);

  protected:
    doubleVector          qRes;
    doubleVector          times;
    bool                  success;
    bool                  finalOutputCreated;
    std::string           yLabel;
    std::string           outputLabel;

    bool                  useTimeForXAxis;
    bool                  useVarForYAxis;
    int                   nResultsToStore;
    int                   numAdditionalFilters;
    int                   cacheIdx;
    bool                  useCache;

    virtual void          Execute(void);
    virtual void          UpdateDataObjectInfo(void);

    virtual int           AdditionalPipelineFilters(void)
                                            { return numAdditionalFilters; }

    virtual void          CreateFinalOutput(void);
    virtual bool          ExecutionSuccessful(void) { return success; }
    avtDataTree_p         CreateTree(const doubleVector &,
                                     const doubleVector &,
                                     stringVector &,
                                     const bool);
};


#endif


