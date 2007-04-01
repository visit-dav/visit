/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkUniqueFeatureEdges.h,v $
  Language:  C++
  Date:      $Date: 2000/12/10 20:08:37 $
  Version:   $Revision: 1.32 $


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkUniqueFeatureEdges - extract boundary, non-manifold, and/or 
// sharp edges from polygonal data
// .SECTION Description
// vtkUniqueFeatureEdges is a filter to extract special types of edges from
// input polygonal data. These edges are either 1) boundary (used by 
// one polygon) or a line cell; 2) non-manifold (used by three or more 
// polygons); 3) feature edges (edges used by two triangles and whose
// dihedral angle > FeatureAngle); or 4) manifold edges (edges used by
// exactly two polygons). 

// .SECTION Caveats
// This is almost exactly like vtkFeatureEdges with the exceptions:
//     (1)  there is no edge 'coloring'. 
//     (2)  vtkEdgeTable is used to ensure no duplicate edges are inserted. 
//     (3)  ghostCells compared to UpdateGhostLevel instead of 0. 

// .SECTION See Also
// vtkFeatureVertices

#ifndef __vtkUniqueFeatureEdges_h
#define __vtkUniqueFeatureEdges_h
#include <visit_vtk_exports.h>
#include "vtkPolyDataToPolyDataFilter.h"

#include <vtkPointLocator.h>

class VISIT_VTK_API vtkUniqueFeatureEdges : public vtkPolyDataToPolyDataFilter
{
public:
  vtkTypeMacro(vtkUniqueFeatureEdges,vtkPolyDataToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with feature angle = 30; all types of edges extracted
  // and colored.
  static vtkUniqueFeatureEdges *New();

  // Description:
  // Turn on/off the extraction of boundary edges.
  vtkSetMacro(BoundaryEdges,int);
  vtkGetMacro(BoundaryEdges,int);
  vtkBooleanMacro(BoundaryEdges,int);

  // Description:
  // Turn on/off the extraction of feature edges.
  vtkSetMacro(FeatureEdges,int);
  vtkGetMacro(FeatureEdges,int);
  vtkBooleanMacro(FeatureEdges,int);

  // Description:
  // Specify the feature angle for extracting feature edges.
  vtkSetClampMacro(FeatureAngle,float,0.0,180.0);
  vtkGetMacro(FeatureAngle,float);

  // Description:
  // Turn on/off the extraction of non-manifold edges.
  vtkSetMacro(NonManifoldEdges,int);
  vtkGetMacro(NonManifoldEdges,int);
  vtkBooleanMacro(NonManifoldEdges,int);

  // Description:
  // Turn on/off the extraction of manifold edges.
  vtkSetMacro(ManifoldEdges,int);
  vtkGetMacro(ManifoldEdges,int);
  vtkBooleanMacro(ManifoldEdges,int);

  // Description:
  // Set / get a spatial locator for merging points. By
  // default an instance of vtkMergePoints is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified.
  void CreateDefaultLocator();

  // Description:
  // Return MTime also considering the locator.
  unsigned long GetMTime();

protected:
  vtkUniqueFeatureEdges();
  ~vtkUniqueFeatureEdges();

  // Usual data generation method
  void Execute();
  void ComputeInputUpdateExtents(vtkDataObject *output);
  
  float FeatureAngle;
  int BoundaryEdges;
  int FeatureEdges;
  int NonManifoldEdges;
  int ManifoldEdges;
  vtkPointLocator *Locator;

private:
  vtkUniqueFeatureEdges(const vtkUniqueFeatureEdges&);
  void operator=(const vtkUniqueFeatureEdges&);
};

#endif


