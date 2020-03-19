// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                 avtAggregateRayLengthDistributionQuery.h                  //
// ************************************************************************* //

#ifndef AVT_AGGREGATE_RAY_LENGTH_DISTRIBUTION_QUERY_H
#define AVT_AGGREGATE_RAY_LENGTH_DISTRIBUTION_QUERY_H

#include <query_exports.h>

#include <avtLineScanQuery.h>


// ****************************************************************************
//  Class: avtAggregateRayLengthDistributionQuery
//
//  Purpose:
//    A query that calculates a probability density function of how much mass
//    a particle at a random location and direction inside a shape will 
//    encounter moving along that direction.  In this case, the particle
//    may enter and exit the shape multiple times.  Mass 
//    is defined as linear mass.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

class QUERY_API avtAggregateRayLengthDistributionQuery : public avtLineScanQuery
{
  public:
                              avtAggregateRayLengthDistributionQuery();
    virtual                  ~avtAggregateRayLengthDistributionQuery();

    virtual const char       *GetType(void) 
                                 { return "avtAggregateRayLengthDistributionQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating probability distribution of mass."; };

  protected:
    double                   *count;

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual void              ExecuteLineScan(vtkPolyData *);

    void                      WalkLine(int startPtId, int endPtId, 
                                     vtkPolyData *output, vtkIntArray *lineids, 
                                     int lineid, vtkDataArray *arr, 
                                     double additionalMass);
    double                    GetMass(int startPtId, int endPtId, 
                                     vtkPolyData *output, vtkIntArray *lineids, 
                                     int lineid, vtkDataArray *arr);
};


#endif


