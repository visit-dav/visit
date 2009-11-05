/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSkewLookupTable.h,v $
  Language:  C++
  Date:      $Date: 2000/12/10 20:08:12 $
  Version:   $Revision: 1.55 $


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
// .NAME vtkSkewLookupTable - map scalar values into colors using 
// skewed color table
// .SECTION Description
// vtkSkewLookupTable is an object that is used by mapper objects to map scalar
// values into rgba (red-green-blue-alpha transparency) color specification,
// or rgba into scalar values. The difference between this class and its
// superclass vtkLookupTable is that this class performs scalar mapping based
// on a skewed lookup process. (Uses SkewFactor set by user, value of 1.
// behaves the same as the superclass). 
//
// .SECTION See Also
// vtkLookupTable

// WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
//  The modules vtkVisItOpenGLPolyDataMapper and 
//  vtkOpenGLRectilinearGridMapper make a copy of the lookup
//  table.  When they do, they have special coding for dealing with skew
//  lookup tables.  This special coding is necessary because the
//  routine vtkLookupTable::DeepCopy is not virtual.
//
//  So: if you modify this class, especially if you add new data members,
//  please look at the mapper classes and search for
//  vtkSkewLookupTable to see if modifications are necessary.
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING

#ifndef __vtkSkewLookupTable_h
#define __vtkSkewLookupTable_h
#include <visit_vtk_light_exports.h>

#include "vtkLookupTable.h"

// *************************************************************************
// Modifications:
//   Kathleen Bonnell, Fri Feb 15 12:50:15 PST 2002 
//   Removed SetTableRange methods, deferred implementation to base class.
//
//   Brad Whitlock, Fri Dec 19 15:49:14 PST 2008
//   RemovedSkewTheValue
//
// *************************************************************************

class VISIT_VTK_LIGHT_API vtkSkewLookupTable : public vtkLookupTable
{
public:
  static vtkSkewLookupTable *New();
  
  vtkTypeMacro(vtkSkewLookupTable,vtkLookupTable);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Map one value through the lookup table.
  unsigned char *MapValue(float v);

  // Description:
  // map a set of scalars through the lookup table
  void MapScalarsThroughTable2(void *input, unsigned char *output,
                               int inputDataType, int numberOfValues,
                               int inputIncrement, int outputIncrement);

  // Description:
  // Sets/Gets the skew factor. 
  vtkSetMacro(SkewFactor, float);
  vtkGetMacro(SkewFactor, float);

protected:
  vtkSkewLookupTable(int sze=256, int ext=256);
  ~vtkSkewLookupTable(){};

  float SkewFactor;

private:
  vtkSkewLookupTable(const vtkSkewLookupTable&);
  void operator=(const vtkSkewLookupTable&);
};


#endif



