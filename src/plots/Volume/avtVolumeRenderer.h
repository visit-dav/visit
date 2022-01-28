// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtVolumeRenderer.h                           //
// ************************************************************************* //

#ifndef AVT_VOLUME_RENDERER_H
#define AVT_VOLUME_RENDERER_H

#include <avtCustomRenderer.h>
#include <VolumeAttributes.h>

#include <string>

class vtkColorTransferFunction;
class vtkImageData;
class vtkPiecewiseFunction;
class vtkVolume;
class vtkVolumeMapper;
class vtkVolumeProperty;

// ****************************************************************************
//  Class: avtVolumeRenderer
//
//  Purpose:
//      An implementation of an renderer for a volume plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 27, 2001
//
//  Modifications:
//
// ****************************************************************************

class avtVolumeRenderer : public avtCustomRenderer
{
  public:
                            avtVolumeRenderer();
    virtual                ~avtVolumeRenderer();
    static avtVolumeRenderer *New(void);

    void                    SetAtts(const AttributeGroup*);

    virtual bool            OperatesOnScalars(void) { return true; };
    virtual void            Render(vtkDataSet *);

  protected:
    VolumeAttributes           m_atts;
    VolumeAttributes           m_oldAtts;

    vtkColorTransferFunction  *m_transFunc    {nullptr};
    vtkPiecewiseFunction      *m_opacity      {nullptr};
    vtkImageData              *m_imageToRender{nullptr};
    vtkVolumeProperty         *m_volumeProp   {nullptr};
    vtkVolumeMapper           *m_volumeMapper {nullptr};
    vtkVolume                 *m_curVolume    {nullptr};

    bool                       m_useInterpolation{true};
    bool                       m_resetColorMap{true};

    bool                       m_OSPRayEnabled{false};
    int                        m_OSPRayRenderType{0};

    int                        m_nComponents{1};
    int                        m_cellData{0};

    bool                       m_useColorVarMin{false};
    float                      m_colorVarMin{0.0};
    bool                       m_useColorVarMax{false};
    float                      m_colorVarMax{1.0};
    bool                       m_useOpacityVarMin{false};
    float                      m_opacityVarMin{0.0};
    bool                       m_useOpacityVarMax{false};
    float                      m_opacityVarMax{1.0};

    std::string                m_activeVarName  {"default"};
    std::string                m_opacityVarName {"default"};
};

typedef ref_ptr<avtVolumeRenderer> avtVolumeRenderer_p;

#endif
