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

#define VTK_CSG_CELL 50

class VISIT_VTK_API vtkCSGCell : public vtkCell
{
public:
  static vtkCSGCell *New();
  vtkTypeRevisionMacro(vtkCSGCell,vtkCell);

  // Description:
  // See the vtkCell API for descriptions of these methods.
  vtkCell *MakeObject() {return vtkCSGCell::New();};
  int GetCellType() {return VTK_CSG_CELL;};
  int GetCellDimension() {return 0;};
  int GetNumberOfEdges() {return 0;};
  int GetNumberOfFaces() {return 0;};
  vtkCell *GetEdge(int) {return 0;};
  vtkCell *GetFace(int) {return 0;};
  int CellBoundary(int subId, double pcoords[3], vtkIdList *pts);
  void Contour(double value, vtkDataArray *cellScalars, 
               vtkPointLocator *locator, vtkCellArray *verts1, 
               vtkCellArray *lines, vtkCellArray *verts2, 
               vtkPointData *inPd, vtkPointData *outPd,
               vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd);
  void Clip(double value, vtkDataArray *cellScalars, 
            vtkPointLocator *locator, vtkCellArray *pts,
            vtkPointData *inPd, vtkPointData *outPd,
            vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd,
            int insideOut);

  int EvaluatePosition(double x[3], double* closestPoint, 
                       int& subId, double pcoords[3], 
                       double& dist2, double *weights);
  void EvaluateLocation(int& subId, double pcoords[3], double x[3],
                        double *weights);
  int IntersectWithLine(double p1[3], double p2[3], double tol, double& t,
                        double x[3], double pcoords[3], int& subId);
  int Triangulate(int index, vtkIdList *ptIds, vtkPoints *pts);
  void Derivatives(int subId, double pcoords[3], double *values, 
                   int dim, double *derivs);

protected:
  vtkCSGCell() {};
  ~vtkCSGCell() {};


private:
  vtkCSGCell(const vtkCSGCell&);  // Not implemented.
  void operator=(const vtkCSGCell&);  // Not implemented.
};

#endif


