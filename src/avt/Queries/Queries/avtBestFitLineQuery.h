// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtBestFitLineQuery.h                            //
// ************************************************************************* //

#ifndef AVT_BEST_FIT_LINE_QUERY_H
#define AVT_BEST_FIT_LINE_QUERY_H

#include <query_exports.h>

#include <avtSummationQuery.h>

// ****************************************************************************
//  Class: avtBestFitLineQuery
//
//  Purpose:
//      A query that will calculate the best fit line to a set of points.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Nov 16 16:21:33 PST 2005
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtBestFitLineQuery : public avtSummationQuery
{
  public:
                         avtBestFitLineQuery();
    virtual             ~avtBestFitLineQuery();

    virtual const char  *GetType(void)  
                             { return "avtBestFitLineQuery"; };

    virtual void PreExecute(void);
    virtual void Execute(vtkDataSet *, const int);
    virtual void PostExecute(void);

  protected:
    double  sums[6];
};


#endif


