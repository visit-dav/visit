// ************************************************************************* //
//                            avtVariableQuery.h                             //
// ************************************************************************* //

#ifndef AVT_DOMAINZONE_QUERY_H
#define AVT_DOMAINZONE_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <PickAttributes.h>


class vtkDataSet;


// ****************************************************************************
//  Class: avtVariableQuery
//
//  Purpose:
//    A query that retrieves var information about a mesh given a 
//    particular domain and zone number.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtVariableQuery : public avtDatasetQuery
{
  public:
                              avtVariableQuery();
    virtual                  ~avtVariableQuery();


    virtual const char       *GetType(void)   { return "avtVariableQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Retrieving var information on mesh."; };

  protected:

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);   

    PickAttributes                  pickAtts;
    int                             searchDomain;
    int                             searchElement;
};


#endif
