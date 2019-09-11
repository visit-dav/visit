// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __VTK_OFFSCREEN_RENDERING_FACTORY_H
#define __VTK_OFFSCREEN_RENDERING_FACTORY_H

#include <vtkObjectFactory.h>
#include <vtkVersion.h>
#include <vtkObjectFactoryCollection.h>
#include "visit_vtk_offscreen_exports.h"

class VISIT_VTK_OFFSCREEN_API vtkOffScreenRenderingFactory : public vtkObjectFactory
{
public:
  vtkOffScreenRenderingFactory();
  static void ForceOffScreen();
  static vtkOffScreenRenderingFactory* New() { return new vtkOffScreenRenderingFactory;}
  virtual const char* GetVTKSourceVersion() { return VTK_SOURCE_VERSION; }
  const char* GetDescription() { return "vtkOffScreenRenderingFactory"; }
};

#endif

