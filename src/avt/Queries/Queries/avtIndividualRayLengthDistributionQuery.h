// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                 avtIndividualRayLengthDistributionQuery.h                 //
// ************************************************************************* //

#ifndef AVT_INDIVIDUAL_RAY_LENGTH_DISTRIBUTION_QUERY_H
#define AVT_INDIVIDUAL_RAY_LENGTH_DISTRIBUTION_QUERY_H

#include <query_exports.h>

#include <avtLineScanQuery.h>


// ****************************************************************************
//  Class: avtIndividualRayLengthDistributionQuery
//
//  Purpose:
//    A query that calculates a probability density function of how much mass
//    a particle at a random location and direction inside a shape will 
//    encounter before it exits the shape.  Mass in this case is defined as
//    linear mass.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

class QUERY_API avtIndividualRayLengthDistributionQuery : public avtLineScanQuery
{
  public:
                              avtIndividualRayLengthDistributionQuery();
    virtual                  ~avtIndividualRayLengthDistributionQuery();

    virtual const char       *GetType(void) 
                                 { return "avtIndividualRayLengthDistributionQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating probability distribution of mass."; };

  protected:
    double                   *count;

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual void              ExecuteLineScan(vtkPolyData *);

    void                      WalkLine(int startPtId, int endPtId, vtkPolyData *output, 
                                       vtkIntArray *lineids, int lineid, vtkDataArray *arr);
};


#endif


