// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtConnComponentsQuery.h                       //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_QUERY_H
#define AVT_CONN_COMPONENTS_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

class vtkDataSet;
class avtConnComponentsExpression;

// ****************************************************************************
//  Class: avtConnComponentsQuery
//
//  Purpose:
//      Base class for connected components queries. Uses an instance of 
//      avtConnComponentsExpression to obtain the number of connected 
//      components.
//
//  Programmer: Cyrus Harrison
//  Creation:   February 5, 2007 
//
// ****************************************************************************

class QUERY_API avtConnComponentsQuery : public avtDatasetQuery
{
  public:
                                    avtConnComponentsQuery();
    virtual                        ~avtConnComponentsQuery();

    virtual const char             *GetType(void)
                                        { return "avtConnComponentsQuery"; };
    virtual const char             *GetDescription(void)
                                  { return "Finding number of components"; };

  protected:

    avtConnComponentsExpression    *cclFilter;

    int                             nComps;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
};


#endif


