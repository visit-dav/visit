// ************************************************************************* //
//                            avtVariableQuery.h                             //
// ************************************************************************* //

#ifndef AVT_VARIABLE_QUERY_H
#define AVT_VARIABLE_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <PickAttributes.h>


class vtkDataSet;
class avtExpressionEvaluatorFilter;


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
//    Kathleen Bonnell, Thu Apr  1 19:06:07 PST 2004
//    Added VerifyInput, RetriveVarInfo, RetrieveNodes, RetrieveZones, eef and
//    src.
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

    virtual bool              OriginalData(void) { return true; };

  protected:

    virtual void                    VerifyInput(void);
    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);   

    void                            RetrieveVarInfo(vtkDataSet *);   
    bool                            RetrieveNodes(vtkDataSet *, int);   
    bool                            RetrieveZones(vtkDataSet *, int);   

    PickAttributes                  pickAtts;
    int                             searchDomain;
    int                             searchElement;
    avtExpressionEvaluatorFilter   *eef;
    avtQueryableSource             *src;
};


#endif
