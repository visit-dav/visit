/*=========================================================================

Program:   ParaView
Module:    vtkPVOSPRayImageVolumeRepresentation.cxx

Copyright (c) Kitware, Inc.
All rights reserved.
See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVOSPRayImageVolumeRepresentation.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCommand.h"
#include "vtkExtentTranslator.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOutlineSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkPVCacheKeeper.h"
#include "vtkPVLODVolume.h"
#include "vtkPVRenderView.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkVolumeProperty.h"
#include "vtkAbstractVolumeMapper.h"
#include "vtkPExtentTranslator.h"

#include "vtkOSPRayVolumeRayCastMapper.h"
#include "vtkOSPRayCompositeMapper.h"
#include "vtkOSPRayLODActor.h"
#include "vtkOSPRayActor.h"
#include "vtkOSPRayPolyDataMapper.h"
#include "vtkOSPRayProperty.h"
#include "vtkObjectFactory.h"
#include "vtkVolumeProperty.h"
#include "vtkPiecewiseFunction.h"
#include "vtkOSPRayPVLODVolume.h"

#include <map>
#include <string>



 vtkStandardNewMacro(vtkPVOSPRayImageVolumeRepresentation);
//----------------------------------------------------------------------------
 vtkPVOSPRayImageVolumeRepresentation::vtkPVOSPRayImageVolumeRepresentation()
 {
  this->VolumeMapper = vtkOSPRayVolumeRayCastMapper::New();

  this->Property = vtkVolumeProperty::New();

  this->Actor = vtkOSPRayPVLODVolume::New();
  this->Actor->SetProperty(this->Property);

  this->CacheKeeper = vtkPVCacheKeeper::New();

  this->OutlineSource = vtkOutlineSource::New();
  this->OutlineMapper = vtkPolyDataMapper::New();

  this->Cache = vtkImageData::New();

  this->CacheKeeper->SetInputData(this->Cache);

  vtkMath::UninitializeBounds(this->DataBounds);
  this->DataSize = 0;

  this->ColorArrayName = 0;
  this->ColorAttributeType = POINT_DATA;
}

//----------------------------------------------------------------------------
vtkPVOSPRayImageVolumeRepresentation::~vtkPVOSPRayImageVolumeRepresentation()
{
  this->VolumeMapper->Delete();
  this->Property->Delete();
  this->Actor->Delete();
  this->OutlineSource->Delete();
  this->OutlineMapper->Delete();
  this->CacheKeeper->Delete();

  this->Cache->Delete();
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::FillInputPortInformation(
  int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type,
  vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
  {
    return 0;
  }
  if (request_type == vtkPVView::REQUEST_UPDATE())
  {
    vtkPVRenderView::SetPiece(inInfo, this,
      this->OutlineSource->GetOutputDataObject(0),
      this->DataSize);
    outInfo->Set(vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1);

    vtkPVRenderView::SetGeometryBounds(inInfo, this->DataBounds);

    vtkPVOSPRayImageVolumeRepresentation::PassOrderedCompositingInformation(
      this, inInfo);

    vtkPVRenderView::SetRequiresDistributedRendering(inInfo, this, true);
  }
  else if (request_type == vtkPVView::REQUEST_UPDATE_LOD())
  {
    vtkPVRenderView::SetRequiresDistributedRenderingLOD(inInfo, this, true);
  }
  else if (request_type == vtkPVView::REQUEST_RENDER())
  {
    this->UpdateMapperParameters();

    vtkAlgorithmOutput* producerPort = vtkPVRenderView::GetPieceProducer(inInfo, this);
    if (producerPort)
    {
      this->OutlineMapper->SetInputConnection(producerPort);
    }
  }
  return 1;
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::PassOrderedCompositingInformation(
  vtkPVDataRepresentation* self, vtkInformation* inInfo)
{
  (void)inInfo;
  if (self->GetNumberOfInputConnections(0) == 1)
  {
    vtkAlgorithmOutput* connection = self->GetInputConnection(0, 0);
    vtkAlgorithm* inputAlgo = connection->GetProducer();
    vtkStreamingDemandDrivenPipeline* sddp =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(inputAlgo->GetExecutive());

    int extent[6] = {1, -1, 1, -1, 1, -1};
    sddp->GetWholeExtent(sddp->GetOutputInformation(connection->GetIndex()),
      extent);
    double origin[3], spacing[3];
    vtkImageData* image = vtkImageData::SafeDownCast(
      inputAlgo->GetOutputDataObject(connection->GetIndex()));
    if (!image)
    {
      std::cerr << "expected image data in vtkPVOSPRayImageVolumeRepresentation\n";
      return;
    }
    image->GetOrigin(origin);
    image->GetSpacing(spacing);

    vtkNew<vtkPExtentTranslator> translator;
    translator->GatherExtents(image);
    vtkPVRenderView::SetOrderedCompositingInformation(
      inInfo, self, translator.GetPointer(), extent, origin, spacing);
  }
}

//----------------------------------------------------------------------------
int vtkPVOSPRayImageVolumeRepresentation::RequestData(vtkInformation* request,
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkMath::UninitializeBounds(this->DataBounds);
  this->DataSize = 0;

// Pass caching information to the cache keeper.
  this->CacheKeeper->SetCachingEnabled(this->GetUseCache());
  this->CacheKeeper->SetCacheTime(this->GetCacheKey());

  if (inputVector[0]->GetNumberOfInformationObjects()==1)
  {
    vtkImageData* input = vtkImageData::GetData(inputVector[0], 0);
    if (!this->GetUsingCacheForUpdate())
    {
      this->Cache->ShallowCopy(input);
    }
    this->CacheKeeper->Update();

    this->Actor->SetEnableLOD(0);
    this->VolumeMapper->SetInputConnection(
      this->CacheKeeper->GetOutputPort());

    this->OutlineSource->SetBounds(vtkImageData::SafeDownCast(
      this->CacheKeeper->GetOutputDataObject(0))->GetBounds());
    this->OutlineSource->GetBounds(this->DataBounds);
    this->OutlineSource->Update();

    this->DataSize = this->CacheKeeper->GetOutputDataObject(0)->GetActualMemorySize();
  }
  else
  {
// when no input is present, it implies that this processes is on a node
// without the data input i.e. either client or render-server, in which case
// we show only the outline.
    this->VolumeMapper->RemoveAllInputs();
//    this->Actor->SetEnableLOD(1);
  }

  return this->Superclass::RequestData(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::IsCached(double cache_key)
{
  return this->CacheKeeper->IsCached(cache_key);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::MarkModified()
{
  if (!this->GetUseCache())
  {
// Cleanup caches when not using cache.
    this->CacheKeeper->RemoveAllCaches();
  }
  this->Superclass::MarkModified();
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->GetRenderer()->AddActor(this->Actor);
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkPVOSPRayImageVolumeRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->GetRenderer()->RemoveActor(this->Actor);
    return true;
  }
  return false;
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::UpdateMapperParameters()
{
  const char* colorArrayName = NULL;
  int fieldAssociation = vtkDataObject::FIELD_ASSOCIATION_POINTS;

  vtkInformation *info = this->GetInputArrayInformation(0);
  if (info &&
    info->Has(vtkDataObject::FIELD_ASSOCIATION()) &&
    info->Has(vtkDataObject::FIELD_NAME()))
  {
    colorArrayName = info->Get(vtkDataObject::FIELD_NAME());
    fieldAssociation = info->Get(vtkDataObject::FIELD_ASSOCIATION());
  }

  this->VolumeMapper->SelectScalarArray(colorArrayName);
  switch (fieldAssociation)
  {
    case vtkDataObject::FIELD_ASSOCIATION_CELLS:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_CELL_FIELD_DATA);
    break;

    case vtkDataObject::FIELD_ASSOCIATION_NONE:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_FIELD_DATA);
    break;

    case vtkDataObject::FIELD_ASSOCIATION_POINTS:
    default:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
    break;
  }

  vtkOSPRayVolumeRayCastMapper* ospMapper = vtkOSPRayVolumeRayCastMapper::SafeDownCast(this->VolumeMapper);
  this->Actor->SetMapper(this->VolumeMapper);
  this->Actor->SetVisibility(colorArrayName != NULL && colorArrayName[0] != 0);
  this->Actor->SetVisibility(1);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


//***************************************************************************
// Forwarded to Actor.

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetOrientation(double x, double y, double z)
{
  this->Actor->SetOrientation(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetOrigin(double x, double y, double z)
{
  this->Actor->SetOrigin(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetPickable(int val)
{
  this->Actor->SetPickable(val);
}
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetPosition(double x , double y, double z)
{
  this->Actor->SetPosition(x, y, z);
}
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScale(double x, double y, double z)
{
  this->Actor->SetScale(x, y, z);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetVisibility(bool val)
{
  this->Superclass::SetVisibility(val);
  this->Actor->SetVisibility(val? 1 : 0);
}

//***************************************************************************
// Forwarded to vtkVolumeProperty.
//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetInterpolationType(int val)
{
  this->Property->SetInterpolationType(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetColor(vtkColorTransferFunction* lut)
{
  this->Property->SetColor(lut);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScalarOpacity(vtkPiecewiseFunction* pwf)
{
  this->Property->SetScalarOpacity(pwf);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScalarOpacity2(vtkPiecewiseFunction* pwf)
{
  this->Property->SetScalarOpacity(pwf);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetScalarOpacityUnitDistance(double val)
{
  this->Property->SetScalarOpacityUnitDistance(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetAmbient(double val)
{
  this->Property->SetAmbient(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetDiffuse(double val)
{
  this->Property->SetDiffuse(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetSpecular(double val)
{
  this->Property->SetSpecular(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetSpecularPower(double val)
{
  this->Property->SetSpecularPower(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetShade(bool val)
{
  this->Property->SetShade(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetIndependantComponents(bool val)
{
  this->Property->SetIndependentComponents(val);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayImageVolumeRepresentation::SetRequestedRenderMode(int mode)
{
}

void vtkPVOSPRayImageVolumeRepresentation::SetSamplingRate(double sr)
{
  SamplingRate=sr;
  vtkOSPRayVolumeRayCastMapper* ospMapper = vtkOSPRayVolumeRayCastMapper::SafeDownCast(this->VolumeMapper);
  ospMapper->SetSamplingRate(sr);
}