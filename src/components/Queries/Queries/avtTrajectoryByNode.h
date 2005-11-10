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
//
// ****************************************************************************

class QUERY_API avtTrajectoryByNode : public avtVariableByNodeQuery
{
  public:
                              avtTrajectoryByNode();
    virtual                  ~avtTrajectoryByNode();


    virtual const char       *GetType(void)   { return "avtTrajectoryByNode"; };
    virtual const char       *GetDescription(void)
                                 { return "Retrieving var information on mesh."; };

    virtual void              GetTimeCurveSpecs(bool &timeForX, int &nRes)
                                 { timeForX = false; nRes = 2; }
  protected:
    virtual void                    Preparation(const avtDataAttributes &); 
    virtual void                    PostExecute(void);
};


#endif
