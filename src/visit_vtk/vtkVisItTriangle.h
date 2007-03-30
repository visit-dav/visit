// ************************************************************************* //
//                          vtkVisItTriangle.h                               // 
// ************************************************************************* //
#ifndef __vtkVisItTriangle_h
#define __vtkVisItTriangle_h

#include <vtkTriangle.h>
#include <visit_vtk_exports.h>


// ****************************************************************************
//  Module: vtkVisItTriangle
//
//  Purpose:    To override vtkTriangle, due to a bug in 
//              vtkTriangle::IntersectWithLine routine
//              (as of 10 April 03).
//
//  Notes:
//       This file can be removed when the fixed Kitware version is installed. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 10, 2003
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItTriangle : public vtkTriangle
{
public:
  static vtkVisItTriangle *New();
  vtkTypeRevisionMacro(vtkVisItTriangle,vtkTriangle);


  // Description:
  // Plane intersection plus in/out test on triangle. The in/out test is 
  // performed using tol as the tolerance.
  int IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                        float x[3], float pcoords[3], int& subId);


protected:
  vtkVisItTriangle();
  virtual ~vtkVisItTriangle(){};


private:
  vtkVisItTriangle(const vtkVisItTriangle&);  // Not implemented.
  void operator=(const vtkVisItTriangle&);  // Not implemented.
};
#endif
