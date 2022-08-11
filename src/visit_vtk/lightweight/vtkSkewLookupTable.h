// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <visit-config.h> // For LIB_VERSION_LE

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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Map one value through the lookup table.
#if LIB_VERSION_LE(VTK, 8,1,0)
  unsigned char *MapValue(double v) override;
#else
  const unsigned char *MapValue(double v) override;
#endif

  // Description:
  // map a set of scalars through the lookup table
  void MapScalarsThroughTable2(void *input, unsigned char *output,
                               int inputDataType, int numberOfValues,
                               int inputIncrement, int outputIncrement) override;

  // Description:
  // Sets/Gets the skew factor. 
  vtkSetMacro(SkewFactor, float);
  vtkGetMacro(SkewFactor, float);

protected:
  vtkSkewLookupTable(int sze=256, int ext=256);
  ~vtkSkewLookupTable(){};

  float SkewFactor;

private:
  vtkSkewLookupTable(const vtkSkewLookupTable&) = delete;
  void operator=(const vtkSkewLookupTable&) = delete;
};


#endif

