/* ======================================================================================= 
   Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas at Austin  
   All rights reserved.
                                                                                           
   Licensed under the BSD 3-Clause License, (the "License"); you may not use this file     
   except in compliance with the License.                                                  
   A copy of the License is included with this software in the file LICENSE.               
   If your copy does not contain the License, you may obtain a copy of the License at:     
                                                                                           
       http://opensource.org/licenses/BSD-3-Clause                                         
                                                                                           
   Unless required by applicable law or agreed to in writing, software distributed under   
   the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY 
   KIND, either express or implied.                                                        
   See the License for the specific language governing permissions and limitations under   
   limitations under the License.

   pvOSPRay is derived from VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
   Copyright (c) 2007, Los Alamos National Security, LLC
   ======================================================================================= */

#include "vtkPVOSPRayRepresentation.h"

#include "vtkCompositePolyDataMapper2.h"
#include "vtkInformation.h"
#include "vtkOSPRayCompositeMapper.h"
#include "vtkOSPRayLODActor.h"
#include "vtkOSPRayPolyDataMapper.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayManager.h"
#include "vtkObjectFactory.h"

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkPVOSPRayRepresentation);


//-----------------------------------------------------------------------------
vtkPVOSPRayRepresentation::vtkPVOSPRayRepresentation()
{
  vtkOSPRayManager::Singleton();
  this->Mapper->Delete();
  this->Mapper = vtkOSPRayCompositeMapper::New();
  this->LODMapper->Delete();
  this->LODMapper = vtkOSPRayCompositeMapper::New();

  this->Actor->Delete();
  this->Actor = vtkOSPRayLODActor::New();
  this->Property->Delete();
  this->Property = vtkOSPRayProperty::New();

  this->Actor->SetMapper(this->Mapper);
  this->Actor->SetLODMapper(this->LODMapper);
  this->Actor->SetProperty(this->Property);

  vtkInformation* keys = vtkInformation::New();
  this->Actor->SetPropertyKeys(keys);
  keys->Delete();

  this->SetSuppressLOD(true);
  this->SuppressLOD = true;
}

//-----------------------------------------------------------------------------
vtkPVOSPRayRepresentation::~vtkPVOSPRayRepresentation()
{
}

//-----------------------------------------------------------------------------
void vtkPVOSPRayRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkPVOSPRayRepresentation::SetMaterialType(char *newval)
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  OSPRayProperty->SetMaterialType(newval);
}

//----------------------------------------------------------------------------
char * vtkPVOSPRayRepresentation::GetMaterialType()
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  return OSPRayProperty->GetMaterialType();
}

//----------------------------------------------------------------------------
void vtkPVOSPRayRepresentation::SetReflectance(double newval)
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  OSPRayProperty->SetReflectance(newval);
}

//----------------------------------------------------------------------------
double vtkPVOSPRayRepresentation::GetReflectance()
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  return OSPRayProperty->GetReflectance();
}

//----------------------------------------------------------------------------
void vtkPVOSPRayRepresentation::SetThickness(double newval)
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  OSPRayProperty->SetThickness(newval);
}

//----------------------------------------------------------------------------
double vtkPVOSPRayRepresentation::GetThickness()
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  return OSPRayProperty->GetThickness();
}

//----------------------------------------------------------------------------
void vtkPVOSPRayRepresentation::SetEta(double newval)
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  OSPRayProperty->SetEta(newval);
}

//----------------------------------------------------------------------------
double vtkPVOSPRayRepresentation::GetEta()
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  return OSPRayProperty->GetEta();
}

//----------------------------------------------------------------------------
void vtkPVOSPRayRepresentation::SetN(double newval)
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  OSPRayProperty->SetN(newval);
}

//----------------------------------------------------------------------------
double vtkPVOSPRayRepresentation::GetN()
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  return OSPRayProperty->GetN();
}

//----------------------------------------------------------------------------
void vtkPVOSPRayRepresentation::SetNt(double newval)
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  OSPRayProperty->SetNt(newval);
}

//----------------------------------------------------------------------------
double vtkPVOSPRayRepresentation::GetNt()
{
  vtkOSPRayProperty *OSPRayProperty = vtkOSPRayProperty::SafeDownCast(this->Property);
  return OSPRayProperty->GetNt();
}
