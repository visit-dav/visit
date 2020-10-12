// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtDefaultRenderer.h                         //
// ************************************************************************* //

#ifndef AVT_DEFAULT_RENDERER_H
#define AVT_DEFAULT_RENDERER_H

#include <avtVolumeRendererImplementation.h>
#include <VolumeAttributes.h>

#include <vtkVolume.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkGPUVolumeRayCastMapper.h>

// ****************************************************************************
//  Class: avtDefaultRenderer
//
//  Purpose:
//      A default volume renderer using vtkSmartVolumeMapper. The mapper checks
//      for hardware compatability and rendering parameters to choose which 
//      rendering method to use. 
//
//  Programmer: Alister Maguire
//  Creation:   April  3, 2017
//
//  Modifications:
//  
//    Alister Maguire, Tue Dec 11 10:18:31 PST 2018
//    Changed pointers to standard instead of smart. 
//    Added transfer function and opacity. 
//
// ****************************************************************************

class avtDefaultRenderer : public avtVolumeRendererImplementation
{
  public:
                               avtDefaultRenderer();
    virtual                   ~avtDefaultRenderer();

  protected:
    virtual void               Render(const RenderProperties &props,
                                      const VolumeData &volume);

    vtkColorTransferFunction  *transFunc;
    vtkPiecewiseFunction      *opacity;
    vtkVolume                 *curVolume;
    vtkImageData              *imageToRender;
    vtkVolumeProperty         *volumeProp;
    //vtkSmartVolumeMapper      *mapper;
    vtkGPUVolumeRayCastMapper      *mapper;

    VolumeAttributes           oldAtts;

    bool                       resetColorMap;
    bool                       useInterpolation;
};

#endif 
