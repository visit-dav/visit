// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vtkVisItViewNodeFactory.h>
#include <vtkOSPRayVisItDataSetMapperNode.h>
#include <vtkOSPRayVisItAxisActorNode.h>
#include <vtkOSPRayVisItCubeAxesActorNode.h>

//============================================================================
vtkViewNode *vtkVisItViewNodeFactory::ds_maker()
{
  vtkOSPRayVisItDataSetMapperNode *vn = vtkOSPRayVisItDataSetMapperNode::New();
  return vn;
}

//-----------------------------------------------------------------------------
vtkViewNode *vtkVisItViewNodeFactory::cube_axes_act_maker()
{
  vtkOSPRayVisItCubeAxesActorNode *vn = vtkOSPRayVisItCubeAxesActorNode::New();
  return vn;
}

//-----------------------------------------------------------------------------
vtkViewNode *vtkVisItViewNodeFactory::axis_act_maker()
{
  vtkOSPRayVisItAxisActorNode *vn = vtkOSPRayVisItAxisActorNode::New();
  return vn;
}
