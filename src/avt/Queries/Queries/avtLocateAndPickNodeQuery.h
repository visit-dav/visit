// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtLocateAndPickNodeQuery.h                        //
// ************************************************************************* //

#ifndef AVT_LOCATEANDPICKNODE_QUERY_H
#define AVT_LOCATEANDPICKNODE_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <PickAttributes.h>

class avtLocateQuery;
class avtPickQuery;
class vtkDataSet;

// ****************************************************************************
//  Class: avtLocateAndPickNodeQuery
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 22, 2007 
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 17 09:55:14 PST 2011
//    Added SetNumVars.
//
//    Kathleen Biagas, Mon Jun 20 10:41:02 PDT 2011
//    Added SetInputParams, removed SetNumVars, added domain, node.
//
// ****************************************************************************

class QUERY_API avtLocateAndPickNodeQuery : public avtDatasetQuery
{
  public:
                              avtLocateAndPickNodeQuery();
    virtual                  ~avtLocateAndPickNodeQuery();


    virtual const char       *GetType(void)
                                { return "avtLocateAndPickNodeQuery"; }
    virtual const char       *GetDescription(void)
                                { return "Locating and Picking Node."; }
    virtual const char       *GetShortDescription(void)
                                { return "Pick Node"; }

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
    avtLocateQuery           *lnq;
    avtPickQuery             *npq;
    int                       domain;
    int                       node;
};


#endif


