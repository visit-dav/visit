// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtQueryOverTimeFilter.h                            //
// ************************************************************************* //

#ifndef AVT_QUERYOVERTIME_FILTER_H
#define AVT_QUERYOVERTIME_FILTER_H

#include <query_exports.h>

#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>

#include <QueryOverTimeAttributes.h>
#include <SILRestrictionAttributes.h>

#include <string>

class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtQueryOverTimeFilter
//
//  Purpose:
//    Performs a query over time. 
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
// ****************************************************************************

class QUERY_API avtQueryOverTimeFilter : public avtTimeLoopFilter,
                                         public avtDatasetToDatasetFilter
{
  public:
                          avtQueryOverTimeFilter(const AttributeGroup*);
    virtual              ~avtQueryOverTimeFilter();

    static avtFilter     *Create(const AttributeGroup*);

    virtual const char   *GetType(void)  { return "avtQueryOverTimeFilter"; };
    virtual const char   *GetDescription(void) { return "Querying over Time"; };

    void                  SetSILAtts(const SILRestrictionAttributes *silAtts);
    virtual bool          FilterSupportsTimeParallelization(void);

  protected:
    QueryOverTimeAttributes   atts;
    SILRestrictionAttributes  querySILAtts;
    doubleVector          qRes;
    doubleVector          times;
    bool                  success;
    bool                  finalOutputCreated;
    std::string           label;

    bool                  useTimeForXAxis;
    bool                  useVarForYAxis;
    int                   nResultsToStore;
    int                   numAdditionalFilters;
    int                   cacheIdx;
    bool                  useCache;

    virtual void          Execute(void);
    virtual void          UpdateDataObjectInfo(void);

    virtual int           AdditionalPipelineFilters(void) 
                                            { return numAdditionalFilters; };

    virtual void          CreateFinalOutput(void);
    virtual bool          ExecutionSuccessful(void) { return success; } ;
    avtDataTree_p         CreateTree(const doubleVector &, 
                                     const doubleVector &,
                                     stringVector &,
                                     const bool);
};


#endif


