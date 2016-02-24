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

// .NAME vtkOSPRayLODActor -
// .SECTION Description
//

#include "vtkOSPRay.h"
#include "vtkOSPRayLODActor.h"
#include "vtkObjectFactory.h"
#include "vtkOSPRayActor.h"
#include "vtkMatrix4x4.h"

vtkStandardNewMacro(vtkOSPRayLODActor);

//----------------------------------------------------------------------------
vtkOSPRayLODActor::vtkOSPRayLODActor()
{
  this->Device->Delete();
  this->Device = vtkOSPRayActor::New();
  vtkMatrix4x4 *m;
  m = vtkMatrix4x4::New();
  this->Device->SetUserMatrix(m);
  m->Delete();
}

//----------------------------------------------------------------------------
void vtkOSPRayLODActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkOSPRayLODActor::SetVisibility(int newval)
{
  this->Device->SetVisibility(newval);
  this->Superclass::SetVisibility(newval);
}
