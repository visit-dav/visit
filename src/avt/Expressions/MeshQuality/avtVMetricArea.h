// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtVMetricArea.h                             //
// ************************************************************************* //

#ifndef AVT_VMETRIC_AREA_H
#define AVT_VMETRIC_AREA_H

#include <expression_exports.h>

#include <avtVerdictExpression.h>


// ****************************************************************************
//  Class: avtVMetricArea
//
//  Purpose:
//      This metric measures area.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu May 19 10:55:30 PDT 2005
//    Added support for operating on rectilinear meshes directly.
//
//    Hank Childs, Thu Jul 24 12:48:13 PDT 2008
//    Added support for polygonal areas.
//
// ****************************************************************************

class EXPRESSION_API avtVMetricArea : public avtVerdictExpression
{
  public:
    virtual double     Metric(double coords[][3], int type);

    virtual bool       OperateDirectlyOnMesh(vtkDataSet *);
    virtual void       MetricForWholeMesh(vtkDataSet *, vtkDataArray *);
    virtual bool       SummationValidForOddShapes(void) { return true; };
};

#endif


