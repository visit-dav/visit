// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef vtkAnariVisItViewNodeFactory_h
#define vtkAnariVisItViewNodeFactory_h

#include <plotter_exports.h>

class vtkViewNode;

//============================================================================
namespace vtkAnariVisItViewNodeFactory
{
    vtkViewNode PLOTTER_API *pd_maker();
    vtkViewNode PLOTTER_API *cube_axes_act_maker();
    vtkViewNode PLOTTER_API *axis_act_maker();
}

#endif
