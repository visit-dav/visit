// ************************************************************************* //
//                            avtWatertightQuery.h                           //
// ************************************************************************* //

#ifndef AVT_WATERTIGHT_QUERY_H
#define AVT_WATERTIGHT_QUERY_H

#include <query_exports.h>

#include <avtDatasetQuery.h>

class     avtExternalNodeExpression;


// ****************************************************************************
//  Class: avtWatertightQuery
//
//  Purpose:
//      Queries whether or not a surface is watertight, meaning it forms a
//      closed surface.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2005
//
// ****************************************************************************

class QUERY_API avtWatertightQuery : public avtDatasetQuery
{
  public:
                         avtWatertightQuery();
    virtual             ~avtWatertightQuery();

    virtual const char  *GetType(void)  
                             { return "avtWatertightQuery"; };
    virtual const char  *GetDescription(void)
                             { return "Watertight"; };

  protected:
    avtExternalNodeExpression *external_nodes;
    bool                       foundExternalNode;

    virtual void               VerifyInput(void);
    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    virtual void               Execute(vtkDataSet *, const int);
    virtual void               PreExecute(void);
    virtual void               PostExecute(void);
    virtual int                GetNFilters() { return 1; };

};


#endif


