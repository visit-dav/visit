/* =======================================================================================
   Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas
   at Austin
   All rights reserved.

   Licensed under the BSD 3-Clause License, (the "License"); you may not use
   this file
   except in compliance with the License.
   A copy of the License is included with this software in the file LICENSE.
   If your copy does not contain the License, you may obtain a copy of the
   License at:

http://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software
distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY
KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under
limitations under the License.

pvOSPRay is derived from VTK/ParaView Los Alamos National Laboratory Modules
(PVLANL)
Copyright (c) 2007, Los Alamos National Security, LLC
=======================================================================================
*/

#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"

#include "vtkOSPRay.h"
#include "vtkOSPRayActor.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayPolyDataMapper.h"
#include "vtkOSPRayProperty.h"
#include "vtkOSPRayRenderer.h"
#include "vtkOSPRayTexture.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkDataArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkGlyph3D.h"
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkScalarsToColors.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTubeFilter.h"
#include "vtkUnsignedCharArray.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"

#include <vector>

#include <math.h>
#include <algorithm>


vtkStandardNewMacro(vtkOSPRayPolyDataMapper);

namespace vtkosp {
    class Vec3 {
        public:
            Vec3(float x, float y, float z) {
                vals[0] = x;
                vals[1] = y;
                vals[2] = z;
            }
            float operator[](unsigned int i) const { return vals[i]; }
            float x() { return vals[0]; }
            float y() { return vals[1]; }
            float z() { return vals[2]; }

            float vals[3];
    };
    class Vec4 {
        public:
            Vec4(float x, float y, float z, float w) {
                vals[0] = x;
                vals[1] = y;
                vals[2] = z;
                vals[3] = w;
            }
            float operator[](unsigned int i) const { return vals[i]; }
            float x() { return vals[0]; }
            float y() { return vals[1]; }
            float z() { return vals[2]; }
            float w() { return vals[3]; }

            float vals[4];
    };
    class Vec2 {
        public:
            Vec2(float x, float y) {
                vals[0] = x;
                vals[1] = y;
            }
            float operator[](unsigned int i) const { return vals[i]; }
            float x() { return vals[0]; }
            float y() { return vals[1]; }

            float vals[2];
    };
    class Mesh {
        public:
            size_t size() { return vertex_indices.size() / 3; }
            std::vector<size_t> vertex_indices;
            std::vector<Vec3> vertices;
            std::vector<Vec3> vertexNormals;
            std::vector<Vec2> texCoords;
            std::vector<size_t> texture_indices;
            std::vector<size_t> normal_indices;
            std::vector<Vec4> colors;
            std::vector<ospray::vec3fa> wireframe_vertex;
            std::vector<int> wireframe_index;
            std::vector<ospray::vec3f> points_vertex;
    };
}

int vtkOSPRayPolyDataMapper::timestep = 0;  // HACK!
void* alignedMalloc(size_t size, size_t align=64)
{
    if (size == 0) return NULL;
    char* base = (char*)malloc(size + align + sizeof(int));
    if (base == NULL) throw std::bad_alloc();

    char* unaligned = base + sizeof(int);
    char*   aligned = unaligned + align - ((size_t)unaligned & (align - 1));
    ((int*)aligned)[-1] = (int)((size_t)aligned - (size_t)base);
    return aligned;
}

void alignedFree(const void* ptr) {
    if (ptr == NULL) return;
    int ofs = ((int*)ptr)[-1];
    free((char*)ptr - ofs);
}

//----------------------------------------------------------------------------
// Construct empty object.
vtkOSPRayPolyDataMapper::vtkOSPRayPolyDataMapper() {
    this->InternalColorTexture = NULL;
    this->OSPRayManager = NULL;
    this->PointSize = 1.0;
    this->LineWidth = 1.0;
    this->Representation = VTK_SURFACE;
}

//----------------------------------------------------------------------------
// Destructor (don't call ReleaseGraphicsResources() since it is virtual
vtkOSPRayPolyDataMapper::~vtkOSPRayPolyDataMapper() {
    if (this->InternalColorTexture) {
        this->InternalColorTexture->Delete();
    }

    if (this->OSPRayManager) {
        this->OSPRayManager->Delete();
    }

}

//----------------------------------------------------------------------------
// Release the graphics resources used by this mapper.  In this case, release
// the display list if any.
void vtkOSPRayPolyDataMapper::ReleaseGraphicsResources(vtkWindow *win) {
    this->Superclass::ReleaseGraphicsResources(win);

    if (this->InternalColorTexture) {
        this->InternalColorTexture->Delete();
    }
    this->InternalColorTexture = NULL;
}

//----------------------------------------------------------------------------
// Receives from Actor -> maps data to primitives
// called by Mapper->Render() (which is called by Actor->Render())
void vtkOSPRayPolyDataMapper::RenderPiece(vtkRenderer *ren, vtkActor *act) {
    vtkOSPRayRenderer *OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast(ren);
    if (!OSPRayRenderer) {
        return;
    }
    if (!this->OSPRayManager) {
        this->OSPRayManager = OSPRayRenderer->GetOSPRayManager();
        this->OSPRayManager->Register(this);
    }

    // write geometry, first ask the pipeline to update data
    vtkPolyData *input = this->GetInput();
    if (input == NULL) {
        vtkErrorMacro(<< "No input to vtkOSPRayPolyDataMapper!");
        return;
    } else {
        this->InvokeEvent(vtkCommand::StartEvent, NULL);

        // Static = 1:  this mapper does NOT need to propagate updates to other
        // mappers
        // down the pipeline and therefore saves the time that would be otherwise
        // taken
        if (!this->Static) {
            this->Update();
        }

        this->InvokeEvent(vtkCommand::EndEvent, NULL);

        vtkIdType numPts = input->GetNumberOfPoints();
        if (numPts == 0) {
            vtkDebugMacro(<< "No points from the input to vtkOSPRayPolyDataMapper!");
            input = NULL;
            return;
        }
    }

    if (this->LookupTable == NULL) {
        this->CreateDefaultLookupTable();
    }

    // For vertex coloring, this sets this->Colors as side effect.
    // For texture map coloring, this sets ColorCoordinates
    // and ColorTextureMap as a side effect.
    this->MapScalars(act->GetProperty()->GetOpacity());

    if (this->ColorTextureMap) {
        if (!this->InternalColorTexture) {
            this->InternalColorTexture = vtkOSPRayTexture::New();
            this->InternalColorTexture->RepeatOff();
        }
        this->InternalColorTexture->SetInputData(this->ColorTextureMap);
    }

    // if something has changed, regenerate OSPRay primitives if required
    if (this->GetMTime() > this->BuildTime ||
            input->GetMTime() > this->BuildTime ||
            act->GetProperty()->GetMTime() > this->BuildTime ||
            act->GetMatrix()->GetMTime() > this->BuildTime) {

        // If we are coloring by texture, then load the texture map.
        // Use Map as indicator, because texture hangs around.
        if (this->ColorTextureMap) {
            this->InternalColorTexture->Load(ren, true);
        }

        this->Draw(ren, act);
        this->BuildTime.Modified();
    }

    this->Update();
    input = NULL;
}

//----------------------------------------------------------------------------
void vtkOSPRayPolyDataMapper::DrawPolygons(vtkPolyData *polys,
        vtkPoints *ptarray,
        vtkosp::Mesh *mesh
        ) {

    int total_triangles = 0;
    vtkCellArray *cells = polys->GetPolys();
    vtkIdType npts = 0, *index = 0, cellNum = 0;

    switch (this->Representation) {
        case VTK_POINTS: {
                             double coord[3];
                             for (cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++) {
                                 ptarray->GetPoint(index[0], coord);
                                 mesh->points_vertex.push_back
                                     (ospray::vec3f(coord[0], coord[1], coord[2]));
                                 for (vtkIdType i = 1; i < npts; i++) {

                                     ptarray->GetPoint(index[i], coord);
                                     mesh->points_vertex.push_back
                                         (ospray::vec3f(coord[0], coord[1], coord[2]));
                                 }
                             }
                         }  // VTK_POINTS;
                         break;
        case VTK_WIREFRAME: {
                                double coord0[3];
                                for (cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++) {
                                    ptarray->GetPoint(index[0], coord0);
                                    mesh->wireframe_vertex.push_back(
                                            ospray::vec3fa(coord0[0], coord0[1], coord0[2]));
                                    for (vtkIdType i = 1; i < npts; i++) {

                                        mesh->wireframe_index.push_back(mesh->wireframe_vertex.size() - 1);
                                        ptarray->GetPoint(index[i], coord0);
                                        mesh->wireframe_vertex.push_back(
                                                ospray::vec3fa(coord0[0], coord0[1], coord0[2]));
                                    }
                                }
                            }  // VTK_WIREFRAME:
                            break;
        case VTK_SURFACE: {
                              // write polygons with on the fly triangulation, assuming polygons are
                              // simple and
                              // can be triangulated into "fans"
                              for (cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++) {
                                  int triangle[3];

                                  // the first triangle
                                  triangle[0] = index[0];
                                  triangle[1] = index[1];
                                  triangle[2] = index[2];
                                  mesh->vertex_indices.push_back(triangle[0]);
                                  mesh->vertex_indices.push_back(triangle[1]);
                                  mesh->vertex_indices.push_back(triangle[2]);

                                  if (!mesh->vertexNormals.empty()) {
                                      mesh->normal_indices.push_back(triangle[0]);
                                      mesh->normal_indices.push_back(triangle[1]);
                                      mesh->normal_indices.push_back(triangle[2]);
                                  }

                                  if (!mesh->texCoords.empty()) {
                                      if (this->CellScalarColor) {
                                          mesh->texture_indices.push_back(cellNum);
                                          mesh->texture_indices.push_back(cellNum);
                                          mesh->texture_indices.push_back(cellNum);
                                      } else {
                                          mesh->texture_indices.push_back(triangle[0]);
                                          mesh->texture_indices.push_back(triangle[1]);
                                          mesh->texture_indices.push_back(triangle[2]);
                                      }
                                  }
                                  total_triangles++;

                                  // the remaining triangles, of which
                                  // each introduces a triangle after extraction
                                  for (int i = 3; i < npts; i++) {
                                      triangle[1] = triangle[2];
                                      triangle[2] = index[i];
                                      mesh->vertex_indices.push_back(triangle[0]);
                                      mesh->vertex_indices.push_back(triangle[1]);
                                      mesh->vertex_indices.push_back(triangle[2]);

                                      if (!mesh->vertexNormals.empty()) {
                                          mesh->normal_indices.push_back(triangle[0]);
                                          mesh->normal_indices.push_back(triangle[1]);
                                          mesh->normal_indices.push_back(triangle[2]);
                                      }

                                      if (!mesh->texCoords.empty()) {
                                          if (this->CellScalarColor) {
                                              mesh->texture_indices.push_back(cellNum);
                                              mesh->texture_indices.push_back(cellNum);
                                              mesh->texture_indices.push_back(cellNum);
                                          } else {
                                              mesh->texture_indices.push_back(triangle[0]);
                                              mesh->texture_indices.push_back(triangle[1]);
                                              mesh->texture_indices.push_back(triangle[2]);
                                          }
                                      }
                                      total_triangles++;
                                  }
                              }

                              for (int i = 0; i < total_triangles; i++) {
                              }

                              for (cells->InitTraversal();
                                      this->Edges && cells->GetNextCell(npts, index); cellNum++) {
                                  double coord0[3];
                                  ptarray->GetPoint(index[0], coord0);
                                  mesh->wireframe_vertex.push_back(
                                          ospray::vec3fa(coord0[0], coord0[1], coord0[2]));
                                  for (vtkIdType i = 1; i < npts; i++) {
                                      mesh->wireframe_index.push_back(mesh->wireframe_vertex.size() - 1);
                                      ptarray->GetPoint(index[i], coord0);
                                      mesh->wireframe_vertex.push_back(
                                              ospray::vec3fa(coord0[0], coord0[1], coord0[2]));
                                  }
                              }


                          }  // VTK_SURFACE
                          break;
        default:
                          std::cerr << "unknwon representation type\n";
                          break;
    }
}

//----------------------------------------------------------------------------
void vtkOSPRayPolyDataMapper::DrawTStrips(vtkPolyData *polys,
        vtkPoints *ptarray,
        vtkosp::Mesh *mesh)
{

    // total number of triangles
    int total_triangles = 0;

    vtkCellArray *cells = polys->GetStrips();
    vtkIdType npts = 0, *index = 0, cellNum = 0;
    ;

    switch (this->Representation) {
        case VTK_POINTS: {
                             for (cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++) {
                                 double coord[3];
                                 for (int i = 0; i < npts; i++) {
                                     ptarray->GetPoint(index[i], coord);
                                 }
                                 total_triangles++;
                             }
                         }  // VTK_POINTS;
                         break;
        case VTK_WIREFRAME: {
                                double coord0[3];
                                double coord1[3];
                                double coord2[3];
                                for (cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++) {
                                    ptarray->GetPoint(index[0], coord0);
                                    mesh->wireframe_vertex.push_back(
                                            ospray::vec3fa(coord0[0], coord0[1], coord0[2]));
                                    for (vtkIdType i = 2; i < npts; i++) {
                                        mesh->wireframe_index.push_back(mesh->wireframe_vertex.size() - 1);
                                        ptarray->GetPoint(index[0], coord0);
                                        mesh->wireframe_vertex.push_back(
                                                ospray::vec3fa(coord0[0], coord0[1], coord0[2]));
                                    }
                                }
                            }  // VTK_WIREFRAME:
                            break;
        case VTK_SURFACE: {
                              for (cells->InitTraversal(); cells->GetNextCell(npts, index); cellNum++) {
                                  // count of the i-th triangle in a strip
                                  int numtriangles2 = 0;

                                  int triangle[3];
                                  // the first triangle
                                  triangle[0] = index[0];
                                  triangle[1] = index[1];
                                  triangle[2] = index[2];
                                  mesh->vertex_indices.push_back(triangle[0]);
                                  mesh->vertex_indices.push_back(triangle[1]);
                                  mesh->vertex_indices.push_back(triangle[2]);

                                  if (!mesh->vertexNormals.empty()) {
                                      mesh->normal_indices.push_back(triangle[0]);
                                      mesh->normal_indices.push_back(triangle[1]);
                                      mesh->normal_indices.push_back(triangle[2]);
                                  }

                                  if (!mesh->texCoords.empty()) {
                                      if (this->CellScalarColor) {
                                          mesh->texture_indices.push_back(cellNum);
                                          mesh->texture_indices.push_back(cellNum);
                                          mesh->texture_indices.push_back(cellNum);
                                      } else {
                                          mesh->texture_indices.push_back(triangle[0]);
                                          mesh->texture_indices.push_back(triangle[1]);
                                          mesh->texture_indices.push_back(triangle[2]);
                                      }
                                  }

                                  total_triangles++;
                                  numtriangles2++;

                                  // the rest of triangles
                                  for (int i = 3; i < npts; i++) {
                                      int tmp[3];
                                      if (numtriangles2 % 2 == 1) {
                                          // an odd triangle
                                          tmp[0] = triangle[1];
                                          tmp[1] = triangle[2];
                                          tmp[2] = index[i];

                                          triangle[0] = tmp[0];
                                          triangle[1] = tmp[2];
                                          triangle[2] = tmp[1];
                                      } else {
                                          // an even triangle
                                          tmp[0] = triangle[1];
                                          tmp[1] = triangle[2];
                                          tmp[2] = index[i];

                                          triangle[0] = tmp[1];
                                          triangle[1] = tmp[0];
                                          triangle[2] = tmp[2];
                                      }

                                      mesh->vertex_indices.push_back(triangle[0]);
                                      mesh->vertex_indices.push_back(triangle[1]);
                                      mesh->vertex_indices.push_back(triangle[2]);

                                      if (!mesh->vertexNormals.empty()) {
                                          mesh->normal_indices.push_back(triangle[0]);
                                          mesh->normal_indices.push_back(triangle[1]);
                                          mesh->normal_indices.push_back(triangle[2]);
                                      }

                                      if (!mesh->texCoords.empty()) {
                                          if (this->CellScalarColor) {
                                              mesh->texture_indices.push_back(cellNum);
                                              mesh->texture_indices.push_back(cellNum);
                                              mesh->texture_indices.push_back(cellNum);
                                          } else {
                                              mesh->texture_indices.push_back(triangle[0]);
                                              mesh->texture_indices.push_back(triangle[1]);
                                              mesh->texture_indices.push_back(triangle[2]);
                                          }
                                      }

                                      total_triangles++;
                                      numtriangles2++;
                                  }
                              }


                          } break;

        default:
                          std::cerr << "unkown representation type\n";
    }
}

void FindAllData(vtkPolyData *polydata) {

    std::cout << "------------------------------------------------ " << std::endl;

    std::cout << "Normals: " << polydata->GetPointData()->GetNormals()
        << std::endl;

    vtkIdType numberOfPointArrays = polydata->GetPointData()->GetNumberOfArrays();
    std::cout << "Number of PointData arrays: " << numberOfPointArrays
        << std::endl;

    vtkIdType numberOfCellArrays = polydata->GetCellData()->GetNumberOfArrays();
    std::cout << "Number of CellData arrays: " << numberOfCellArrays << std::endl;

    std::cout << "Type table/key: " << std::endl;
    ;
    // more values can be found in <VTK_DIR>/Common/vtkSetGet.h
    std::cout << VTK_UNSIGNED_CHAR << " unsigned char" << std::endl;
    std::cout << VTK_UNSIGNED_INT << " unsigned int" << std::endl;
    std::cout << VTK_FLOAT << " float" << std::endl;
    std::cout << VTK_DOUBLE << " double" << std::endl;

    for (vtkIdType i = 0; i < numberOfPointArrays; i++) {
        int dataTypeID = polydata->GetPointData()->GetArray(i)->GetDataType();
        int arrayTypeID = polydata->GetPointData()->IsArrayAnAttribute(i);
        std::cout << "Array " << i << ": "
            << polydata->GetPointData()->GetArrayName(i)
            << " (type: " << dataTypeID << ")" << arrayTypeID << std::endl;
    }

    for (vtkIdType i = 0; i < numberOfCellArrays; i++) {
        int dataTypeID = polydata->GetCellData()->GetArray(i)->GetDataType();
        std::cout << "Array " << i << ": "
            << polydata->GetCellData()->GetArrayName(i)
            << " (type: " << dataTypeID << ")" << std::endl;
    }
    std::cout << "------------------------------------------------ " << std::endl;
}

//----------------------------------------------------------------------------
// Draw method for OSPRay.
void vtkOSPRayPolyDataMapper::Draw(vtkRenderer *renderer, vtkActor *actor) {
    vtkOSPRayActor *OSPRayActor = vtkOSPRayActor::SafeDownCast(actor);
    if (!OSPRayActor) {
        return;
    }
    vtkOSPRayProperty *OSPRayProperty =
        vtkOSPRayProperty::SafeDownCast(OSPRayActor->GetProperty());
    if (!OSPRayProperty) {
        return;
    }
    vtkOSPRayRenderer *OSPRayRenderer = vtkOSPRayRenderer::SafeDownCast(renderer);

    vtkPolyData *input = this->GetInput();

    vtkInformation *inputInfo = this->GetInput()->GetInformation();
    // std::cout << __PRETTY_FUNCTION__ << " (" << this << ") " << "actor: (" <<
    // OSPRayActor << ") mode: (" << OSPRayActor->OSPRayModel << ") " << std::endl;


    // if (inputInfo && inputInfo->Has(vtkDataObject::DATA_TIME_STEP())) {
    //   double time = inputInfo->Get(vtkDataObject::DATA_TIME_STEP());
    //   timestep = time;
    //   if (OSPRayActor->cache[time] != NULL) {

    //     OSPRayActor->OSPRayModel = OSPRayActor->cache[time];
    //     return;

    //   }

    // } else if (!inputInfo) {
    // } else {
    //   if (OSPRayActor->cache[timestep] != NULL) {
    //   }
    // }
    OSPRayActor->MeshMTime.Modified();

    // Compute we need to for color
    this->Representation = OSPRayProperty->GetRepresentation();
    this->Edges = OSPRayProperty->GetEdgeVisibility();

    this->CellScalarColor = false;
    if ((this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_DATA ||
                this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA ||
                this->ScalarMode == VTK_SCALAR_MODE_USE_FIELD_DATA ||
                !input->GetPointData()->GetScalars()) &&
            this->ScalarMode != VTK_SCALAR_MODE_USE_POINT_FIELD_DATA) {
        this->CellScalarColor = true;
    }

    OSPMaterial ospMaterial = NULL;
    vtkosp::Mesh *mesh = new vtkosp::Mesh();

    // force create a new material every time this is called in case the renderer
    // has changed
    osp::Material *osmat = 0;
    if (!osmat) {
        OSPRayProperty->CreateOSPRayProperty();
        ospMaterial = ((OSPMaterial)OSPRayProperty->GetOSPRayMaterial());
    } else
        ospMaterial = ((OSPMaterial)osmat);

    if (!this->ScalarVisibility || (!this->Colors && !this->ColorCoordinates)) {

    } else if (this->Colors) {
        if (OSPRayProperty->GetInterpolation() == VTK_FLAT) {
        } else {
            if (OSPRayProperty->GetOpacity() < 1.0) {
            } else {
                if (OSPRayProperty->GetSpecular() == 0) {
                } else {
                }
            }
        }

        for (int i = 0; i < this->Colors->GetNumberOfTuples(); i++) {
            unsigned char *color = this->Colors->GetPointer(4 * i);
            mesh->colors.push_back(vtkosp::Vec4(color[0] / 255.0, color[1] / 255.0,
                        color[2] / 255.0, 1));
        }

    } else if (this->ColorCoordinates) {
        osp::Texture2D *texture = this->InternalColorTexture->GetOSPRayTexture();
        Assert(texture);
        ospSetObject(ospMaterial, "map_Kd", ((OSPTexture2D)(texture)));
        ospCommit(ospMaterial);

        for (int i = 0; i < this->ColorCoordinates->GetNumberOfTuples(); i++) {
            double *tcoord = this->ColorCoordinates->GetTuple(i);
            if (tcoord[0] >= 1.0) tcoord[0] = 0.99999;  // avoid sampling texture at 1
            mesh->texCoords.push_back(vtkosp::Vec2(tcoord[0], tcoord[1]));
        }

    } else if (input->GetPointData()->GetTCoords() && actor->GetTexture()) {
        vtkOSPRayTexture *osprayTexture =
            vtkOSPRayTexture::SafeDownCast(actor->GetTexture());
        if (osprayTexture) {
            ospSetObject(ospMaterial, "map_Kd",
                    ((OSPTexture2D)(osprayTexture->GetOSPRayTexture())));
            ospCommit(ospMaterial);
        }

        vtkDataArray *tcoords = input->GetPointData()->GetTCoords();
        for (int i = 0; i < tcoords->GetNumberOfTuples(); i++) {
            double *tcoord = tcoords->GetTuple(i);
            if (tcoord[0] >= 1.0) tcoord[0] = 0.99999;  // avoid sampling texture at 1
            mesh->texCoords.push_back(vtkosp::Vec2(tcoord[0], tcoord[1]));
        }
    }

    // transform point coordinates according to actor's transformation matrix
    vtkTransform *transform = vtkTransform::New();
    transform->SetMatrix(actor->GetMatrix());
    vtkPoints *points = vtkPoints::New();
    transform->TransformPoints(input->GetPoints(), points);

    // obtain the OpenGL-based point size and line width
    // that are specified through vtkProperty
    this->PointSize = OSPRayProperty->GetPointSize();
    this->LineWidth = OSPRayProperty->GetLineWidth();
    if (this->PointSize < 0.0) {
        this->PointSize = 1.0;
    }
    if (this->LineWidth < 0.0) {
        this->LineWidth = 1.0;
    }
    this->PointSize = sqrt(this->PointSize) * 0.010;
    this->LineWidth = sqrt(this->LineWidth) * 0.005;

    // convert VTK_VERTEX cells to OSPRay spheres
    if (input->GetNumberOfVerts() > 0) {
        vtkCellArray *ca = input->GetVerts();
        ca->InitTraversal();
        vtkIdType npts;
        vtkIdType *pts;
        vtkPoints *ptarray = points;
        double coord[3];
        vtkIdType cell;
        vtkIdType point;
        while ((cell = ca->GetNextCell(npts, pts)))
        {
            for (int p = 0; p < npts; p++)
            {
                point = pts[p];
                ptarray->GetPoint(point, coord);
                mesh->points_vertex.push_back
                    (ospray::vec3f(coord[0], coord[1], coord[2]));
            }
        }
    }

    std::vector<ospray::vec3fa> slVertex;
    std::vector<ospray::vec3fa> slColors;
    std::vector<int> slIndex;
    float slRadius;

    // convert VTK_LINE type cells to OSPRay cylinders
    if (input->GetNumberOfLines() > 0) {
        vtkCellArray *ca = input->GetLines();
        ca->InitTraversal();
        vtkIdType npts;
        vtkIdType *pts;
        vtkPoints *ptarray = points;
        std::vector<ospray::vec3fa> tmpColors;
        int scalarSize = ptarray->GetNumberOfPoints();
        double coord0[3];
        vtkIdType cell;

        vtkScalarsToColors *vstc = GetLookupTable();
        vtkDataArray *scalar = input->GetPointData()->GetScalars(NULL);
        if (scalar) {
            int vectorSize = (scalar) ? scalar->GetNumberOfComponents() : 0;
            unsigned char *output = new unsigned char[scalarSize * 4];
            vstc->SetVectorModeToMagnitude();
            vstc->MapVectorsThroughTable(scalar->GetVoidPointer(0), output,
                    scalar->GetDataType(), scalarSize,
                    vectorSize, VTK_RGBA);
            for (int ii = 0; ii < scalarSize; ii++) {
                double color[3];
                for (int jj = 0; jj < 3; jj++) {
                    color[jj] = float(output[ii * 4 + jj]) / 255.0;
                }
                tmpColors.push_back(ospray::vec3fa(color[0], color[1], color[2]));
            }
        } else if (vstc) {

            if (this->ColorCoordinates) {
                //std::cout << "Tex coords " << this->ColorCoordinates->GetSize() << std::endl;
                double* minmax = vstc->GetRange();
                //std::cout << "m: " << minmax[0] << " M:" << minmax[1] << std::endl;

                double scale = minmax[1] - minmax[0];

                for (int i = 0; i < scalarSize; i++) {
                    double *tcoord = this->ColorCoordinates->GetTuple(i);
                    double *color = vstc->GetColor((tcoord[0] * scale) + minmax[0]);
                    tmpColors.push_back(ospray::vec3fa(color[0], color[1], color[2]));
                }

            } else {
                double solidColor[3];
                OSPRayProperty->GetDiffuseColor(solidColor);
                for (int i = 0; i < scalarSize; i++) {
                    tmpColors.push_back(
                            ospray::vec3fa(solidColor[0], solidColor[1], solidColor[2]));
                }
            }
        }

        std::vector<ospray::vec3fa> tmpPoints;
        for (int ii = 0; ii < ptarray->GetNumberOfPoints(); ii++) {
            ptarray->GetPoint(ii, coord0);
            tmpPoints.push_back(ospray::vec3fa(coord0[0], coord0[1], coord0[2]));
        }

        slRadius = this->LineWidth / 0.005;

        while ((cell = ca->GetNextCell(npts, pts))) {
            if (npts <= 2) continue;
            slVertex.push_back(tmpPoints[pts[0]]);
            slColors.push_back(tmpColors[pts[0]]);
            for (vtkIdType i = 1; i < npts; i++) {
                slIndex.push_back(slVertex.size() - 1);
                slVertex.push_back(tmpPoints[pts[i]]);
                slColors.push_back(tmpColors[pts[i]]);
            }
        }

    }

    // convert coordinates to OSPRay format
    for (int i = 0; i < points->GetNumberOfPoints(); i++) {
        double *pos = points->GetPoint(i);
        bool wasNan = false;
        int fixIndex = i - 1;
        do {
            wasNan = false;
            for (int j = 0; j < 3; j++) {
                if (std::isnan(pos[j])) {
                    wasNan = true;
                }
            }
            if (wasNan && fixIndex >= 0) pos = points->GetPoint(fixIndex--);
        } while (wasNan == true && fixIndex >= 0);
        mesh->vertices.push_back(vtkosp::Vec3(pos[0], pos[1], pos[2]));
    }

    // Do flat shading by not supplying vertex normals to OSPRay
    if (OSPRayProperty->GetInterpolation() != VTK_FLAT) {
        vtkPointData *pointData = input->GetPointData();
        if (pointData->GetNormals()) {
            vtkDataArray *normals = vtkFloatArray::New();
            normals->SetNumberOfComponents(3);
            transform->TransformNormals(pointData->GetNormals(), normals);
            for (int i = 0; i < normals->GetNumberOfTuples(); i++) {
                double *normal = normals->GetTuple(i);
                mesh->vertexNormals.push_back(
                        vtkosp::Vec3(normal[0], normal[1], normal[2]));
            }
            normals->Delete();
        }
    }


    // convert polygons to OSPRay format
    if (input->GetNumberOfPolys() > 0) {
        this->DrawPolygons(input, points, mesh /*, sphereGroup, tubeGroup*/);
    }

    // convert triangle strips to OSPRay format
    if (input->GetNumberOfStrips() > 0) {
        this->DrawTStrips(input, points, mesh /*, sphereGroup, tubeGroup*/);
    }

    // delete transformed point coordinates
    transform->Delete();
    points->Delete();

    if (mesh->size() ||
            mesh->wireframe_vertex.size() ||
            slVertex.size() ||
            mesh->points_vertex.size()) {
        //
        // ospray
        //
        OSPRenderer renderer = ((OSPRenderer) this->OSPRayManager->OSPRayRenderer);
        // OSPRayActor->OSPRayModel = ospNewModel();
        //TODO: There should be a better way to clear geometries than to remake model
        // printf("actor frame: %d renderer frame: %d\n", OSPRayActor->GetLastFrame(),OSPRayRenderer->GetFrame());
        if (OSPRayActor->GetLastFrame() < OSPRayRenderer->GetFrame())
        {
            // printf("new actor model\n");
            OSPRayActor->OSPRayModel = ospNewModel();
            OSPRayActor->LastFrame = OSPRayRenderer->GetFrame();
        }

        if (mesh->size() && !mesh->points_vertex.size()) {

            size_t numNormals = mesh->vertexNormals.size();
            size_t numTexCoords = mesh->texCoords.size();
            size_t numPositions = mesh->vertices.size();
            size_t numTriangles = mesh->vertex_indices.size() / 3;

            // printf("building mesh with numTriangles %d\n", numTriangles);


            ospray::vec3fa *vertices = (ospray::vec3fa *)alignedMalloc(
                    sizeof(ospray::vec3fa) * numPositions);
            ospray::vec3i *triangles = (ospray::vec3i *)alignedMalloc(
                    sizeof(ospray::vec3i) * numTriangles);
            ospray::vec3fa *normals = (ospray::vec3fa *)alignedMalloc(
                    sizeof(ospray::vec3fa) * numNormals);

            for (size_t i = 0; i < numPositions; i++) {
                vertices[i] =
                    ospray::vec3fa(mesh->vertices[i].x(), mesh->vertices[i].y(),
                            mesh->vertices[i].z());
            }
            for (size_t i = 0, mi = 0; i < numTriangles; i++, mi += 3) {
                triangles[i] = embree::Vec3i(mesh->vertex_indices[mi + 0],
                        mesh->vertex_indices[mi + 1],
                        mesh->vertex_indices[mi + 2]);
            }

            for (size_t i = 0; i < numNormals; i++) {
                normals[i] = ospray::vec3fa(mesh->vertexNormals[i].x(),
                        mesh->vertexNormals[i].y(),
                        mesh->vertexNormals[i].z());
            }

            OSPGeometry ospMesh = ospNewTriangleMesh();
            OSPData position = ospNewData(numPositions, OSP_FLOAT3A, &vertices[0]);
            ospSetData(ospMesh, "position", position);

            if (!mesh->normal_indices.empty()) {
                OSPData normal =
                    ospNewData(mesh->vertexNormals.size(), OSP_FLOAT3A, &normals[0]);
                ospSetData(ospMesh, "vertex.normal", normal);
            }

            OSPData index = ospNewData(numTriangles, OSP_INT3, &triangles[0]);
            ospSetData(ospMesh, "index", index);

            if (!mesh->texCoords.empty()) {
                OSPData texcoord =
                    ospNewData(mesh->texCoords.size(), OSP_FLOAT2, &mesh->texCoords[0]);
                assert(mesh->texCoords.size() > 0);
                ospSetData(ospMesh, "vertex.texcoord", texcoord);
            }
            if (!mesh->colors.empty()) {
                // note: to share data use OSP_DATA_SHARED_BUFFER
                OSPData colors =
                    ospNewData(mesh->colors.size(), OSP_FLOAT4, &mesh->colors[0]);
                ospSetData(ospMesh, "vertex.color", colors);
            }

            if (!ospMaterial) {
                OSPRayProperty->CreateOSPRayProperty();
                ospMaterial = ((OSPMaterial)OSPRayProperty->GetOSPRayMaterial());
            }

            ospSetMaterial(ospMesh, ospMaterial);
            ospCommit(ospMesh);

            ospAddGeometry(OSPRayActor->OSPRayModel, ospMesh);

        }

        if (mesh->points_vertex.size()) {
            double Color[3];
            OSPRayProperty->GetColor(Color);
            OSPMaterial pointMat = ospNewMaterial(renderer, "default");
            if (pointMat) {
                ospSet3f(pointMat, "kd", Color[0], Color[1], Color[2]);
                ospCommit(pointMat);
            }
            OSPGeometry ospMesh = ospNewGeometry("spheres");
            OSPData vertex = ospNewData(mesh->points_vertex.size(), OSP_FLOAT3,
                    &mesh->points_vertex[0]);
            ospSetObject(ospMesh, "spheres", vertex);
            ospSet1i(ospMesh, "bytes_per_sphere", 3*sizeof(float));
            ospSet1i(ospMesh, "offset_center", 0*sizeof(float));
            ospSet1i(ospMesh, "offset_radius", -1);//3*sizeof(float));
            ospSet1f(ospMesh, "radius", this->PointSize);
            ospSet1i(ospMesh, "offset_materialID", -1);
            ospSet1i(ospMesh, "materialID", 0);
            ospAddGeometry(OSPRayActor->OSPRayModel, ospMesh);
            if (pointMat)
            {
                ospSetMaterial(ospMesh, pointMat);
            }
            ospCommit(vertex);
            ospCommit(ospMesh);
            ospRelease(vertex);
            ospRelease(ospMesh);
        }

        if (mesh->wireframe_vertex.size()) {
            double edgeColor[3];
            OSPRayProperty->GetEdgeColor(edgeColor);
            OSPMaterial wireMat = ospNewMaterial(renderer, "default");
            if (wireMat) {
                ospSet3f(wireMat, "kd", edgeColor[0], edgeColor[1], edgeColor[2]);
                ospCommit(wireMat);
            }
            OSPGeometry wireGeometry = ospNewGeometry("streamlines");
            Assert(wireGeometry);
            OSPData vertex = ospNewData(mesh->wireframe_vertex.size(), OSP_FLOAT3A,
                    &mesh->wireframe_vertex[0]);
            OSPData index = ospNewData(mesh->wireframe_index.size(), OSP_INT,
                    &mesh->wireframe_index[0]);
            ospSetObject(wireGeometry, "vertex", vertex);
            ospSetObject(wireGeometry, "index", index);
            ospSet1f(wireGeometry, "radius", this->LineWidth);

            if (wireMat) ospSetMaterial(wireGeometry, wireMat);

            ospCommit(wireGeometry);
            ospAddGeometry(OSPRayActor->OSPRayModel, wireGeometry);
        }

        if (slVertex.size()) {
            double solidColor[3];
            OSPGeometry slGeometry = ospNewGeometry("streamlines");
            Assert(slGeometry);
            OSPData vertex = ospNewData(slVertex.size(), OSP_FLOAT3A, &slVertex[0]);
            OSPData color = ospNewData(slColors.size(), OSP_FLOAT3A, &slColors[0]);
            OSPData index = ospNewData(slIndex.size(), OSP_INT, &slIndex[0]);
            ospSetObject(slGeometry, "vertex", vertex);
            ospSetObject(slGeometry, "vertex.color", color);
            ospSetObject(slGeometry, "index", index);
            ospSet1f(slGeometry, "radius", slRadius);
            ospCommit(slGeometry);
            ospAddGeometry(OSPRayActor->OSPRayModel, slGeometry);

        }

        ospCommit(OSPRayActor->OSPRayModel);
        if (inputInfo && inputInfo->Has(vtkDataObject::DATA_TIME_STEP())) {
            double time = inputInfo->Get(vtkDataObject::DATA_TIME_STEP());
            OSPRayActor->cache[time] = OSPRayActor->OSPRayModel;
        } else {
            OSPRayActor->cache[timestep] = OSPRayActor->OSPRayModel;
        }

    } else {
        delete mesh;
    }

}
