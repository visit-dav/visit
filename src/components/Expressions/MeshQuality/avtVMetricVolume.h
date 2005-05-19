// ************************************************************************* //
//                          avtVMetricVolume.h                               //
// ************************************************************************* //

#ifndef AVT_VMETRIC_VOLUME_H
#define AVT_VMETRIC_VOLUME_H

#include <expression_exports.h>
#include <avtVerdictFilter.h>


// ****************************************************************************
//  Class: avtVMetricVolume
//
//  Purpose:
//    This metric measures volume.
//
//  Programmer: Akira Haddox
//  Creation:   June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Aug 31 12:25:02 PDT 2002
//    Added ability to only consider absolute values of volumes.
//
//    Hank Childs, Thu May 19 10:55:30 PDT 2005
//    Added support for operating on rectilinear meshes directly.
//
// ****************************************************************************

class EXPRESSION_API avtVMetricVolume : public avtVerdictFilter
{
  public:
                       avtVMetricVolume();

    virtual double     Metric(double coords[][3], int type);

    void               UseOnlyPositiveVolumes(bool val)
                                  { useOnlyPositiveVolumes = val; };

    virtual bool       OperateDirectlyOnMesh(vtkDataSet *);
    virtual void       MetricForWholeMesh(vtkDataSet *, vtkDataArray *);

  protected:
    bool               useOnlyPositiveVolumes;
};


#endif


