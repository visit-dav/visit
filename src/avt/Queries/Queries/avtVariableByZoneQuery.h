// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtVariableByZoneQuery.h                           //
// ************************************************************************* //

#ifndef AVT_VARIABLEBYZONE_QUERY_H
#define AVT_VARIABLEBYZONE_QUERY_H
#include <query_exports.h>

#include <avtPickByZoneQuery.h>

#include <PickAttributes.h>


class vtkDataSet;


// ****************************************************************************
//  Class: avtVariableByZoneQuery
//
//  Purpose:
//    A query that retrieves var information about a mesh given a
//    particular domain and zone number.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 29, 2004
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDataAttributes arg to Preparation.
//
//    Kathleen Bonnell, Tue Jul  8 15:40:45 PDT 2008
//    Added GetTimeCurveSpecs method.
//
//    Kathleen Bonnell, Tue Mar  1 16:07:47 PST 2011
//    Added SetNumVars method.
//
//    Kathleen Biagas, Mon Jun 20 10:31:43 PDT 2011
//    Added SetInputParams, removed SetNumVars, added domain, zone.
//
//    Kathleen Biagas, Wed Sep 11, 2024
//    Added QueryAttributes argument to GetTimeCurveSpecs.
//
// ****************************************************************************

class QUERY_API avtVariableByZoneQuery : public avtPickByZoneQuery
{
  public:
                              avtVariableByZoneQuery();
    virtual                  ~avtVariableByZoneQuery();


    virtual const char       *GetType(void)
                              { return "avtVariableByZoneQuery"; }
    virtual const char       *GetDescription(void)
                              { return "Retrieving var information on mesh."; }

    virtual void              SetInputParams(const MapNode &);

    const MapNode            &GetTimeCurveSpecs(const QueryAttributes *) override;

  protected:
    virtual void                    Preparation(const avtDataAttributes &);
    virtual void                    PostExecute(void);

    int domain;
    int zone;
};


#endif
