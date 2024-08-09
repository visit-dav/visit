// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtVisItVTKRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_VISIT_VTK_RENDERER_H
#define AVT_VISIT_VTK_RENDERER_H

#include <VolumeAttributes.h>

class vtkDataSet;
class vtkRenderer;
class vtkColorTransferFunction;
class vtkImageData;
class vtkPiecewiseFunction;
class vtkVolume;
class vtkVolumeMapper;
class vtkVolumeProperty;

class avtVisItVTKRenderer
{
public:
                   avtVisItVTKRenderer();
    virtual       ~avtVisItVTKRenderer();

    void           SetAtts(const AttributeGroup*);

protected:

    int            NumberOfComponents(const std::string activeVariable,
                                      const std::string opacityVariable);

    bool           NeedImage();
    void           UpdateRenderingState(vtkDataSet * in_ds,
                                        vtkRenderer* renderer);

    VolumeAttributes           m_atts;

    // For state changes.
    bool                       m_firstPass{true};

    bool                       m_needImage{false};

    bool                       m_useColorVarMin{false};
    float                      m_colorVarMin{+1.0};
    bool                       m_useColorVarMax{false};
    float                      m_colorVarMax{-1.0};

    std::string                m_opacityVarName{"default"};
    bool                       m_useOpacityVarMin{false};
    float                      m_opacityVarMin{+1.0};
    bool                       m_useOpacityVarMax{false};
    float                      m_opacityVarMax{-1.0};

    // For OSPRay state changes.
    bool                       m_OSPRayEnabled{false};
    int                        m_OSPRayRenderType{0};

    // For ANARI state changes.
    bool                       m_AnariEnabled{false};

    // For generating the image.
    int                        m_nComponents{0};
    int                        m_cellData{0};
    bool                       m_useInterpolation{true};

    double                     m_dataRange[2] = {+1.0, -1.0};
    double                     m_opacityRange[2] = {+1.0, -1.0};

    // For the rendering.
    vtkColorTransferFunction  *m_transFunc     {nullptr};
    vtkPiecewiseFunction      *m_opacity       {nullptr};
    vtkImageData              *m_imageToRender {nullptr};
    vtkVolumeProperty         *m_volumeProperty{nullptr};
    vtkVolumeMapper           *m_volumeMapper  {nullptr};
    vtkVolume                 *m_volume        {nullptr};
};

#endif
