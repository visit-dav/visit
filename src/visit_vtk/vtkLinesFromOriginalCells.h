// .NAME vtkLinesFromOriginalCells - extract cell edges from poly data
//
// .SECTION Description
// vtkLinesFromOriginalCells is a filter that extracts edges from polydata,
// Excludes edges shared between two cells that were part of the same original
// cell, as determined by the field data array "OriginalCellNumbers".
// Utilizes ExtractEdges if the data array is not set.
//
// .SECTION See Also
// vtkExtractEdges

#ifndef __vtkLinesFromOriginalCells_h
#define __vtkLinesFromOriginalCells_h
#include <visit_vtk_exports.h>

#include "vtkPolyDataToPolyDataFilter.h"
#include <vtkPointLocator.h>

class VISIT_VTK_API vtkLinesFromOriginalCells : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkLinesFromOriginalCells *New();
  vtkTypeMacro(vtkLinesFromOriginalCells,vtkPolyDataToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set / get a spatial locator for merging points. By
  // default an instance of vtkMergePoints is used.
  virtual void SetLocator(vtkPointLocator*);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified.
  void CreateDefaultLocator();

  // Description:
  // Return MTime also considering the locator.
  unsigned long GetMTime();

protected:
  vtkLinesFromOriginalCells();
  ~vtkLinesFromOriginalCells();

  // Usual data generation method
  void Execute();

  vtkPointLocator *Locator;

private:
  vtkLinesFromOriginalCells(const vtkLinesFromOriginalCells&);
  void operator=(const vtkLinesFromOriginalCells&);

};

#endif


