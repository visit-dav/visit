// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtNumNodesQuery.h                             //
// ************************************************************************* //

#ifndef AVT_NUMNODES_QUERY_H
#define AVT_NUMNODES_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>


// ****************************************************************************
//  Class: avtNumNodesQuery
//
//  Purpose:
//      This is a dataset query that returns the number of nodes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//    Kathleen Biagas, Tue Jul 26 15:13:06 PDT 2011
//    Add GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtNumNodesQuery : public avtDatasetQuery
{
  public:
                              avtNumNodesQuery();
    virtual                  ~avtNumNodesQuery(); 

    virtual const char        *GetType(void) { return "avtNumNodesQuery"; };
    virtual const char        *GetDescription(void) 
                                  { return "Getting the number of nodes"; };


    virtual void             PerformQuery(QueryAttributes *);

    static void              GetDefaultInputParams(MapNode &);

  protected:

    virtual void             Execute(vtkDataSet*, const int){;};
};

#endif

