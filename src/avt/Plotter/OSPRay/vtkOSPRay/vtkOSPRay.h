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

// .NAME vtkOSPRay.h -
// .SECTION Description
//
#ifndef __vtkOSPRay_h
#define __vtkOSPRay_h

#include "ospray/ospray.h"

#include "vtkConfigure.h"
#include "vtkOSPRayModule.h"

#if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
# include <OpenGL/gl.h> // Include OpenGL API.
#else
#include "vtkOpenGL.h"
#endif

#define DEBUG(x) {   std::cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << " " << x << std::endl; }
#define USE_OSPRAY 1
#define USE_VBOS 0

#endif
