// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


#ifndef vtkVisItExtractCellsByType_h
#define vtkVisItExtractCellsByType_h
#include <visit_vtk_exports.h>
#include <vtkExtractCellsByType.h>

// ***************************************************************************
//  Kathleen Biagas, Thu Jan 15, 2026
//  Subclass from the vtk class in order to provide an easy way to tell the
//  extractor to extract all cells except lines, or all cells except verts.
//
//  Attempted to use the vtk class directly, but 
//  Using 'AddAllCellTypes' followed by calls to 'RemoveCellType(VTK_VERTEX)'
//  for instance, ends up with NO cells being extracted.
//
// ***************************************************************************

class VISIT_VTK_API vtkVisItExtractCellsByType : public vtkExtractCellsByType
{
public:
  static vtkVisItExtractCellsByType *New();
  vtkTypeMacro(vtkVisItExtractCellsByType, vtkExtractCellsByType);

  void AddAllButLines(vtkDataSet *inDS);
  void AddAllButVerts(vtkDataSet *inDS);

protected:
  vtkVisItExtractCellsByType();
  ~vtkVisItExtractCellsByType() override;

private:
  vtkVisItExtractCellsByType(const vtkVisItExtractCellsByType&) = delete;
  void operator=(const vtkVisItExtractCellsByType&) = delete;
};

#endif
