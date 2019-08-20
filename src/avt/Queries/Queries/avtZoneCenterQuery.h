// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtZoneCenterQuery.h                             //
// ************************************************************************* //

#ifndef AVT_ZONECZENTER_QUERY_H
#define AVT_ZONECZENTER_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>


// ****************************************************************************
//  Class: avtZoneCenterQuery
//
//  Purpose:
//      This is a dataset query that returns the coordintes of a zone's center. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 25, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Dec 16 17:16:33 PST 2004
//    Add FindLocalCenter, FindGlobalCenter.
//
//    Kathleen Biagas, Tue Jun 21 10:28:07 PDT 2011
//    Add SetInputParams, element, domain, useGlobalId.
//
//    Kathleen Biagas, Fri Jul 15 16:38:31 PDT 2011
//    Add GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtZoneCenterQuery : public avtDatasetQuery
{
  public:
                             avtZoneCenterQuery();
    virtual                 ~avtZoneCenterQuery(); 

    virtual const char      *GetType(void) { return "avtZoneCenterQuery"; };
    virtual const char      *GetDescription(void) 
                                  { return "Getting zone center."; };

    virtual void             SetInputParams(const MapNode &);
    static  void             GetDefaultInputParams(MapNode &);

    virtual void             PerformQuery(QueryAttributes *);
    virtual bool             OriginalData(void) { return true; };

  protected:

    virtual void             Execute(vtkDataSet*, const int){;};

    bool                     FindGlobalCenter(double coord[3]);
    bool                     FindLocalCenter(double coord[3]);

  private:
    int    element;
    int    domain;
    bool   useGlobalId;
};

#endif

