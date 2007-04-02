// ************************************************************************* //
//                              avtVMetricArea.h                             //
// ************************************************************************* //

#ifndef AVT_VMETRIC_AREA_H
#define AVT_VMETRIC_AREA_H

#include <expression_exports.h>

#include <avtVerdictFilter.h>


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
// ****************************************************************************

class EXPRESSION_API avtVMetricArea : public avtVerdictFilter
{
  public:
    virtual double     Metric(double coords[][3], int type);

    virtual bool       OperateDirectlyOnMesh(vtkDataSet *);
    virtual void       MetricForWholeMesh(vtkDataSet *, vtkDataArray *);
};

#endif


