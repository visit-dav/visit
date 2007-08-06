/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDataSetRemoveGhostCells.h,v $
  Language:  C++
  Date:      $Date: 2001/05/01 16:40:02 $
  Version:   $Revision: 1.1 $


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
// .NAME vtkDataSetRemoveGhostCells - Removes ghost cells whose ghost level 
// is above a specified value

#ifndef __vtkDataSetRemoveGhostCells_h
#define __vtkDataSetRemoveGhostCells_h
#include <visit_vtk_exports.h>

#include "vtkDataSetToDataSetFilter.h"
#include <vtkSetGet.h>


//  Modifications: 
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002
//    Removed FindCellMins and FindCellMax methods.
//
//    Hank Childs, Mon Aug 30 16:27:16 PDT 2004
//    Removed GhostLevels data member.
//
//    Hank Childs, Thu Mar  2 11:14:53 PST 2006
//    Add GenericExecute.
//
class VISIT_VTK_API vtkDataSetRemoveGhostCells : public vtkDataSetToDataSetFilter
{
public:
  static vtkDataSetRemoveGhostCells *New();
  vtkTypeMacro(vtkDataSetRemoveGhostCells, vtkDataSetToDataSetFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro(GhostNodeTypesToRemove,unsigned char);
  vtkGetMacro(GhostNodeTypesToRemove,unsigned char);

protected:
  vtkDataSetRemoveGhostCells();
  ~vtkDataSetRemoveGhostCells() {};

  unsigned char GhostNodeTypesToRemove;

  // Usual data generation method
  void Execute();
  // Specific data generation methods
  void StructuredGridExecute();
  void UnstructuredGridExecute();
  void RectilinearGridExecute();
  void PolyDataExecute();
  void GenericExecute();

private:
  vtkDataSetRemoveGhostCells(const vtkDataSetRemoveGhostCells&);
  void operator=(const vtkDataSetRemoveGhostCells&);

};

#endif
