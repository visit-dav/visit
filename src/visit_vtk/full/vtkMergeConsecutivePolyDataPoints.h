// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    vtkMergeConsecutivePolyDataPoints.h                    //
// ************************************************************************* //

#ifndef VTK_MERGE_CONSECUTIVE_POLY_DATA_POINTS_H
#define VTK_MERGE_CONSECUTIVE_POLY_DATA_POINTS_H
#include <visit_vtk_exports.h>

#include "vtkPolyDataAlgorithm.h"

// ***************************************************************************
//  Class: vtkMergeConsecutivePolyDataPoints
//
//  Purpose:
//    Merge consecutive points within poly data primitives.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Oct 15 15:23:20 PDT 2026
//
//  Modifications:
//
// ***************************************************************************

class VISIT_VTK_API vtkMergeConsecutivePolyDataPoints : public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkMergeConsecutivePolyDataPoints, vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent) override;

    static vtkMergeConsecutivePolyDataPoints *New();

  protected:
    vtkMergeConsecutivePolyDataPoints();
    ~vtkMergeConsecutivePolyDataPoints() {};

    virtual int RequestData(vtkInformation *,
                            vtkInformationVector **,
                            vtkInformationVector *) override;
    virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  private:
    vtkMergeConsecutivePolyDataPoints(const vtkMergeConsecutivePolyDataPoints&)=delete;
    void operator=(const vtkMergeConsecutivePolyDataPoints&)=delete;
};

#endif
