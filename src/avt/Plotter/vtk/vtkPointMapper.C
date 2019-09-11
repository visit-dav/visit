// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkPointMapper.h"

#include <vtkObjectFactory.h>

//-----------------------------------------------------------------------------
vtkAbstractObjectFactoryNewMacro(vtkPointMapper)

//-----------------------------------------------------------------------------
vtkPointMapper::vtkPointMapper()
{
  this->ImposterScaleArray = 0;
  this->ImposterRadius = 0.3f;
  this->UseImposters = false;
  this->InterpolateScalarsBeforeMappingOff();
}

//-----------------------------------------------------------------------------
vtkPointMapper::~vtkPointMapper()
{
  this->SetImposterScaleArray(0);
}

//-----------------------------------------------------------------------------
void vtkPointMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Imposter Scale Array: " 
     << (this->ImposterScaleArray ? this->ImposterScaleArray : "(none)") << "\n";
  os << indent << "Imposter Radius: " << this->ImposterRadius << "\n";
}
