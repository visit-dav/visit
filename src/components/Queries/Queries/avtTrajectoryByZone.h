// ************************************************************************* //
//                          avtTrajectoryByZone.h                            //
// ************************************************************************* //

#ifndef AVT_TRAJECTORYBYZONE_H
#define AVT_TRAJECTORYBYZONE_H
#include <query_exports.h>

#include <avtVariableByZoneQuery.h>


// ****************************************************************************
//  Class: avtTrajectoryByZone
//
//  Purpose:
//    A time query that retrieves var information about a mesh given a 
//    particular domain and zone number.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 8, 2005 
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtTrajectoryByZone : public avtVariableByZoneQuery
{
  public:
                              avtTrajectoryByZone();
    virtual                  ~avtTrajectoryByZone();


    virtual const char       *GetType(void)   { return "avtTrajectoryByZone"; };
    virtual const char       *GetDescription(void)
                                 { return "Retrieving var information on mesh."; };

    virtual void              GetTimeCurveSpecs(bool &timeForX, int &nRes)
                                 { timeForX = false; nRes = 2; }
  protected:
    virtual void                    Preparation(const avtDataAttributes &);
    virtual void                    PostExecute(void);
};


#endif
