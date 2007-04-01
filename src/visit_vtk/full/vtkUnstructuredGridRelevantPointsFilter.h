/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkUnstructuredGridRelevantPointsFilter.h,v $
  Language:  C++
  Date:      $Date: 2000/09/01 16:26:45 $
  Version:   $Revision: 1.41 $


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen
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
// .NAME vtkUnstructuredGridRelevantPointsFilter - strips away points and 
// associated point data that are not referenced by any cell in the input.
// associated with any cells 
// .SECTION Description
// vtkUnstructuredGridRelevantPointsFilter is a filter that takes unstructured
// grid data as input and generates unstructured grid data as output. 
// vtkUnstructuredGridRelevantPointsFilter removes points and associated
// point data that are not referenced by any cell in the input data set. 
//
// .SECTION Caveats
//
// .SECTION See Also
// vtkUnstructuredGridToUnstructuredGridFilter

#ifndef __vtkUnstructuredGridRelevantPointsFilter_h
#define __vtkUnstructuredGridRelevantPointsFilter_h
#include <visit_vtk_exports.h>

#include "vtkUnstructuredGridToUnstructuredGridFilter.h"

class VISIT_VTK_API vtkUnstructuredGridRelevantPointsFilter : 
public vtkUnstructuredGridToUnstructuredGridFilter
{
public:
  static vtkUnstructuredGridRelevantPointsFilter *New();
  vtkTypeMacro(vtkUnstructuredGridRelevantPointsFilter, 
               vtkUnstructuredGridToUnstructuredGridFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkUnstructuredGridRelevantPointsFilter(){};
 ~vtkUnstructuredGridRelevantPointsFilter(){};

  // Usual data generation method
  void Execute();

private:
  vtkUnstructuredGridRelevantPointsFilter
       (const vtkUnstructuredGridRelevantPointsFilter&);
  void operator=(const vtkUnstructuredGridRelevantPointsFilter&);
};

#endif


