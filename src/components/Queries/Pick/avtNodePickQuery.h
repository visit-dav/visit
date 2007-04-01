// ************************************************************************* //
//                            avtNodePickQuery.h                                 //
// ************************************************************************* //

#ifndef AVT_NODEPICK_QUERY_H
#define AVT_NODEPICK_QUERY_H
#include <query_exports.h>

#include <avtPickQuery.h>

#include <string>
#include <PickAttributes.h>
#include <avtTypes.h>

class avtMatrix;
class vtkDataSet;


// ****************************************************************************
//  Class: avtNodePickQuery
//
//  Purpose:
//    This query gathers information about a node picked by the user. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 13, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec  6 14:30:39 PST 2004
//    Added PreExecute and PostExecute methods, minDist, foundNode and
//    foundDomain members.
//    
// ****************************************************************************

class QUERY_API avtNodePickQuery : public avtPickQuery
{
  public:
                                    avtNodePickQuery();
    virtual                        ~avtNodePickQuery();

    virtual const char             *GetType(void)
                                             { return "avtNodePickQuery"; };

    virtual void                    SetInvTransform(const avtMatrix *);

  protected:
    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PostExecute(void);
    virtual void                    PreExecute(void);
    virtual void                    Preparation(void);
    int                             DeterminePickedNode(vtkDataSet *);

  private:
    float                           minDist;
    int                             foundNode;
    int                             foundDomain;
};


#endif


