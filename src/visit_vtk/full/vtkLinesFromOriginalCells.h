// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <vtkPolyDataAlgorithm.h>


class VISIT_VTK_API vtkLinesFromOriginalCells : public vtkPolyDataAlgorithm
{
public:
  static vtkLinesFromOriginalCells *New();
  vtkTypeMacro(vtkLinesFromOriginalCells,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSetMacro(UseOriginalCells, bool);
  vtkGetMacro(UseOriginalCells, bool);
  vtkBooleanMacro(UseOriginalCells, bool);

protected:
  vtkLinesFromOriginalCells();
  ~vtkLinesFromOriginalCells() override;

  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *) override;

  int FillInputPortInformation(int port, vtkInformation *info) override;

  bool UseOriginalCells;
private:
  vtkLinesFromOriginalCells(const vtkLinesFromOriginalCells&) = delete;
  void operator=(const vtkLinesFromOriginalCells&) = delete;
};

#endif
