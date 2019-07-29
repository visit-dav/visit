// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkCSGCell - an CSG cell used to help vtkCSGGrid satisfy the
// interface requirements for a vtkDataSet
// .SECTION Description
// vtkCSGCell is a concrete implementation of vtkCell. It is used
// primarily to help the vtkCSGGrid satisfy the interface requirements
// for a vtkDataSet. The analogy of point with boundary (see vtkCSGGrid)
// is used here as well. That is, replace the word "Point" with
// "Boundary" in this interface and it will make pretty good sense.
#ifndef __vtkCSGCell_h
#define __vtkCSGCell_h
#include <visit_vtk_exports.h>

#include "vtkCell.h"

class vtkPointLocator;

#define VTK_CSG_CELL 50

class VISIT_VTK_API vtkCSGCell : public vtkCell
{
public:
  static vtkCSGCell *New();
  vtkTypeMacro(vtkCSGCell,vtkCell);

  // Description:
  // See the vtkCell API for descriptions of these methods.
  vtkCell *MakeObject() {return vtkCSGCell::New();};
  int GetCellType() override {return VTK_CSG_CELL;};
  int GetCellDimension() override {return 0;};
  int GetNumberOfEdges() override {return 0;};
  int GetNumberOfFaces() override {return 0;};
  vtkCell *GetEdge(int) override {return 0;};
  vtkCell *GetFace(int) override {return 0;};
  int CellBoundary(int subId, double pcoords[3], vtkIdList *pts) override;

  virtual void Contour(double, vtkDataArray*, vtkIncrementalPointLocator*, vtkCellArray*, vtkCellArray*, vtkCellArray*, vtkPointData*, vtkPointData*, vtkCellData*, vtkIdType, vtkCellData*) override;

  virtual void Clip(double, vtkDataArray*, vtkIncrementalPointLocator*, vtkCellArray*, vtkPointData*, vtkPointData*, vtkCellData*, vtkIdType, vtkCellData*, int) override;

  int EvaluatePosition(double x[3], double* closestPoint, 
                       int& subId, double pcoords[3], 
                       double& dist2, double *weights) override;
  void EvaluateLocation(int& subId, double pcoords[3], double x[3],
                        double *weights) override;
  int IntersectWithLine(double p1[3], double p2[3], double tol, double& t,
                        double x[3], double pcoords[3], int& subId) override;
  int Triangulate(int index, vtkIdList *ptIds, vtkPoints *pts) override;
  void Derivatives(int subId, double pcoords[3], double *values, 
                   int dim, double *derivs) override;

protected:
  vtkCSGCell() {};
  ~vtkCSGCell() {};


private:
  vtkCSGCell(const vtkCSGCell&);  // Not implemented.
  void operator=(const vtkCSGCell&);  // Not implemented.
};

#endif


