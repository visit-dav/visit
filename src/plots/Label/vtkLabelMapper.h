// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           vtkLabelMapper.h                                //
// ************************************************************************* //

#ifndef VTK_LABEL_MAPPER_H
#define VTK_LABEL_MAPPER_H

#include <vtkLabelMapperBase.h>
#include <vtkSmartPointer.h>

#include <LabelAttributes.h>


class vtkRenderer;

// ****************************************************************************
// Class: vtkLabelMapper
//
// Purpose:  vtk mapper for labels
//
//
// Programmer: Kathleen Biagas
// Creation:   April 13, 2017
//
// Modifications:
//   Alister Maguire, Mon May 24 10:06:23 PDT 2021
//   Added GetPositionScale.
//
//   Kathleen Biagas, Tue Aug 4, 2026
//   Removed zBufferWarningIssued.
//
//   Eric Brugger, Mon Sep 21 08:41:16 PDT 2026
//   Replaced pointXForm with pointXFormImage and pointXFormTile to properly
//   access the z-buffer with tiled rendering.
//
// ****************************************************************************

class vtkLabelMapper : public vtkLabelMapperBase
{
public:
    static vtkLabelMapper *New();
    vtkTypeMacro(vtkLabelMapper, vtkLabelMapperBase);

    void ReleaseGraphicsResources(vtkWindow *) override;

protected:
    vtkLabelMapper();
   ~vtkLabelMapper();

    vtkTimeStamp BuildTime;

    void BuildLabelsInternal(vtkDataSet*, vtkRenderer*) override;

    // from openglrenderer
    void DrawLabels2D(vtkDataSet *, vtkRenderer *);
    void DrawAllLabels2D(vtkDataSet *);
    void DrawDynamicallySelectedLabels2D(vtkDataSet *, vtkRenderer *);

    void DrawLabels3D(vtkDataSet *, vtkRenderer*);
    void DrawAllCellLabels3D(vtkDataSet *, vtkRenderer*);
    void DrawAllNodeLabels3D(vtkDataSet *, vtkRenderer*);

    void PopulateBinsWithCellLabels3D(vtkDataSet *, vtkRenderer *);
    void PopulateBinsWithNodeLabels3D(vtkDataSet *, vtkRenderer *);
    void PopulateBinsHelper(vtkRenderer *, const unsigned char *,
                            const char *, const double *, vtkIdType, int,
                            const double *);
    void ClearZBuffer();
    void InitializeZBuffer(vtkDataSet *, vtkRenderer *, bool, bool);

    void GetPositionScale(double *);

    template <typename T>
    double *TransformPoints(T inputPoints,
                           const unsigned char *quantizedNormalIndices,
                           int nPoints,
                           vtkViewport *,
                           double *);

    double                 pointXFormImage[4][4];
    double                 pointXFormTile[4][4];

    int                    zBufferMode;
    float                 *zBuffer;
    int                    zBufferWidth;
    int                    zBufferHeight;
    float                  zTolerance;

private:
    vtkLabelMapper(const vtkLabelMapper&) = delete;
    void operator=(const vtkLabelMapper&) = delete;
};

#endif
