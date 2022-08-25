// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_VISIT_VIEWNODEFACTORY_H
#define VTK_VISIT_VIEWNODEFACTORY_H

#include <plotter_exports.h>

#include <visit-config.h> // For LIB_VERSION_LE

class vtkViewNode;

//============================================================================
namespace vtkVisItViewNodeFactory
{
#if LIB_VERSION_LE(VTK,8,1,0)
vtkViewNode PLOTTER_API *pd_maker();
#else
vtkViewNode PLOTTER_API *ds_maker();
#endif
vtkViewNode PLOTTER_API *cube_axes_act_maker();
vtkViewNode PLOTTER_API *axis_act_maker();
}

#endif
