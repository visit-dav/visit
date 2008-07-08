/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVectorReduceFilter.h,v $
  Language:  C++
  Date:      $Date: 2001/03/20 14:10:58 $
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

// .NAME vtkVectorReduceFilter -- Reduce the number of vectors.
//
// .SECTION Description
// Allows a dataset to be reduced by keeping only one out of every N points.
// It takes an input dataset and throws away some points making poly data that
// can go into vtkGlyph3D.
//
// .CAVEATS You can specify the stride in one of two ways -- by specifying how
//  many total elements you want (SetNumberOfElements) or by specifying how
//  many to process for every one saved (SetStride).
//
//  Modifications:
//    Jeremy Meredith, Tue Jul  8 11:07:57 EDT 2008
//    Added ability to limit to only one output vector per original
//    cell/point.  Also, fixed cell-based vector algorithm bugs.


#ifndef __vtkVectorReduceFilter_h
#define __vtkVectorReduceFilter_h
#include <visit_vtk_exports.h>

#include "vtkDataSetToPolyDataFilter.h"

class VISIT_VTK_API vtkVectorReduceFilter : public vtkDataSetToPolyDataFilter
{
public:
  vtkTypeMacro(vtkVectorReduceFilter, vtkDataSetToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate a stride filter that throws away nine of every ten elements.
  static vtkVectorReduceFilter *New();

  void SetStride(int);
  void SetNumberOfElements(int);
  void SetLimitToOriginal(bool);

protected:
  vtkVectorReduceFilter();
  ~vtkVectorReduceFilter() {};

  void Execute();

  int stride;
  int numEls;
  bool origOnly;

private:
  vtkVectorReduceFilter(const vtkVectorReduceFilter&);
  void operator=(const vtkVectorReduceFilter&);
};

#endif


