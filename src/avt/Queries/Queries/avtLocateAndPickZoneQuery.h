// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtLocateAndPickZoneQuery.h                        //
// ************************************************************************* //

#ifndef AVT_LOCATEANDPICKZONE_QUERY_H
#define AVT_LOCATEANDPICKZONE_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <PickAttributes.h>

class avtLocateQuery;
class avtPickQuery;
class vtkDataSet;

// ****************************************************************************
//  Class: avtLocateAndPickZoneQuery
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar  1 13:02:19 PST 2011
//    Added SetNumVars.
//
//    Kathleen Biagas, Mon Jun 20 10:37:22 PDT 2011
//    Added SetInputParams, remove SetNumVars, added domain, zone.
//
// ****************************************************************************

class QUERY_API avtLocateAndPickZoneQuery : public avtDatasetQuery
{
  public:
                              avtLocateAndPickZoneQuery();
    virtual                  ~avtLocateAndPickZoneQuery();


    virtual const char       *GetType(void)
                                { return "avtLocateAndPickZoneQuery"; }
    virtual const char       *GetDescription(void)
                                { return "Locating and Picking Zone."; }
    virtual const char       *GetShortDescription(void)
                                { return "Pick Zone"; }

    virtual void              SetInputParams(const MapNode &); 

    virtual int               GetNFilters(void) { return 2; }

    virtual void              PerformQuery(QueryAttributes *);

    void                      SetPickAttsForTimeQuery(const PickAttributes *pa);

    virtual void              SetInvTransform(const avtMatrix *m);
    virtual void              SetNeedTransform(const bool v);

  protected:
    virtual void              Execute(vtkDataSet*, const int){;}

    PickAttributes            pickAtts;

  private:
    void                      SetPickAtts(const PickAttributes *);
    avtLocateQuery           *lcq;
    avtPickQuery             *zpq;
    int                       domain;
    int                       zone;
};


#endif


