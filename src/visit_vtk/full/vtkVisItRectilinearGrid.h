// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       vtkVisItRectilinearGrid.h                           //
//                                                                           //
// This subclass of vtkRectilinearGrid is here to provide exactly the same   //
// functionality as vtkRectilinearGrid, but with a smaller memory footprint. //
// This class deletes several data members created by its subclass, and      //
// allocates them on demand instead.                                         //
// ************************************************************************* //

#ifndef __vtkVisItRectilinearGrid_h
#define __vtkVisItRectilinearGrid_h

#include <visit_vtk_exports.h>
#include <vtkRectilinearGrid.h>

class VISIT_VTK_API vtkVisItRectilinearGrid : public vtkRectilinearGrid
{
public:
  vtkTypeMacro(vtkVisItRectilinearGrid,vtkRectilinearGrid);
  static vtkVisItRectilinearGrid *New();

  virtual vtkCell *GetCell(vtkIdType cellId) override;
  virtual vtkCell *FindAndGetCell(double x[3],
                                  vtkCell *vtkNotUsed(cell), 
                                  vtkIdType vtkNotUsed(cellId),
                                  double vtkNotUsed(tol2),
                                  int& subId, 
                                  double pcoords[3], double *weights) override;
protected:
  vtkVisItRectilinearGrid();
  virtual ~vtkVisItRectilinearGrid();

private:
  vtkVisItRectilinearGrid(const vtkVisItRectilinearGrid&);
  void operator=(const vtkVisItRectilinearGrid&);
};
#endif


