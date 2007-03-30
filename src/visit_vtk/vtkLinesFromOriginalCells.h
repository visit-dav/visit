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

class VISIT_VTK_API vtkLinesFromOriginalCells : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkLinesFromOriginalCells *New();
  vtkTypeMacro(vtkLinesFromOriginalCells,vtkPolyDataToPolyDataFilter);

protected:
  vtkLinesFromOriginalCells();
  ~vtkLinesFromOriginalCells();

  // Usual data generation method
  void Execute();

private:
  vtkLinesFromOriginalCells(const vtkLinesFromOriginalCells&);
  void operator=(const vtkLinesFromOriginalCells&);

};

#endif


