// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkAnariVisItViewNodeFactory.h"
#include "vtkAnariVisItAxisActorNode.h"
#include "vtkAnariVisItCubeAxesActorNode.h"

#include <vtkAnariPolyDataMapperNode.h>

//============================================================================
vtkViewNode *vtkAnariVisItViewNodeFactory::pd_maker()
{
  vtkAnariPolyDataMapperNode *vn = vtkAnariPolyDataMapperNode::New();
  return vn;
}

//-----------------------------------------------------------------------------
vtkViewNode *vtkAnariVisItViewNodeFactory::cube_axes_act_maker()
{
  vtkAnariVisItCubeAxesActorNode *vn = vtkAnariVisItCubeAxesActorNode::New();
  return vn;
}

//-----------------------------------------------------------------------------
vtkViewNode *vtkAnariVisItViewNodeFactory::axis_act_maker()
{
  vtkAnariVisItAxisActorNode *vn = vtkAnariVisItAxisActorNode::New();
  return vn;
}
