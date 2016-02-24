## ======================================================================================= ##
## Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas at Austin  ##
## All rights reserved.                                                                    ##
##                                                                                         ##
## Licensed under the BSD 3-Clause License, (the "License"); you may not use this file     ##
## except in compliance with the License.                                                  ##
## A copy of the License is included with this software in the file LICENSE.               ##
## If your copy does not contain the License, you may obtain a copy of the License at:     ##
##                                                                                         ##
##     http://opensource.org/licenses/BSD-3-Clause                                         ##
##                                                                                         ##
## Unless required by applicable law or agreed to in writing, software distributed under   ##
## the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY ##
## KIND, either express or implied.                                                        ##
## See the License for the specific language governing permissions and limitations under   ##
## limitations under the License.                                                          ##
## ======================================================================================= ##

set (_vtk_modules)
if(PARAVIEW_USE_MPI)
  list(APPEND _vtk_modules vtkParallelMPI)
endif()

if (BUILD_AGAINST_PARAVIEW)
  list(APPEND _vtk_modules vtkPVClientServerCoreRendering)
endif()

if (VTK_RENDERING_BACKEND STREQUAL "OpenGL2")
  message("GL2")
  list(APPEND _vtk_modules vtkRenderingOpenGL2)
else()
  message("GL1")
  list(APPEND _vtk_modules vtkRenderingOpenGL)
endif()

vtk_module(vtkOSPRay
  DEPENDS
    vtkFiltersCore
    vtkParallelCore
    vtkFiltersHybrid
    ${_vtk_modules}
  TEST_DEPENDS
    vtkTestingRendering
    vtkIOPLY
  TEST_LABELS
    PARAVIEW
  EXCLUDE_FROM_WRAP_HIERARCHY
)
