// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtActualDataNumNodesQuery.h                         //
// ************************************************************************* //

#ifndef AVT_ACTUALDATA_NUMNODES_QUERY_H
#define AVT_ACTUALDATA_NUMNODES_QUERY_H
#include <query_exports.h>

#include <avtNumNodesQuery.h>

class avtCondenseDatasetFilter;


// ****************************************************************************
//  Class: avtActualDataNumNodesQuery
//
//  Purpose:
//      This is a dataset query that returns the number of nodes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep 25 13:37:13 PDT 2008
//    Added ApplyFilter, GetNFilters and condense filter.
//
// ****************************************************************************

class QUERY_API avtActualDataNumNodesQuery : public avtNumNodesQuery
{
  public:
                              avtActualDataNumNodesQuery();
    virtual                  ~avtActualDataNumNodesQuery(); 

    virtual bool             OriginalData(void) { return false; };

  protected:

    virtual avtDataObject_p   ApplyFilters(avtDataObject_p);   
    virtual int               GetNFilters() { return 1; };

  private:

    avtCondenseDatasetFilter *condense;

};

#endif
