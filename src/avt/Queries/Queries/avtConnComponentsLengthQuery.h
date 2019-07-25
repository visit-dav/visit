// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtConnComponentsLengthQuery.h                         //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_LENGTH_QUERY_H
#define AVT_CONN_COMPONENTS_LENGTH_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <avtConnComponentsQuery.h>

#include <vector>

class avtEdgeLength;
class vtkDataSet;


// ****************************************************************************
//  Class: avtConnComponentsLengthQuery
//
//  Purpose:
//      Obtains the length of each connected component.
//      Requires a 1D (lines) dataset.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Jun 15 13:16:47 PDT 2011 
//
// ****************************************************************************

class QUERY_API avtConnComponentsLengthQuery : public avtConnComponentsQuery
{
  public:
                                    avtConnComponentsLengthQuery();
    virtual                        ~avtConnComponentsLengthQuery();

    virtual const char             *GetType(void)
                                  { return "avtConnComponentsLengthQuery"; };
    virtual const char             *GetDescription(void)
                                  { return "Finding per component length."; };

  protected:

    avtEdgeLength                  *lengthFilter;
    std::vector<double>             lengthPerComp;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);
    virtual void                    VerifyInput(void);

};


#endif



