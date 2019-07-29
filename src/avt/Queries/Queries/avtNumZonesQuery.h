// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtNumZonesQuery.h                             //
// ************************************************************************* //

#ifndef AVT_NUMZONES_QUERY_H
#define AVT_NUMZONES_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>


// ****************************************************************************
//  Class: avtNumZonesQuery
//
//  Purpose:
//      This is a dataset query that returns the number of zones.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//    Kathleen Biagas, Tue Jul 26 15:13:06 PDT 2011
//    Add GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtNumZonesQuery : public avtDatasetQuery
{
  public:
                              avtNumZonesQuery();
    virtual                  ~avtNumZonesQuery(); 

    virtual const char        *GetType(void) { return "avtNumZonesQuery"; };
    virtual const char        *GetDescription(void) 
                                  { return "Getting the number of zones"; };

    virtual void             PerformQuery(QueryAttributes *);

    static void              GetDefaultInputParams(MapNode &);

  protected:

    virtual void             Execute(vtkDataSet*, const int){;};
};

#endif

