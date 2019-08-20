// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDistanceFromBoundaryQuery.h                      //
// ************************************************************************* //

#ifndef AVT_DISTANCE_FROM_BOUNDARY_QUERY_H
#define AVT_DISTANCE_FROM_BOUNDARY_QUERY_H

#include <query_exports.h>

#include <avtLineScanQuery.h>


// ****************************************************************************
//  Class: avtDistanceFromBoundaryQuery
//
//  Purpose:
//    A query that calculates a probability density function of where the
//    mass lies.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

class QUERY_API avtDistanceFromBoundaryQuery : public avtLineScanQuery
{
  public:
                              avtDistanceFromBoundaryQuery();
    virtual                  ~avtDistanceFromBoundaryQuery();

    virtual const char       *GetType(void) 
                                 { return "avtDistanceFromBoundaryQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating mass as a function of distance from the boundary."; };

  protected:
    double                   *mass;

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual void              ExecuteLineScan(vtkPolyData *);

            void              WalkLine(int startPtId, int endPtId, vtkPolyData *output, 
                                       vtkIntArray *lineids, int lineid, vtkDataArray *arr);
};


#endif


