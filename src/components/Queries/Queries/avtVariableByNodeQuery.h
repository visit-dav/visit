// ************************************************************************* //
//                        avtVariableByNodeQuery.h                           //
// ************************************************************************* //

#ifndef AVT_VARIABLEBYNODE_QUERY_H
#define AVT_VARIABLEBYNODE_QUERY_H
#include <query_exports.h>

#include <avtPickByNodeQuery.h>

#include <PickAttributes.h>


class vtkDataSet;


// ****************************************************************************
//  Class: avtVariableByNodeQuery
//
//  Purpose:
//    A query that retrieves var information about a mesh given a 
//    particular domain and node number.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 29, 2004
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtVariableByNodeQuery : public avtPickByNodeQuery
{
  public:
                              avtVariableByNodeQuery();
    virtual                  ~avtVariableByNodeQuery();


    virtual const char       *GetType(void)   { return "avtVariableByNodeQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Retrieving var information on mesh."; };

  protected:
    virtual void                    Preparation(void); 
    virtual void                    PostExecute(void);
};


#endif
