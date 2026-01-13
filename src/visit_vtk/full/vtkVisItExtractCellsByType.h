// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


#ifndef vtkVisItExtractCellsByType_h
#define vtkVisItExtractCellsByType_h
#include <visit_vtk_exports.h>
#include <vtkExtractCellsByType.h>


class VISIT_VTK_API vtkVisItExtractCellsByType : public vtkExtractCellsByType
{
public:
  static vtkVisItExtractCellsByType *New();
  vtkTypeMacro(vtkVisItExtractCellsByType, vtkExtractCellsByType);

  void AddAllCellTypes2();

protected:
  vtkVisItExtractCellsByType();
  ~vtkVisItExtractCellsByType() override;

private:
  vtkVisItExtractCellsByType(const vtkVisItExtractCellsByType&) = delete;
  void operator=(const vtkVisItExtractCellsByType&) = delete;
};

#endif
