/*=========================================================================

  Program:   ParaView
  Module:    vtkOSPRayPVLODVolume.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOSPRayPVLODVolume.h"

#include "vtkDataSet.h"
#include "vtkImageData.h"
#include "vtkLODProp3D.h"
#include "vtkMapper.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkTransform.h"
#include "vtkUnstructuredGridBase.h"
#include "vtkUnstructuredGridVolumeMapper.h"
#include "vtkVolumeMapper.h"
#include "vtkVolumeProperty.h"

#include <math.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkOSPRayPVLODVolume);

//----------------------------------------------------------------------------
vtkOSPRayPVLODVolume::vtkOSPRayPVLODVolume()
{
  this->LODProp = vtkLODProp3D::New();
  this->LODProp->AutomaticLODSelectionOff();
  this->LODProp->AutomaticPickLODSelectionOff();

  this->HighLODId = -1;
  this->LowLODId = -1;

  this->MapperBounds[0] = this->MapperBounds[1] = this->MapperBounds[2] = 0;
  this->MapperBounds[3] = this->MapperBounds[4] = this->MapperBounds[5] = 0;

  this->EnableLOD = 0;
}

//----------------------------------------------------------------------------
vtkOSPRayPVLODVolume::~vtkOSPRayPVLODVolume()
{
  this->LODProp->Delete();
}

//----------------------------------------------------------------------------
int vtkOSPRayPVLODVolume::SelectLOD()
{
  if (this->LowLODId < 0)
    {
    return this->HighLODId;
    }
  if (this->HighLODId < 0)
    {
    return this->LowLODId;
    }

  if (this->EnableLOD)
    {
    return this->LowLODId;
    }

  return this->HighLODId;
}


//-----------------------------------------------------------------------------
bool vtkOSPRayPVLODVolume::CanRender()
{
  int lodid =this->LODProp->GetSelectedLODID();
  if (lodid >= 0)
    {
    vtkAbstractMapper3D* mapper = this->LODProp->GetLODMapper(lodid);
    if (vtkVolumeMapper* imageVolumeMapper = vtkVolumeMapper::SafeDownCast(mapper))
      {
      int unused = 0;
      vtkDataSet* input = imageVolumeMapper->GetInput();
      vtkDataArray* scalars = input == NULL? NULL :
        vtkAbstractMapper::GetScalars(input,
          imageVolumeMapper->GetScalarMode(),
          imageVolumeMapper->GetArrayAccessMode(),
          imageVolumeMapper->GetArrayId(),
          imageVolumeMapper->GetArrayName(),
          unused);
      return scalars != NULL;
      }
    else if (vtkUnstructuredGridVolumeMapper* ugMapper = vtkUnstructuredGridVolumeMapper::SafeDownCast(mapper))
      {
      int unused = 0;
      vtkDataSet* input = ugMapper->GetInput();
      vtkDataArray* scalars = input == NULL? NULL :
        vtkAbstractMapper::GetScalars(input,
          ugMapper->GetScalarMode(),
          ugMapper->GetArrayAccessMode(),
          ugMapper->GetArrayId(),
          ugMapper->GetArrayName(),
          unused);
      return scalars != NULL;
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
int vtkOSPRayPVLODVolume::RenderOpaqueGeometry(vtkViewport *vp)
{
  if (!this->CanRender()) { return 1; }
  int retval = this->LODProp->RenderOpaqueGeometry(vp);

  this->EstimatedRenderTime = this->LODProp->GetEstimatedRenderTime();

  return retval;
}

//-----------------------------------------------------------------------------
int vtkOSPRayPVLODVolume::RenderTranslucentPolygonalGeometry(vtkViewport *vp)
{
  if (!this->CanRender()) { return 1; }
  //std::cout << __PRETTY_FUNCTION__ << " 1" << std::endl;
  int retval = this->LODProp->RenderTranslucentPolygonalGeometry(vp);

  this->EstimatedRenderTime = this->LODProp->GetEstimatedRenderTime();

  return retval;
}


//-----------------------------------------------------------------------------
int vtkOSPRayPVLODVolume::RenderVolumetricGeometry(vtkViewport *vp)
{
  int retval = this->LODProp->RenderVolumetricGeometry(vp);

  this->EstimatedRenderTime = this->LODProp->GetEstimatedRenderTime();

  return retval;
}

//-----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::ReleaseGraphicsResources(vtkWindow *renWin)
{
  this->Superclass::ReleaseGraphicsResources(renWin);
  this->LODProp->ReleaseGraphicsResources(renWin);
}


//-----------------------------------------------------------------------------
// Get the bounds for this Actor as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
double *vtkOSPRayPVLODVolume::GetBounds()
{
  int i,n;
  double *bounds, bbox[24], *fptr;

  int lod = this->SelectLOD();
  if (lod < 0)
    {
    return this->Bounds;
    }

  vtkAbstractMapper3D *mapper = this->LODProp->GetLODMapper(lod);

  vtkDebugMacro( << "Getting Bounds" );

  // get the bounds of the Mapper if we have one
  if (!mapper)
    {
    return this->Bounds;
    }

  bounds = mapper->GetBounds();
  // Check for the special case when the mapper's bounds are unknown
  if (!bounds)
    {
    return bounds;
    }

  // Check for the special case when the actor is empty.
  if (bounds[0] > bounds[1])
    {
    memcpy( this->MapperBounds, bounds, 6*sizeof(double) );
    vtkMath::UninitializeBounds(this->Bounds);
    this->BoundsMTime.Modified();
    return this->Bounds;
    }

  // Check if we have cached values for these bounds - we cache the
  // values returned by this->Mapper->GetBounds() and we store the time
  // of caching. If the values returned this time are different, or
  // the modified time of this class is newer than the cached time,
  // then we need to rebuild.
  if ( ( memcmp( this->MapperBounds, bounds, 6*sizeof(double) ) != 0 ) ||
       ( this->GetMTime() > this->BoundsMTime ) )
    {
    vtkDebugMacro( << "Recomputing bounds..." );

    memcpy( this->MapperBounds, bounds, 6*sizeof(double) );

    // fill out vertices of a bounding box
    bbox[ 0] = bounds[1]; bbox[ 1] = bounds[3]; bbox[ 2] = bounds[5];
    bbox[ 3] = bounds[1]; bbox[ 4] = bounds[2]; bbox[ 5] = bounds[5];
    bbox[ 6] = bounds[0]; bbox[ 7] = bounds[2]; bbox[ 8] = bounds[5];
    bbox[ 9] = bounds[0]; bbox[10] = bounds[3]; bbox[11] = bounds[5];
    bbox[12] = bounds[1]; bbox[13] = bounds[3]; bbox[14] = bounds[4];
    bbox[15] = bounds[1]; bbox[16] = bounds[2]; bbox[17] = bounds[4];
    bbox[18] = bounds[0]; bbox[19] = bounds[2]; bbox[20] = bounds[4];
    bbox[21] = bounds[0]; bbox[22] = bounds[3]; bbox[23] = bounds[4];

    // save the old transform
    this->Transform->Push();
    this->Transform->SetMatrix(this->GetMatrix());

    // and transform into actors coordinates
    fptr = bbox;
    for (n = 0; n < 8; n++)
      {
      this->Transform->TransformPoint(fptr,fptr);
      fptr += 3;
      }

    this->Transform->Pop();

    // now calc the new bounds
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = VTK_DOUBLE_MAX;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -VTK_DOUBLE_MAX;
    for (i = 0; i < 8; i++)
      {
      for (n = 0; n < 3; n++)
        {
        if (bbox[i*3+n] < this->Bounds[n*2])
          {
          this->Bounds[n*2] = bbox[i*3+n];
          }
        if (bbox[i*3+n] > this->Bounds[n*2+1])
          {
          this->Bounds[n*2+1] = bbox[i*3+n];
          }
        }
      }
    this->BoundsMTime.Modified();
    }

  return this->Bounds;
}


//-----------------------------------------------------------------------------
int vtkOSPRayPVLODVolume::HasTranslucentPolygonalGeometry()
{
  if (this->SelectLOD() == this->LowLODId)
    {
    return 1;
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::ShallowCopy(vtkProp *prop)
{
  vtkOSPRayPVLODVolume *a = vtkOSPRayPVLODVolume::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->LODProp->ShallowCopy(a->LODProp);
    }

  // Now do superclass
  this->vtkVolume::ShallowCopy(prop);
}


//-----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::SetProperty(vtkVolumeProperty *property)
{
  this->Superclass::SetProperty(property);

  this->UpdateLODProperty();
}


//-----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::SetMapper(vtkAbstractVolumeMapper *mapper)
{
  if (this->HighLODId >= 0)
    {
    if (mapper == this->LODProp->GetLODMapper(this->HighLODId)) return;
    this->LODProp->RemoveLOD(this->HighLODId);
    this->HighLODId = -1;
    }

  if (mapper)
    {
    this->HighLODId = this->LODProp->AddLOD(mapper, this->GetProperty(), 1.0);
    this->UpdateLODProperty();
    }
}

//-----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::SetLODMapper(vtkAbstractVolumeMapper *mapper)
{
  if (this->LowLODId >= 0)
    {
    if (mapper == this->LODProp->GetLODMapper(this->LowLODId)) return;
    this->LODProp->RemoveLOD(this->LowLODId);
    this->LowLODId = -1;
    }

  if (mapper)
    {
    this->LowLODId = this->LODProp->AddLOD(mapper, this->GetProperty(), 0.0);
    this->UpdateLODProperty();
    }
}

//-----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::SetLODMapper(vtkMapper *mapper)
{
  if (this->LowLODId >= 0)
    {
    if (mapper == this->LODProp->GetLODMapper(this->LowLODId)) return;
    this->LODProp->RemoveLOD(this->LowLODId);
    this->LowLODId = -1;
    }

  if (mapper)
    {
    vtkProperty *property = vtkProperty::New();
    property->SetOpacity(0.5);
    this->LowLODId = this->LODProp->AddLOD(mapper, property, 0.0);
    property->Delete();
    this->UpdateLODProperty();
    }
}


//-----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::SetAllocatedRenderTime(double t, vtkViewport *v)
{
  this->Superclass::SetAllocatedRenderTime(t, v);

  // This is a good time to update the LODProp.
  this->LODProp->SetUserMatrix(this->GetMatrix());

  int lod = this->SelectLOD();
  if (lod < 0)
    {
    vtkErrorMacro(<< "You must give me a mapper!");
    }
  this->LODProp->SetSelectedLODID(lod);
  this->LODProp->SetSelectedPickLODID(lod);

  this->LODProp->SetAllocatedRenderTime(t, v);
}

//-----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::UpdateLODProperty()
{
  if (!this->Property)
    {
    vtkErrorMacro("No property is defined cannot update.");
    return;
    }

  if (this->HighLODId >= 0)
    {
    this->LODProp->SetLODProperty(this->HighLODId, this->Property);
    }

  if (this->LowLODId >= 0)
    {
    vtkMapper *mapper;
    this->LODProp->GetLODMapper(this->LowLODId, &mapper);
    if (mapper)
      {
      }
    else
      {
      // This is a volume mapper.  Just share the property.
      this->LODProp->SetLODProperty(this->LowLODId, this->Property);
      }
    }
}


//----------------------------------------------------------------------------
void vtkOSPRayPVLODVolume::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "EnableLOD: " << this->EnableLOD << endl;
  os << indent << "LODProp: " << endl;
  this->LODProp->PrintSelf(os, indent.GetNextIndent());
}
