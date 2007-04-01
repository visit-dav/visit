// ************************************************************************* //
//                         avtPickByNodeQuery.h                              //
// ************************************************************************* //

#ifndef AVT_PICKBYNODE_QUERY_H
#define AVT_PICKBYNODE_QUERY_H
#include <query_exports.h>

#include <avtPickQuery.h>

class vtkDataSet;


// ****************************************************************************
//  Class: avtPickByNodeQuery
//
//  Purpose:
//    This query gathers information about a node picked by the user. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2004 
//
//  Modifications:
//    
// ****************************************************************************

class QUERY_API avtPickByNodeQuery : public avtPickQuery
{
  public:
                                    avtPickByNodeQuery();
    virtual                        ~avtPickByNodeQuery();


    virtual const char             *GetType(void)
                                             { return "avtPickByNodeQuery"; };

    virtual void                    SetTransform(const avtMatrix *m);


  protected:

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    Preparation(void);
};


#endif


