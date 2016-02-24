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

// .NAME vtkPVOSPRayRepresentation - representation for OSPRay views
// .SECTION Description
// This replaces the GL mapper, actor and property for a display pipline
// with the OSPRay versions of those so that the object can be drawn
// in an OSPRay renderer within a pvOSPRay view

#ifndef __vtkPVOSPRayRepresentation_h
#define __vtkPVOSPRayRepresentation_h

#include "vtkGeometryRepresentationWithFaces.h"
#include "vtkOSPRayModule.h"

class VTKOSPRAY_EXPORT vtkPVOSPRayRepresentation :
  public vtkGeometryRepresentationWithFaces
{
public:
  static vtkPVOSPRayRepresentation* New();
  vtkTypeMacro(vtkPVOSPRayRepresentation, vtkGeometryRepresentationWithFaces);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // control that ray traced rendering characteristics of this object
  void SetMaterialType(char *);
  char *GetMaterialType();
  void SetReflectance(double );
  double GetReflectance();
  void SetThickness(double );
  double GetThickness();
  void SetEta(double);
  double GetEta();
  void SetN(double);
  double GetN();
  void SetNt(double);
  double GetNt();
  void SetOSPSuppressLOD(bool st) { this->SuppressLOD = st; }
  bool GetOSPSuppressLOD() { return this->SuppressLOD; }


//BTX
protected:
  vtkPVOSPRayRepresentation();
  ~vtkPVOSPRayRepresentation();

private:

  vtkPVOSPRayRepresentation(const vtkPVOSPRayRepresentation&); // Not implemented.
  void operator=(const vtkPVOSPRayRepresentation&); // Not implemented.

//ETX
};


#endif
