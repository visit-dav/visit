/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVertexFilter.h,v $
  Language:  C++
  Date:      $Date: 2001/08/30 14:10:58 $
  Version:   $Revision: 1.1 $
  Thanks:    Hank Childs, B Division, Lawrence Livermore Nat'l Laboratory

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
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


=========================================================================*/

// .NAME vtkVertexFilter -- Make a vertex at each point incident to a cell
//     or at each cell.
//
// .SECTION Description
//     You can specify if you would like cell vertices or point vertices.
//

#ifndef __vtkVertexFilter_h
#define __vtkVertexFilter_h
#include <visit_vtk_exports.h>

#include "vtkDataSetToPolyDataFilter.h"

class VISIT_VTK_API vtkVertexFilter : public vtkDataSetToPolyDataFilter
{
public:
  vtkTypeMacro(vtkVertexFilter, vtkDataSetToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(VertexAtPoints,int);
  vtkGetMacro(VertexAtPoints,int);
  vtkBooleanMacro(VertexAtPoints,int);

  // Description:
  // Instantiate a stride filter that throws away nine of every ten elements.
  static vtkVertexFilter *New();

protected:
  int VertexAtPoints;

  vtkVertexFilter();
  ~vtkVertexFilter() {};

  void Execute();

private:
  vtkVertexFilter(const vtkVertexFilter&);
  void operator=(const vtkVertexFilter&);
};

#endif


