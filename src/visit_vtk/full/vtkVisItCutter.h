/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItCutter.h,v $
  Language:  C++
  Date:      $Date: 2002/01/22 15:29:13 $
  Version:   $Revision: 1.54 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItCutter - Cut vtkDataSet with user-specified implicit function
// .SECTION Description
// vtkVisItCutter is a filter to cut through data using any subclass of 
// vtkImplicitFunction. That is, a polygonal surface is created
// corresponding to the implicit function F(x,y,z) = value(s), where
// you can specify one or more values used to cut with.
//
// In VTK, cutting means reducing a cell of dimension N to a cut surface
// of dimension N-1. For example, a tetrahedron when cut by a plane (i.e.,
// vtkPlane implicit function) will generate triangles. (Clipping takes
// a N dimensional cell and creates N dimension primitives.)
//
// vtkVisItCutter is generally used to "slice-through" a dataset, generating
// a surface that can be visualized. It is also possible to use vtkVisItCutter
// to do a form of volume rendering. vtkVisItCutter does this by generating
// multiple cut surfaces (usually planes) which are ordered (and rendered)
// from back-to-front. The surfaces are set translucent to give a 
// volumetric rendering effect.

// .SECTION See Also
// vtkImplicitFunction vtkClipPolyData

#ifndef __vtkVisItCutter_h
#define __vtkVisItCutter_h

#include <visit_vtk_exports.h>
#include "vtkDataSetToPolyDataFilter.h"
#include "vtkContourValues.h" // needed for inline functions.


class vtkImplicitFunction;
class vtkPointLocator;

#define VTK_SORT_BY_VALUE 0
#define VTK_SORT_BY_CELL 1

class VISIT_VTK_API vtkVisItCutter : public vtkDataSetToPolyDataFilter
{
public:
  vtkTypeRevisionMacro(vtkVisItCutter,vtkDataSetToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with user-specified implicit function; initial value of 0.0; and
  // generating cut scalars turned off.
  static vtkVisItCutter *New();

  // Description:
  // Set a particular contour value at contour number i. The index i ranges 
  // between 0<=i<NumberOfContours.
  void SetValue(int i, float value) 
    {this->ContourValues->SetValue(i,value);}
  
  // Description:
  // Get the ith contour value.
  float GetValue(int i) 
    {return this->ContourValues->GetValue(i);}

  // Description:
  // Get a pointer to an array of contour values. There will be
  // GetNumberOfContours() values in the list.
  float *GetValues() 
    {return this->ContourValues->GetValues();}

  // Description:
  // Fill a supplied list with contour values. There will be
  // GetNumberOfContours() values in the list. Make sure you allocate
  // enough memory to hold the list.
  void GetValues(float *contourValues)
    {this->ContourValues->GetValues(contourValues);}
  
  // Description:
  // Set the number of contours to place into the list. You only really
  // need to use this method to reduce list size. The method SetValue()
  // will automatically increase list size as needed.
  void SetNumberOfContours(int number) 
    {this->ContourValues->SetNumberOfContours(number);}

  // Description:
  // Get the number of contours in the list of contour values.
  int GetNumberOfContours() 
    {return this->ContourValues->GetNumberOfContours();}

  // Description:
  // Generate numContours equally spaced contour values between specified
  // range. Contour values will include min/max range values.
  void GenerateValues(int numContours, float range[2]) 
    {this->ContourValues->GenerateValues(numContours, range);}

  // Description:
  // Generate numContours equally spaced contour values between specified
  // range. Contour values will include min/max range values.
  void GenerateValues(int numContours, float rangeStart, float rangeEnd) 
    {this->ContourValues->GenerateValues(numContours, rangeStart, rangeEnd);}

  // Description:
  // Override GetMTime because we delegate to vtkContourValues and refer to
  // vtkImplicitFunction.
  unsigned long GetMTime();

  // Description
  // Specify the implicit function to perform the cutting.
  virtual void SetCutFunction(vtkImplicitFunction*);
  vtkGetObjectMacro(CutFunction,vtkImplicitFunction);

  // Description:
  // If this flag is enabled, then the output scalar values will be
  // interpolated from the implicit function values, and not the input scalar
  // data.
  vtkSetMacro(GenerateCutScalars,int);
  vtkGetMacro(GenerateCutScalars,int);
  vtkBooleanMacro(GenerateCutScalars,int);

  // Description:
  // Specify a spatial locator for merging points. By default, 
  // an instance of vtkMergePoints is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Specify a cell list to cut against.  This allows outside modules to 
  // perform optimizations on which cells are cut.
  void SetCellList(int *, int);

  // Description:
  // Set the sorting order for the generated polydata. There are two
  // possibilities:
  //   Sort by value = 0 - This is the most efficient sort. For each cell,
  //      all contour values are processed. This is the default.
  //   Sort by cell = 1 - For each contour value, all cells are processed.
  //      This order should be used if the extracted polygons must be rendered
  //      in a back-to-front or front-to-back order. This is very problem 
  //      dependent.
  // For most applications, the default order is fine (and faster).
  vtkSetClampMacro(SortBy,int,VTK_SORT_BY_VALUE,VTK_SORT_BY_CELL);
  vtkGetMacro(SortBy,int);
  void SetSortByToSortByValue() 
    {this->SetSortBy(VTK_SORT_BY_VALUE);}
  void SetSortByToSortByCell() 
    {this->SetSortBy(VTK_SORT_BY_CELL);}
  const char *GetSortByAsString();

  // Description:
  // Create default locator. Used to create one when none is specified. The 
  // locator is used to merge coincident points.
  void CreateDefaultLocator();

protected:
  vtkVisItCutter(vtkImplicitFunction *cf=NULL);
  ~vtkVisItCutter();

  void Execute();
  vtkImplicitFunction *CutFunction;
  
  vtkPointLocator *Locator;
  int *CellList;
  int  CellListSize;
  int SortBy;
  vtkContourValues *ContourValues;
  int GenerateCutScalars;
private:
  vtkVisItCutter(const vtkVisItCutter&);  // Not implemented.
  void operator=(const vtkVisItCutter&);  // Not implemented.
};

// Description:
// Return the sorting procedure as a descriptive character string.
inline const char *vtkVisItCutter::GetSortByAsString(void)
{
  if ( this->SortBy == VTK_SORT_BY_VALUE ) 
    {
    return "SortByValue";
    }
  else 
    {
    return "SortByCell";
    }
}


#endif


