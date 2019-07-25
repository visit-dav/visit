// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       vtkVisItStructuredGrid.h                            //
//                                                                           //
// This subclass of vtkStructuredGrid is here to provide exactly the same    //
// functionality as vtkStructuredGrid, but with a smaller memory footprint.  //
// This class deletes several data members created by its subclass, and      //
// allocates them on demand instead.                                         //
// ************************************************************************* //

#ifndef __vtkVisItStructuredGrid_h
#define __vtkVisItStructuredGrid_h

#include <visit_vtk_exports.h>
#include <vtkStructuredGrid.h>

class VISIT_VTK_API vtkVisItStructuredGrid : public vtkStructuredGrid
{
public:
  vtkTypeMacro(vtkVisItStructuredGrid,vtkStructuredGrid);
  static vtkVisItStructuredGrid *New();

  virtual vtkCell *GetCell(vtkIdType cellId) override;

protected:
  vtkVisItStructuredGrid();
  virtual ~vtkVisItStructuredGrid();

private:
  vtkVisItStructuredGrid(const vtkVisItStructuredGrid&);
  void operator=(const vtkVisItStructuredGrid&);
};
#endif


