// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtMassDistributionQuery.h                       //
// ************************************************************************* //

#ifndef AVT_MASS_DISTRIBUTION_QUERY_H
#define AVT_MASS_DISTRIBUTION_QUERY_H

#include <query_exports.h>

#include <avtLineScanQuery.h>


// ****************************************************************************
//  Class: avtMassDistributionQuery
//
//  Purpose:
//    A query that calculates a probability density function of where the
//    mass lies.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

class QUERY_API avtMassDistributionQuery : public avtLineScanQuery
{
  public:
                              avtMassDistributionQuery();
    virtual                  ~avtMassDistributionQuery();

    virtual const char       *GetType(void) 
                                 { return "avtMassDistributionQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating mass distribution."; };

  protected:
    double                   *mass;

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual void              ExecuteLineScan(vtkPolyData *);
};


#endif


