// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFlattenQuery.h                              //
// ************************************************************************* //

#ifndef AVT_FLATTEN_QUERY_H
#define AVT_FLATTEN_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

class vtkDataset;

// ****************************************************************************
//  Class: avtFlattenQuery
//
//  Purpose:
//    A query that provides flattened field data
//
//  Programmer: Chris Laganella
//  Creation:   Tue Jan 11 17:21:22 EST 2022
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtFlattenQuery : public avtDatasetQuery
{
  public:
                              avtFlattenQuery();
    virtual                  ~avtFlattenQuery();


    virtual const char       *GetType(void)   { return "avtFlattenQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Provides flattened field data."; };

  protected:

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    VerifyInput(void);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
};


#endif
