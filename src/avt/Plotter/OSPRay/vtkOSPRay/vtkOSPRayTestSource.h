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

// .NAME vtkOSPRayTestSource - produce triangles to benchmark OSPRay with
// .SECTION Description
// This class produces polydata with a configurable number of triangles.
// It respects parallelism, so that parallel scaling can be tested, and the
// produced data is invariant with respect to the number of processors.
// Configuration options change the charactersitics of the generated "scene"

#ifndef __vtkOSPRayTestSource_h
#define __vtkOSPRayTestSource_h

#include "vtkOSPRayModule.h"
#include "vtkPolyDataAlgorithm.h"

class VTKOSPRAY_EXPORT vtkOSPRayTestSource : public vtkPolyDataAlgorithm
{
public:
  // Description:
  // Create a new instance with (50,50,50) points in the (u-v-w) directions.
  static vtkOSPRayTestSource *New();
  vtkTypeMacro(vtkOSPRayTestSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the number of triangles to produce.
  // Default is 100 triangles
  vtkSetMacro(Resolution,vtkIdType);
  vtkGetMacro(Resolution,vtkIdType);

  // Description:
  // Set/Get the spatial locality
  // 0.0 behind infinite, 1.0 being none,
  // this affects distance beteen sequential points
  // Default is 0.1
  vtkSetMacro(DriftFactor,double);
  vtkGetMacro(DriftFactor,double);

  // Description:
  // Set/Get the memory locality
  // 0.0 behind infinite (tri uses sequential pts),
  // 1.0 being none (tri uses any point)
  // Default is 0.01
  vtkSetMacro(SlidingWindow,double);
  vtkGetMacro(SlidingWindow,double);

protected:
  vtkOSPRayTestSource();
  ~vtkOSPRayTestSource();

  int RequestInformation(vtkInformation *info, vtkInformationVector **input,
                         vtkInformationVector *output);

  int RequestData(vtkInformation *info, vtkInformationVector **input,
                  vtkInformationVector *output);

  vtkIdType Resolution;
  double DriftFactor;
  double SlidingWindow;

private:

  vtkOSPRayTestSource(const vtkOSPRayTestSource&);  // Not implemented.
  void operator=(const vtkOSPRayTestSource&);  // Not implemented.

};

#endif
