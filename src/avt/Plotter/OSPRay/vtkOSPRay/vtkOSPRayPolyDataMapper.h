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

// .NAME vtkOSPRayPolyDataMapper -
// .SECTION Description
//
// .NAME vtkOSPRayPolyDataMapper - a PolyDataMapper for the OSPRay library
// .SECTION Description
// vtkOSPRayPolyDataMapper is a subclass of vtkPolyDataMapper.
// vtkOSPRayPolyDataMapper is a geometric PolyDataMapper for the OSPRay
// Raytracer library.

#ifndef __vtkOSPRayPolyDataMapper_h
#define __vtkOSPRayPolyDataMapper_h

#include "vtkOSPRayModule.h"
#include "vtkPolyDataMapper.h"
#include "vtkOSPRayTexture.h"

#include <map>

class vtkSphereSource;
class vtkGlyph3D;
class vtkTubeFilter;
class vtkAppendPolyData;

//BTX
namespace OSPRay {
class Mesh;
class Group;
}
//ETX
class vtkCellArray;
class vtkPoints;
class vtkProperty;
class vtkRenderWindow;
class vtkOSPRayRenderer;
class vtkOSPRayManager;

namespace vtkosp
{
  class Mesh;
}
namespace osp
{
  class Model;
}

class VTKOSPRAY_EXPORT vtkOSPRayPolyDataMapper : public vtkPolyDataMapper
{
public:
  static vtkOSPRayPolyDataMapper *New();
  vtkTypeMacro(vtkOSPRayPolyDataMapper,vtkPolyDataMapper);

  // Description:
  // Implement superclass render method.
  virtual void RenderPiece(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OSPRay.
  virtual void Draw(vtkRenderer *ren, vtkActor *a);

protected:
  vtkOSPRayPolyDataMapper();
  ~vtkOSPRayPolyDataMapper();

  //BTX
  void DrawPolygons(vtkPolyData *, vtkPoints *,
                    vtkosp::Mesh * );
  void DrawTStrips(vtkPolyData *, vtkPoints *,
                    vtkosp::Mesh* );
  //ETX

private:
  vtkOSPRayPolyDataMapper(const vtkOSPRayPolyDataMapper&); // Not implemented.
  void operator=(const vtkOSPRayPolyDataMapper&); // Not implemented.

  vtkOSPRayManager *OSPRayManager;

  vtkOSPRayTexture* InternalColorTexture;
  int Representation;
  int Edges;
  double PointSize;
  double LineWidth;
  std::map<int, osp::Model*> cache;
  bool CellScalarColor;
  static int timestep;
//BTX
  class Helper;
  Helper *MyHelper;
//ETX
};

#endif
