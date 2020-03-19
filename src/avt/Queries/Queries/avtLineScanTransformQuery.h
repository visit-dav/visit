// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtLineScanTransformQuery.h                         //
// ************************************************************************* //

#ifndef AVT_LINE_SCAN_TRANSFORM_QUERY_H
#define AVT_LINE_SCAN_TRANSFORM_QUERY_H

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

class QUERY_API avtLineScanTransformQuery : public avtLineScanQuery
{
  public:
                              avtLineScanTransformQuery();
    virtual                  ~avtLineScanTransformQuery();

    virtual const char       *GetType(void) 
                                 { return "avtLineScanTransformQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating the line scan transform."; };

  protected:
    double                   *lengths;
    int                       numLineIntersections;

    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual void              ExecuteLineScan(vtkPolyData *);

//            void              WalkLine(int startPtId, int endPtId, vtkPolyData *output, 
//                                       vtkIntArray *lineids, int lineid, vtkDataArray *arr);
};


#endif


