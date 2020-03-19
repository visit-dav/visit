// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkCellIntersections_h
#define __vtkCellIntersections_h

#include <visit_vtk_light_exports.h>
#include <vtkObject.h>

class vtkCell;
class vtkVertex;
class vtkPolyVertex;
class vtkLine;
class vtkPolyLine;
class vtkTriangle;
class vtkTriangleStrip;
class vtkPolygon;
class vtkPixel;
class vtkQuad;
class vtkTetra;
class vtkVoxel;
class vtkHexahedron;
class vtkWedge;
class vtkPyramid;
class vtkQuadraticHexahedron;

class VISIT_VTK_LIGHT_API vtkCellIntersections : public vtkObject
{
public:
  vtkTypeMacro(vtkCellIntersections,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  static vtkCellIntersections *New();

  // Description:
  // Boolean controls whether to test for Co-Planar condition.
  vtkSetMacro(TestCoPlanar,bool);
  vtkGetMacro(TestCoPlanar,bool);
  vtkBooleanMacro(TestCoPlanar,bool);

  int CellIntersectWithLine(vtkCell *, double [3], double [3], 
                                double&, double [3]);

  static int IntersectBox(const double[6], const double [3],
                          const double[3], double [3]);
  static int LineIntersectBox(const double[6], const double [3],
                          const double[3], double [3]);

protected:
  vtkCellIntersections();
  ~vtkCellIntersections();

private:
  vtkCellIntersections(const vtkCellIntersections&);  // Not implemented.
  void operator=(const vtkCellIntersections&);  // Not implemented.


  int VertexIntersectWithLine(vtkVertex *, double [3], double [3], 
                                double&, double [3]);

  int PolyVertexIntersectWithLine(vtkPolyVertex *, double [3], double [3], 
                                double&, double [3]);

  int LineIntersectWithLine(vtkLine *, double [3], double [3], 
                                double&, double [3]);

  int PolyLineIntersectWithLine(vtkPolyLine *, double [3], double [3], 
                                double&, double [3]);

  int TriangleIntersectWithLine(vtkTriangle *, double [3], double [3], 
                                double&, double [3]);

  int TriStripIntersectWithLine(vtkTriangleStrip *, double [3], double [3], 
                                double&, double [3]);

  int PolygonIntersectWithLine(vtkPolygon *, double [3], double [3], 
                                double&, double [3]);

  int PixelIntersectWithLine(vtkPixel *, double [3], double [3], 
                                double&, double [3]);

  int QuadIntersectWithLine(vtkQuad *, double [3], double [3], 
                                double&, double [3]);

  int TetraIntersectWithLine(vtkTetra *, double [3], double [3], 
                                double&, double [3]);

  int VoxelIntersectWithLine(vtkVoxel *, double [3], double [3], 
                                double&, double [3]);

  int HexIntersectWithLine(vtkHexahedron *, double [3], double [3], 
                                double&, double [3]);

  int WedgeIntersectWithLine(vtkWedge *, double [3], double [3], 
                                double&, double [3]);

  int PyramidIntersectWithLine(vtkPyramid *, double [3], double [3], 
                                double&, double [3]);

  int QuadraticHexahedronIntersectWithLine(vtkQuadraticHexahedron *, 
                                double [3], double [3], double&, double [3]);

  vtkTriangle *triangle;
  vtkQuad *quad;
  bool TestCoPlanar;
};

#endif


