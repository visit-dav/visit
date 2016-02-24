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

// .NAME vtkPVOSPRayOutlineRepresentation - representation for OSPRay views
// .SECTION Description
// This replaces the GL mapper, actor and property for a display pipline
// with the OSPRay versions of those so that the object can be drawn
// in an OSPRay renderer within a pvOSPRay view

#ifndef __vtkPVOSPRayOutlineRepresentation_h
#define __vtkPVOSPRayOutlineRepresentation_h

#include "vtkPVOSPRayRepresentation.h"
#include "vtkGeometryRepresentationWithFaces.h"
#include "vtkOSPRayModule.h"

class VTKOSPRAY_EXPORT vtkPVOSPRayOutlineRepresentation :
  public vtkPVOSPRayRepresentation
{
public:
  static vtkPVOSPRayOutlineRepresentation* New();
  vtkTypeMacro(vtkPVOSPRayOutlineRepresentation, vtkPVOSPRayRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void SetRepresentation(const char*)
    { this->Superclass::SetRepresentation("Wireframe"); }
  virtual void SetUseOutline(int)
    { this->Superclass::SetUseOutline(1); }
  virtual void SetSuppressLOD(bool)
    { this->Superclass::SetSuppressLOD(true); }
  virtual void SetPickable(int)
    { this->Superclass::SetPickable(0); }


//BTX
protected:
  vtkPVOSPRayOutlineRepresentation();
  ~vtkPVOSPRayOutlineRepresentation();

    virtual void SetRepresentation(int)
    { this->Superclass::SetRepresentation(WIREFRAME); }

private:

  vtkPVOSPRayOutlineRepresentation(const vtkPVOSPRayOutlineRepresentation&); // Not implemented.
  void operator=(const vtkPVOSPRayOutlineRepresentation&); // Not implemented.

//ETX
};


#endif
