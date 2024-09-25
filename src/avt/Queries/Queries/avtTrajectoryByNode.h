// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtTrajectoryByNode.h                          //
// ************************************************************************* //

#ifndef AVT_TRAJECTORYBYNODE_H
#define AVT_TRAJECTORYBYNODE_H
#include <query_exports.h>

#include <avtVariableByNodeQuery.h>


// ****************************************************************************
//  Class: avtTrajectoryByNode
//
//  Purpose:
//    A time query that retrieves var information about a mesh given a
//    particular domain and node number.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 8, 2005
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul  8 15:43:15 PDT 2008
//    Changed GetTimeCurveSpecs signature.
//
//    Kathleen Biagas, Tue Jul 26 10:03:11 PDT 2011
//    Add GetDefaultInputParams.
//
//    Kathleen Biagas, Wed Sep 11, 2024
//    Add QueryAttributes argument to GetTimeCurveSpecs.
//    Addd GetShortDescription.
//
// ****************************************************************************

class QUERY_API avtTrajectoryByNode : public avtVariableByNodeQuery
{
  public:
                              avtTrajectoryByNode();
    virtual                  ~avtTrajectoryByNode();


    virtual const char       *GetType(void)   { return "avtTrajectoryByNode"; }
    virtual const char       *GetDescription(void)
                               { return "Retrieving var information on mesh.";}
    virtual const char       *GetShortDescription(void)
                              { return "Trajectory"; }


    const MapNode    &GetTimeCurveSpecs(const QueryAttributes *) override;

    static void               GetDefaultInputParams(MapNode &);

  protected:
    virtual void              Preparation(const avtDataAttributes &);
    virtual void              PostExecute(void);
};


#endif
