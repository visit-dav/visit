// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vtkVisItViewNodeFactory.h>
#if LIB_VERSION_LE(VTK,8,1,0)
#include <vtkOSPRayPolyDataMapperNode.h>
#else
#include <vtkOSPRayVisItDataSetMapperNode.h>
#endif
#include <vtkOSPRayVisItAxisActorNode.h>
#include <vtkOSPRayVisItCubeAxesActorNode.h>

//============================================================================
#if LIB_VERSION_LE(VTK,8,1,0)
vtkViewNode *vtkVisItViewNodeFactory::pd_maker()
{
  vtkOSPRayPolyDataMapperNode *vn = vtkOSPRayPolyDataMapperNode::New();
  return vn;
}
#else
vtkViewNode *vtkVisItViewNodeFactory::ds_maker()
{
  vtkOSPRayVisItDataSetMapperNode *vn = vtkOSPRayVisItDataSetMapperNode::New();
  return vn;
}
#endif

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
