// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                   avtDirectDatabaseQOTFilter.h                            //
// ************************************************************************* //

#ifndef AVT_DIRECT_DATABASE_QOT_FILTER_H
#define AVT_DIRECT_DATABASE_QOT_FILTER_H

#include <query_exports.h>

#include <QueryOverTimeAttributes.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtQueryOverTimeFilter.h>

#include <string>

class vtkRectilinearGrid;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class: avtDirectDatabaseQOTFilter
//
//  Purpose:
//      Perform a query over time by communciating directly with the database
//      readers.
//
//      Using this filter requires that the avtDataRequest be updated to
//      request a query over time dataset (QOTDataset), which can be
//      accomplished by setting the retrieveQOTDataset within avtDataRequest.
//
//      Using this filter is much faster than using the TimeLoopQOTFilter,
//      but there are some significant drawbacks that must be noted:
//
//          1. This filter can only retrieve "original" data, meaning that
//             the query will be performed before any other plots and filters
//             are applied (excluding the expression filter).
//          2. The QOTDataset is limited in its ability to process expressions.
//          3. This filter cannot preserve coordinates during a QOT pick.
//
//      When using "actual" data or complex expressions, the TimeLoopQOTFilter
//      should be relied on.
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 13:46:56 MST 2019
//
//  Modifications:
//    Alister Maguire, Thu Nov  5 10:00:31 PST 2020
//    Changed VerifyAndRefineTimesteps to VerifyAndRefinePointTimestep
//    and added VerifyAndRefineArrayTimesteps.
//
//    Kathleen Biagas, Wed Sep 11, 2024
//    Add outputLabel.
//
// ****************************************************************************

class QUERY_API avtDirectDatabaseQOTFilter : public avtQueryOverTimeFilter
{
  public:
                             avtDirectDatabaseQOTFilter(const AttributeGroup*);
    virtual                 ~avtDirectDatabaseQOTFilter();

    static avtFilter        *Create(const AttributeGroup*);

    virtual const char      *GetType(void)
                               { return "avtDirectDatabaseQOTFilter"; }
    virtual const char      *GetDescription(void)
                               { return "Querying over Time"; }

  protected:
    bool                     success;
    bool                     finalOutputCreated;
    bool                     useTimeForXAxis;
    bool                     useVarForYAxis;
    std::string              YLabel;
    std::string              outputLabel;

    virtual void             Execute(void);
    virtual void             UpdateDataObjectInfo(void);

    virtual bool             ExecutionSuccessful(void)
                               { return success; }

    vtkUnstructuredGrid     *VerifyAndRefinePointTimesteps(
                                 vtkUnstructuredGrid *);

    vtkUnstructuredGrid     *VerifyAndRefineArrayTimesteps(
                                 vtkUnstructuredGrid *);

    avtDataTree_p            ConstructCurveTree(vtkUnstructuredGrid *,
                                                const bool);
};

#endif
